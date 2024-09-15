

#include "App.h"
#include "imgui/imgui_internal.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "imgui_memory_editor.h"
#include <tracy/Tracy.hpp>
#include <cassert>

using namespace std::chrono;
using namespace std::chrono_literals;

using hr_clock = std::chrono::high_resolution_clock;


App::App()
    : mDisplayBuffer(Display::w, Display::h)
    , mEmulationSpeedComboIdx(static_cast<int>(EmulationSpeed::Full))
    , mLastEmulateCall(-1ns)
{
    // create an OpenGL texture identifier for the display image
    glGenTextures(1, &mGLDisplayTexture);

    // create OpenGL textures to display tiles
    mTileTextures.resize(VRam::maxTiles);
    glGenTextures(VRam::maxTiles, mTileTextures.data());

    // create buffers for single tiles
    mTileBuffers.resize(VRam::maxTiles);

    // do the  same for OAMs
    mOamTextures.resize(OAMRam::oamCount);
    glGenTextures(OAMRam::oamCount, mOamTextures.data());

    mOamBuffers.resize(OAMRam::oamCount);
}


// Simple helper function to load an image into a OpenGL texture with common settings
static bool LoadTextureFromMatrix(const Matrix& mat, GLuint& outTexture, RgbaBufferIf& buffer)
{
    glBindTexture(GL_TEXTURE_2D, outTexture);

    // turn the tile data into an RGB buffer
    mat.fillRgbaBuffer(buffer);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mat.width(), mat.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.ptr());

    return true;
}

static ImVec4 rgbaPixelToImVec4(RgbaPixel pix)
{
    // colors in ImVec4 are expressed as values between 0 and 1
    return ImVec4{ 
        (float)pix.R / 255.f,
        (float)pix.G / 255.f,
        (float)pix.B / 255.f,
        (float)pix.A / 255.f
    };
}


static Joypad::PressedButton getPressedButtons()
{
    Joypad::PressedButton btns;

    if (ImGui::IsKeyDown(ImGuiKey_W)) btns.add(Joypad::Btn::Up);
    if (ImGui::IsKeyDown(ImGuiKey_A)) btns.add(Joypad::Btn::Left);
    if (ImGui::IsKeyDown(ImGuiKey_S)) btns.add(Joypad::Btn::Down);
    if (ImGui::IsKeyDown(ImGuiKey_D)) btns.add(Joypad::Btn::Right);

    if (ImGui::IsKeyDown(ImGuiKey_N)) btns.add(Joypad::Btn::A);
    if (ImGui::IsKeyDown(ImGuiKey_M)) btns.add(Joypad::Btn::B);
    if (ImGui::IsKeyDown(ImGuiKey_Enter)) btns.add(Joypad::Btn::Start);
    if (ImGui::IsKeyDown(ImGuiKey_0)) btns.add(Joypad::Btn::Select);

    return btns;
}


std::optional<nanoseconds> App::emulateFor() const
{
    // when emulating at 100% speed we have to emulate at 60 gameboy frames per second,
    // the app is limited to 60fps so we have to emulate for 1 frame

    // the gameboy is actually running slightly slower than 60fps.
    // the PPU renders one line in 108.7us so, considering that we have 144 lines + 10 v-blank lines,
    // a full frame takes 108.7us * 154 = 16.7398ms.
    // This means that the gameboy runs at 59.737870 fps
    // 
    // 1 frame at 59.737...fps takes 16.7398ms so we have to emulate until the total amount of
    // executed m-cycles amounts to that time value

    // an empty optional means 'unbound'

    static constexpr auto frameTime = 16739800ns;

    switch (mConfig.emulationSpeed) {
    case EmulationSpeed::Quarter: return frameTime/4;
    case EmulationSpeed::Half: return frameTime/2;
    case EmulationSpeed::Full: return frameTime;
    default:
    case EmulationSpeed::Unbound: return {};
    }
}


const char* const plotOvershootPlotName = "EmulateOvershoot";
const char* const plotTimeSinceLastEmulateCall = "TimeSinceLastEmulateCall";
const char* const plotRequiredFrames = "RequiredFrames";

void App::startup()
{
    TracyPlotConfig(plotOvershootPlotName, tracy::PlotFormatType::Number, false, false, 0);
    TracyPlotConfig(plotTimeSinceLastEmulateCall, tracy::PlotFormatType::Number, false, false, 0);
    TracyPlotConfig(plotRequiredFrames, tracy::PlotFormatType::Number, false, false, 0);
}

bool App::emulate()
{
    ZoneScoped;

    auto currTime = getCurrTime();

    // check which buttons are pressed once here (input state won't be 
    // updated until the next call to the app loop)
    mGameboy.joypad.action(getPressedButtons());

    // emulate for a while
    if (mConfig.emulationSpeed == EmulationSpeed::Full)
        emulateFullSpeed(currTime);
    else
        emulateOtherSpeeds(currTime);

    mLastEmulateCall = currTime;

    return true;
}


bool App::emulateFullSpeed(std::chrono::nanoseconds currTime)
{
    // when emulating at 100% speed we emulate exactly for the required number of frames, taking into 
    // account how much time elapsed between the current call and the last App::emulate() call
    // 
    // if the requested number of frames is not reached in the corresponding gb tim we return anyway,
    // if the ppu is disabled frames won't be ready but the emulation must keep going.

    uint32_t requiredFrames = 1;

    if (mLastEmulateCall != -1ns) {
        auto timeSinceLastCall = currTime - mLastEmulateCall;
        TracyPlot(plotTimeSinceLastEmulateCall, timeSinceLastCall.count());

        requiredFrames = uint32_t(timeSinceLastCall / 16666667ns);
        if (requiredFrames == 0)
            ++requiredFrames;
    }

    TracyPlot(plotRequiredFrames, (int64_t)requiredFrames);


    auto realTargetGbTime = emulateFor().value();
    auto targetGbTime = realTargetGbTime + (2 * CPU::longestInstructionCycles * GameBoyClassic::machinePeriod);
    nanoseconds elapsedGbTime = 0ns;

    uint32_t renderedFrames = 0;
    while (elapsedGbTime < targetGbTime && renderedFrames < requiredFrames) {
        auto [stillGoing, stepRes] = mGameboy.emulate();

        elapsedGbTime += GameBoyClassic::machinePeriod * stepRes.cpuRes.cycles;

        if (stepRes.frameReady) {
            ++renderedFrames;
        }

        if (!stillGoing)
            break;
    }

    TracyPlot(plotOvershootPlotName, (elapsedGbTime - realTargetGbTime).count());

    if (renderedFrames == 0) {
        TracyMessageL("No frame!");
    }

    return true;
}

bool App::emulateOtherSpeeds(std::chrono::nanoseconds /*currTime*/)
{
    if (auto targetGbTime = emulateFor(); targetGbTime) {
        // emulate until a target amount of time has passed for the emulated gameboy
        nanoseconds elapsedGbTime = 0ns;

        while (elapsedGbTime < targetGbTime) {
            auto [stillGoing, stepRes] = mGameboy.emulate();

            if (!stillGoing)
                break;

            elapsedGbTime += GameBoyClassic::machinePeriod * stepRes.cpuRes.cycles;
        }
    }
    else {
        // unbound emulation speed, emulate as much as possible for 1/60 seconds
        // since we run the app at 60fps
        // actually run for 16ms, leave some time for drawing the ui and other stuff
        auto start = hr_clock::now();

        while (hr_clock::now() - start < 16ms) {
            mGameboy.emulate();
        }
    }

    return true;
}




void App::updateUI()
{
    UISetupDocking();

    UIDrawMenu();
    UIDrawControlWindow();
    UIDrawEmulationWindow();
    UIDrawRegsTables();
    UIDrawMemoryEditorWindow();
    UIDrawTileViewerWindow();
}

void App::UISetupDocking()
{    
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    }
}

void App::UIDrawMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open rom file...", "CTRL+O")) {
                IGFD::FileDialogConfig config;

                if (mConfig.currentRomPath.empty())
                    config.path = ".";
                else
                    config.path = mConfig.currentRomPath.parent_path().string().c_str();

                config.flags = ImGuiFileDialogFlags_Modal;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose Rom File", ".gb", config);
            }
            if (ImGui::BeginMenu("Open Recent...")) {
                if (mConfig.recentRomsPath.empty()) {
                    ImGui::MenuItem("Recent rom files will\nbe displayed here...", nullptr, nullptr, false);
                }
                else {
                    for (const auto& p : mConfig.recentRomsPath) {
                        if (ImGui::MenuItem(p.string().c_str())) {
                            loadRomFile(p);
                        }
                    }
                }

                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Alt+F4")) {
                closeWindow();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem("Memory editor", nullptr, &mConfig.showMemoryEditor);
            ImGui::MenuItem("Tile viewer", nullptr, &mConfig.showTileViewer);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("About")) {
            if (ImGui::MenuItem("About gbemu")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // handle file dialog
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2{ 350, 250 })) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            // action if OK
            if (loadRomFile(ImGuiFileDialog::Instance()->GetFilePathName())) {
                mConfig.recentRomsPath.push_front(mConfig.currentRomPath);
                if (mConfig.recentRomsPath.size() > 10)
                    mConfig.recentRomsPath.pop_back();
            }
        }
        
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGui::BeginPopupModal("Loading failed", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Rom loading failed! Error:");
        ImGui::Text("%s", cartridgeLoadingResToStr(mConfig.loadingRes));
        
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

bool App::loadRomFile(const std::filesystem::path& path)
{
    mConfig.loadingRes = mGameboy.loadCartridge(path);

    if (mConfig.loadingRes != CartridgeLoadingRes::Ok) {
        ImGui::OpenPopup("Loading failed");
        return false;
    }
    else {
        // if loaded successfully we store the path
        mConfig.currentRomPath = path;
        return true;
    }
}


void App::UIDrawControlWindow()
{
    ImGui::Begin("Emulation", nullptr, ImGuiWindowFlags_NoCollapse);

    // --------------------------------------------------------------------------------------------
    ImGui::SeparatorText("Controls");

    ImGui::Text("Status: %s", GameBoyClassic::statusToStr(mGameboy.status));


    auto btnSize = ImVec2(60, 0);
    
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.71f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.f, 0.71f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.f, 0.81f, 0.8f));
    if (ImGui::Button("Stop", btnSize)) { mGameboy.stop(); }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.33f, 0.71f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.33f, 0.71f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.33f, 0.81f, 0.8f));
    if (ImGui::Button("Start", btnSize)) { mGameboy.play(); }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.16f, 0.71f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.16f, 0.71f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.16f, 0.81f, 0.8f));
    if (ImGui::Button("Pause", btnSize)) { mGameboy.pause(); }
    ImGui::PopStyleColor(3);

    // next line
    btnSize = ImVec2(80, 0);

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.60f, 0.71f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.60f, 0.71f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.60f, 0.81f, 0.8f));
    if (ImGui::Button("Step in", btnSize)) { mGameboy.step(); }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    if (mGameboy.cpu.callNesting() == 0)
        ImGui::BeginDisabled();

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.60f, 0.71f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.60f, 0.71f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.60f, 0.81f, 0.8f));
    if (ImGui::Button("Step out", btnSize)) { mGameboy.stepReturn(); }
    ImGui::PopStyleColor(3);

    if (mGameboy.cpu.callNesting() == 0)
        ImGui::EndDisabled();

    
    static const ImGuiComboFlags comboFlags = ImGuiComboFlags_None;
    static const char* speedItems[] = {
        emulationSpeedToStr(EmulationSpeed::Quarter),
        emulationSpeedToStr(EmulationSpeed::Half),
        emulationSpeedToStr(EmulationSpeed::Full),
        emulationSpeedToStr(EmulationSpeed::Unbound)
    };
    auto preview = speedItems[mEmulationSpeedComboIdx];

    ImGui::PushItemWidth(100);
    if (ImGui::BeginCombo("Emulation speed", preview, comboFlags)) {

        for (int n = 0; n < IM_ARRAYSIZE(speedItems); n++)
        {
            const bool isSelected = (mEmulationSpeedComboIdx == n);
            if (ImGui::Selectable(speedItems[n], isSelected)) {
                mEmulationSpeedComboIdx = n;
                mConfig.emulationSpeed = static_cast<EmulationSpeed>(n);
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::PopItemWidth();

    // --------------------------------------------------------------------------------------------
    ImGui::SeparatorText("Save states");

    if (ImGui::Button("Save state", btnSize)) {

        IGFD::FileDialogConfig config;
        config.path = ".";
        config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite;
        ImGuiFileDialog::Instance()->OpenDialog("SaveStateFileDlgKey", "Save current emulation state", ".gbstate", config);
    }

    if (ImGuiFileDialog::Instance()->Display("SaveStateFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2{ 350, 250 })) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            mConfig.currentSaveStateErr = mGameboy.saveState(ImGuiFileDialog::Instance()->GetFilePathName());

            if (mConfig.currentSaveStateErr != SaveStateError::NoError) {
                ImGui::OpenPopup("Save state error");
            }
        }

        ImGuiFileDialog::Instance()->Close();
    }


    ImGui::SameLine();
    
    
    if (ImGui::Button("Load state", btnSize)) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("LoadStateFileDlgKey", "Load emulation state", ".gbstate", config);
    }

    if (ImGuiFileDialog::Instance()->Display("LoadStateFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2{ 350, 250 })) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            mConfig.currentSaveStateErr = mGameboy.loadState(ImGuiFileDialog::Instance()->GetFilePathName());

            if (mConfig.currentSaveStateErr != SaveStateError::NoError) {
                ImGui::OpenPopup("Save state error");
            }
        }

        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGui::BeginPopupModal("Save state error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", saveStateErrorToStr(mConfig.currentSaveStateErr));
        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }


    // --------------------------------------------------------------------------------------------
    ImGui::SeparatorText("Cartridge info");

    ImGui::TextWrapped("Path: %s", mGameboy.romFilePath.string().c_str());
    ImGui::Spacing();

    static const ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable
        | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

    if (ImGui::BeginTable("cartridgeInfo", 2, tableFlags))
    {
        ImGui::TableSetupColumn("Field");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        auto drawTableLineUint = [](const char* name, const char* fmt, uint32_t val) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(name);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(fmt, val);
        };
        
        auto drawTableLineStr = [](const char* name, const char* fmt, const char* val) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(name);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(fmt, val);
        };

        drawTableLineStr("Title", "%s", mGameboy.cartridge.header.title().c_str());
        drawTableLineStr("CGB Flag", "%s", CGBFlagToStr(mGameboy.cartridge.header.cgbFlag()));
        drawTableLineStr("New Licensee Code", "%s", mGameboy.cartridge.header.newLicenseeCode());
        drawTableLineStr("SGB Flag", "%s", SGBFlagToStr(mGameboy.cartridge.header.sgbFlag()));
        drawTableLineStr("Cartridge Type", "%s", cartTypeToStr(mGameboy.cartridge.header.cartType()));
        drawTableLineUint("ROM Size", "%u", mGameboy.cartridge.header.romSize());
        drawTableLineUint("RAM Size", "%u", mGameboy.cartridge.header.ramSize());
        drawTableLineStr("Destination code", "%s", destCodeToStr(mGameboy.cartridge.header.destCode()));
        drawTableLineStr("Old Licensee Code", "%s", mGameboy.cartridge.header.oldLicenseeCode());
        drawTableLineUint("Mask ROM version", "%x", mGameboy.cartridge.header.maskRomVersionNum());
        drawTableLineUint("Header checksum", "0x%02x", mGameboy.cartridge.header.headerChecksum());
        drawTableLineUint("Global checksum", "0x%04x", mGameboy.cartridge.header.globalChecksum());

        ImGui::EndTable();
    }

    ImGui::End();
}

void App::UIDrawEmulationWindow()
{
    LoadTextureFromMatrix(mGameboy.ppu.display.getFrontBuf(), mGLDisplayTexture, mDisplayBuffer);

    FrameImage(mDisplayBuffer.ptr(), mGameboy.ppu.display.w, mGameboy.ppu.display.h, 0, false);

    ImGui::Begin("GB Display");
    ImGui::Image((void*)(intptr_t)mGLDisplayTexture, ImVec2(320, 288));
    ImGui::End();
}

void App::UIDrawMemoryEditorWindow()
{
    if (!mConfig.showMemoryEditor)
        return;

    ImGui::Begin("Memory Editor", &mConfig.showMemoryEditor);
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("MemoryEditorTabs", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("ROM"))
        {
            static MemoryEditor memEdit;
            memEdit.DrawContents(mGameboy.cartridge.mbc->rom.data(), mGameboy.cartridge.mbc->rom.size(), 0);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("VRAM"))
        {
            static MemoryEditor memEdit;
            memEdit.DrawContents(mGameboy.ppu.vram.data(), mGameboy.ppu.vram.size(), mGameboy.ppu.vram.startAddr());
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("ExtRAM"))
        {
            static MemoryEditor memEdit;
            memEdit.DrawContents(mGameboy.cartridge.mbc->ram.data(), mGameboy.cartridge.mbc->ram.size(), mmap::external_ram::start);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("RAM"))
        {
            static MemoryEditor memEdit;
            memEdit.DrawContents(mGameboy.wram.data(), mGameboy.wram.size(), mGameboy.wram.startAddr());
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("OAM"))
        {
            static MemoryEditor memEdit;
            memEdit.DrawContents(mGameboy.ppu.oamRam.data(), mGameboy.ppu.oamRam.size(), mGameboy.ppu.oamRam.startAddr());
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("HIRAM"))
        {
            static MemoryEditor memEdit;
            memEdit.DrawContents(mGameboy.hiRam.data(), mGameboy.hiRam.size(), mGameboy.hiRam.startAddr());
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

//static void drawPaletteTable(const char* tableName, uint8_t paletteId, const PaletteReg& palette)
static void drawPaletteTable(const char* tableName, PaletteReg** palettes, size_t count)
{
    auto colorBtnSize = ImVec2(32, 32);
    auto colorBtnFlags = ImGuiColorEditFlags_NoBorder;

    static ImGuiTableFlags tablePaletteFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg 
            | ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX;

    auto paletteToImvec4 = [](uint8_t val) {
        switch (val) {
        default:
        case 0: return rgbaPixelToImVec4(whiteA);
        case 1: return rgbaPixelToImVec4(lightGreyA);
        case 2: return rgbaPixelToImVec4(darkGreyA);
        case 3: return rgbaPixelToImVec4(blackA);
        }
    };


    if (ImGui::BeginTable(tableName, 6, tablePaletteFlags)) {
        ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("0", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("2", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("3", ImGuiTableColumnFlags_WidthFixed);

        ImGui::TableHeadersRow();

        for (size_t i = 0; i < count; ++i) {
            ImGui::TableNextColumn();
            ImGui::Text("%u", i);
            ImGui::TableNextColumn();
            ImGui::Text("0x%02x", palettes[i]->asU8());
            ImGui::TableNextColumn();
            ImGui::ColorButton("bgp0", paletteToImvec4(palettes[i]->valForId0), colorBtnFlags, colorBtnSize);
            ImGui::TableNextColumn();
            ImGui::ColorButton("bgp1", paletteToImvec4(palettes[i]->valForId1), colorBtnFlags, colorBtnSize);
            ImGui::TableNextColumn();
            ImGui::ColorButton("bgp2", paletteToImvec4(palettes[i]->valForId2), colorBtnFlags, colorBtnSize);
            ImGui::TableNextColumn();
            ImGui::ColorButton("bgp3", paletteToImvec4(palettes[i]->valForId3), colorBtnFlags, colorBtnSize);
        }

        ImGui::EndTable();
    }
}


void App::UIDrawTileViewerWindow()
{
    if (!mConfig.showTileViewer)
        return;

    ImGui::Begin("Tile Viewer", &mConfig.showTileViewer);

    float childWindowWidth = std::max(ImGui::GetContentRegionAvail().x * 0.5f, 300.f);

    if (ImGui::CollapsingHeader("VRAM Tiles")) {

        uint32_t hoveredTileId = 0;
        auto hoveredTile = mGameboy.ppu.vram.getGenericTile(0);
        float childWindowHeight = 430;

        ImGui::BeginChild("TileMap", ImVec2(childWindowWidth, childWindowHeight));

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 1));

        for (uint32_t id = 0; id < VRam::maxTiles; ++id) {
            auto tile = mGameboy.ppu.vram.getGenericTile(id);
            LoadTextureFromMatrix(tile, mTileTextures[id], mTileBuffers[id]);

            ImGui::Image((void*)(intptr_t)mTileTextures[id], ImVec2(16, 16));
            if (ImGui::IsItemHovered()) {
                hoveredTileId = id;
                hoveredTile = tile;
            }

            if (id % 16 != 15)
                ImGui::SameLine();
        }

        ImGui::PopStyleVar();
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        ImGui::BeginChild("TileZoom", ImVec2(0, childWindowHeight));
        
        ImGui::Image((void*)(intptr_t)mTileTextures[hoveredTileId], ImVec2(128, 128));

        static ImGuiTableFlags tableTileDetailsFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable;

        if (ImGui::BeginTable("tableTileDetails", 2, tableTileDetailsFlags)) {
            ImGui::TableSetupColumn("Attr", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Val", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            ImGui::Text("Tile Id");
            ImGui::TableNextColumn();
            ImGui::Text("%u (0x%04x)", hoveredTileId, hoveredTileId);

            ImGui::TableNextColumn();
            ImGui::Text("Tile Address");
            ImGui::TableNextColumn();
            ImGui::Text("0x%04x", hoveredTile.gbAddr);

            ImGui::EndTable();
        }

        ImGui::EndChild();
    }

    if (ImGui::CollapsingHeader("OAM Data")) {
        
        uint8_t hoveredOamId = 0;
        auto hoveredOam = mGameboy.ppu.oamRam.getOAMData(0);
        auto hoveredTile = mGameboy.ppu.vram.getGenericTile(hoveredOam.tileId());
        auto hoveredAttr = hoveredOam.attr();

        float childWindowHeight = 265;

        ImGui::BeginChild("OAMMap", ImVec2(childWindowWidth, childWindowHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 1));

        for (uint8_t id = 0; id < OAMRam::oamCount; ++id) {
            auto oam = mGameboy.ppu.oamRam.getOAMData(id);
            auto tile = mGameboy.ppu.vram.getGenericTile(oam.tileId());

            LoadTextureFromMatrix(tile, mOamTextures[id], mOamBuffers[id]);

            ImGui::Image((void*)(intptr_t)mOamTextures[id], ImVec2(24, 24));
            if (ImGui::IsItemHovered()) {
                hoveredOamId = id;
                hoveredOam = oam;
                hoveredTile = tile;
                hoveredAttr = oam.attr();
            }

            if (id % 10 != 9)
                ImGui::SameLine();
        }

        ImGui::PopStyleVar();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("OAMZoom", ImVec2(0, childWindowHeight));

        ImGui::Image((void*)(intptr_t)mOamTextures[hoveredOamId], ImVec2(128, 128));

        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable;

        if (ImGui::BeginTable("tableOamData", 2, flags)) {
            ImGui::TableSetupColumn("Attr", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Val", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            ImGui::Text("OAM Id");
            ImGui::TableNextColumn();
            ImGui::Text("%u (0x%02x)", hoveredOamId, hoveredOamId);

            ImGui::TableNextColumn();
            ImGui::Text("OAM Address");
            ImGui::TableNextColumn();
            ImGui::Text("0x%04x", hoveredOam.gbAddr);

            ImGui::TableNextColumn();
            ImGui::Text("X, Y");
            ImGui::TableNextColumn();
            ImGui::Text("%u, %u", hoveredOam.x(), hoveredOam.y());

            ImGui::TableNextColumn();
            ImGui::Text("Tile Id");
            ImGui::TableNextColumn();
            ImGui::Text("%u (0x%04x)", hoveredOam.tileId(), hoveredOam.tileId());

            ImGui::TableNextColumn();
            ImGui::Text("Attributes");
            ImGui::TableNextColumn();
            ImGui::Text("VFlip: %u\nHFlip: %u\nPriority: %u\nPalette: %s", hoveredAttr.vFlip(), hoveredAttr.hFlip(), 
                    hoveredAttr.priority(), hoveredAttr.dmgPalette() ? "OBP1" : "OBP0");

            ImGui::EndTable();
        }

        ImGui::EndChild();
    }

    if (ImGui::CollapsingHeader("Palettes")) {

        ImGui::SeparatorText("BGP");
        
        PaletteReg* bgps[] = {
            &mGameboy.ppu.regs.BGP,
        };
        drawPaletteTable("tableBGPData", (PaletteReg**)bgps, 1);
        
        ImGui::SeparatorText("OBP");
        
        PaletteReg* obps[] = {
            &mGameboy.ppu.regs.OBP0,
            &mGameboy.ppu.regs.OBP1,
        };
        drawPaletteTable("tableOBPData", (PaletteReg**)obps, 2);
        
    }

    ImGui::End();
}



struct RegTableEntry {
    const char* fmt;
    const char* name;
    uint32_t val;
};

static const ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable
    | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

static const char* fmtHex4 = "0x%04x";
static const char* fmtHex2 = "0x%02x";


void App::UIDrawCpuRegTable()
{
    RegTableEntry cpuEntries[] = {
        { fmtHex4, "AF", mGameboy.cpu.regs.AF() },
        { fmtHex4, "SP", mGameboy.cpu.regs.SP },
        { fmtHex4, "BC", mGameboy.cpu.regs.BC() },
        { fmtHex4, "PC", mGameboy.cpu.regs.PC },
        { fmtHex4, "DE", mGameboy.cpu.regs.DE() },
        { fmtHex2, "IE", mGameboy.cpu.irqs.readIE()},
        { fmtHex4, "HL", mGameboy.cpu.regs.HL() },
        { fmtHex2, "IF", mGameboy.cpu.irqs.readIF()},
    };

    if (ImGui::BeginTable("cpuRegsTable", 4, tableFlags))
    {
        ImGui::TableSetupColumn("Reg");
        ImGui::TableSetupColumn("Val");
        ImGui::TableSetupColumn("Reg");
        ImGui::TableSetupColumn("Val");
        ImGui::TableHeadersRow();

        auto drawTableLine = [](const RegTableEntry& entry1, const RegTableEntry& entry2) {
            ImGui::TableNextRow();
            
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(entry1.name);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(entry1.fmt, entry1.val);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text(entry2.name);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text(entry2.fmt, entry2.val);
        };

        assert(IM_ARRAYSIZE(cpuEntries) % 2 == 0);

        for (uint32_t i = 0; i < IM_ARRAYSIZE(cpuEntries); i += 2)
            drawTableLine(cpuEntries[i], cpuEntries[i + 1]);

        ImGui::EndTable();
    }
}


void App::UIDrawCpuFlagsTable()
{
    RegTableEntry cpuFlagsEntries[] = {
        { "", "Z", mGameboy.cpu.regs.flags.Z },
        { "", "N", mGameboy.cpu.regs.flags.N },
        { "", "H", mGameboy.cpu.regs.flags.H },
        { "", "C", mGameboy.cpu.regs.flags.C },
    };

    if (ImGui::BeginTable("cpuFlagsTable", 4, tableFlags))
    {
        auto drawTableLine = [](const RegTableEntry& entry1, const RegTableEntry& entry2) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text(entry1.name);
            ImGui::TableSetColumnIndex(1);
            bool bval1 = (bool)entry1.val;
            ImGui::Checkbox("", &bval1);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text(entry2.name);
            ImGui::TableSetColumnIndex(3);
            bool bval2 = (bool)entry2.val;
            ImGui::Checkbox("", &bval2);
        };

        assert(IM_ARRAYSIZE(cpuFlagsEntries) % 2 == 0);

        for (uint32_t i = 0; i < IM_ARRAYSIZE(cpuFlagsEntries); i += 2)
            drawTableLine(cpuFlagsEntries[i], cpuFlagsEntries[i + 1]);

        ImGui::EndTable();
    }
}

void App::UIDrawTimerRegTable()
{
    RegTableEntry timerEntries[] = {
         { fmtHex2, "DIV", mGameboy.timer.readDIV() },
         { fmtHex2, "TIMA", mGameboy.timer.readTIMA() },
         { fmtHex2, "TMA", mGameboy.timer.readTMA() },
         { fmtHex2, "TAC", mGameboy.timer.readTAC() }
    };

    if (ImGui::BeginTable("timerRegsTable", 4, tableFlags))
    {
        ImGui::TableSetupColumn("Reg");
        ImGui::TableSetupColumn("Val");
        ImGui::TableSetupColumn("Reg");
        ImGui::TableSetupColumn("Val");
        ImGui::TableHeadersRow();

        auto drawTableLine = [](const RegTableEntry& entry1, const RegTableEntry& entry2) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text(entry1.name);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(entry1.fmt, entry1.val);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text(entry2.name);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text(entry2.fmt, entry2.val);
        };

        assert(IM_ARRAYSIZE(timerEntries) % 2 == 0);

        for (uint32_t i = 0; i < IM_ARRAYSIZE(timerEntries); i += 2)
            drawTableLine(timerEntries[i], timerEntries[i + 1]);

        ImGui::EndTable();
    }
}

void App::UIDrawPpuRegTable()
{
    RegTableEntry ppuEntries[] = {
        { fmtHex2, "LCDC", mGameboy.ppu.readLCDC() },
        { fmtHex2, "BGP", mGameboy.ppu.readBGP() },
        { fmtHex2, "STAT", mGameboy.ppu.readSTAT() },
        { fmtHex2, "OBP0", mGameboy.ppu.readOBP0() },
        { fmtHex2, "LY", mGameboy.ppu.readLY() },
        { fmtHex2, "OBP1", mGameboy.ppu.readOBP1() },
        { fmtHex2, "LYC", mGameboy.ppu.readLYC() },
        { fmtHex2, "WX", mGameboy.ppu.readWX() },
        { fmtHex2, "SCX", mGameboy.ppu.readSCX() },
        { fmtHex2, "WY", mGameboy.ppu.readWY() },
        { fmtHex2, "SCY", mGameboy.ppu.readSCY() },
        { "%u", "currDot", mGameboy.ppu.getDotCounter() },
    };

    if (ImGui::BeginTable("ppuRegsTable", 4, tableFlags))
    {
        ImGui::TableSetupColumn("Reg");
        ImGui::TableSetupColumn("Val");
        ImGui::TableSetupColumn("Reg");
        ImGui::TableSetupColumn("Val");
        ImGui::TableHeadersRow();

        auto drawTableLine = [](const RegTableEntry& entry1, const RegTableEntry& entry2) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text(entry1.name);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(entry1.fmt, entry1.val);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text(entry2.name);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text(entry2.fmt, entry2.val);
        };

        assert(IM_ARRAYSIZE(ppuEntries) % 2 == 0);

        for (uint32_t i = 0; i < IM_ARRAYSIZE(ppuEntries); i += 2)
            drawTableLine(ppuEntries[i], ppuEntries[i + 1]);

        ImGui::EndTable();
    }
}



void App::UIDrawRegsTables()
{
    ImGui::Begin("Registers");

    ImGui::SeparatorText("CPU");
    UIDrawCpuRegTable();
    UIDrawCpuFlagsTable();

    ImGui::SeparatorText("Timer");
    UIDrawTimerRegTable();

    ImGui::SeparatorText("PPU");
    UIDrawPpuRegTable();

    ImGui::SeparatorText("Current instruction");
    ImGui::Text("%s", mGameboy.dbg.currInstructionStr().c_str());

    ImGui::End();
}

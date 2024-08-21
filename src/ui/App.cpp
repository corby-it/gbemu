

#include "App.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include <tracy/Tracy.hpp>
#include <cassert>

using namespace std::chrono;
using namespace std::chrono_literals;

using hr_clock = std::chrono::high_resolution_clock;


App::App()
    : mDisplayBuffer(Display::w, Display::h)
    , mEmulationSpeedComboIdx(static_cast<int>(EmulationSpeed::Full))
{
    // create an OpenGL texture identifier for the display image
    glGenTextures(1, &mGLDisplayTexture);
}


// Simple helper function to load an image into a OpenGL texture with common settings
static bool LoadTextureFromMatrix(const Matrix& mat, GLuint& outTexture, RgbaBuffer& buffer)
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

std::optional<nanoseconds> App::emulateFor() const
{
    // when emulating at 100% speed we have to emulate at 60 gameboy frames per second,
    // the app is limited to 60fps so we have to emulate for 1 frame

    // 1 frame at 60 fps takes 16.66666667ms so we have to emulate until the total amount of
    // m-cycles executed amounts to that number

    // an empty optional means 'unbound'

    switch (mConfig.emulationSpeed) {
    case EmulationSpeed::Quarter: return 4166667ns;
    case EmulationSpeed::Half: return 8333333ns;
    case EmulationSpeed::Full: return 16666667ns;
    default:
    case EmulationSpeed::Unbound: return {};
    }
}


bool App::emulate()
{
    ZoneScoped;

    // check which buttons are pressed once here (input state won't be 
    // updated until the next call to the app loop)
    mGameboy.joypad.action(getPressedButtons());

    // emulate for a while
    if (mConfig.emulationSpeed == EmulationSpeed::Full) {
        // when emulating at 100% speed we emulate exactly for 1 frame (or for the target gb time,
        // whichever comes first, in case the ppu is disabled and a frame is not ready for a while)

        auto targetGbTime = emulateFor();
        nanoseconds elapsedGbTime = 0ns;

        while (elapsedGbTime < targetGbTime) {
            auto [stillGoing, stepRes] = mGameboy.emulate();

            if (stepRes.frameReady || !stillGoing)
                break;

            elapsedGbTime += GameBoyClassic::machinePeriod * stepRes.cpuRes.cycles;
        }
    }
    else {
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
    }

    return true;
}


std::set<Joypad::Btn> App::getPressedButtons() const
{
    std::set<Joypad::Btn> btns;

    if (ImGui::IsKeyDown(ImGuiKey_W)) btns.insert(Joypad::Btn::Up);
    if (ImGui::IsKeyDown(ImGuiKey_A)) btns.insert(Joypad::Btn::Left);
    if (ImGui::IsKeyDown(ImGuiKey_S)) btns.insert(Joypad::Btn::Down);
    if (ImGui::IsKeyDown(ImGuiKey_D)) btns.insert(Joypad::Btn::Right);

    if (ImGui::IsKeyDown(ImGuiKey_N)) btns.insert(Joypad::Btn::A);
    if (ImGui::IsKeyDown(ImGuiKey_M)) btns.insert(Joypad::Btn::B);
    if (ImGui::IsKeyDown(ImGuiKey_Enter)) btns.insert(Joypad::Btn::Start);
    if (ImGui::IsKeyDown(ImGuiKey_0)) btns.insert(Joypad::Btn::Select);

    return btns;
}



void App::updateUI()
{
    UIDrawMenu();
    UIDrawControlWindow();
    UIDrawEmulationWindow();
    UIDrawRegsTables();
}

void App::UIDrawMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open rom file...", "CTRL+O")) {
                IGFD::FileDialogConfig config;
                config.path = ".";
                config.flags = ImGuiFileDialogFlags_Modal;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose Rom File", ".gb", config);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Alt+F4")) { closeWindow(); }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Todo...")) {} 
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
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            
            mConfig.currentRomPath = ImGuiFileDialog::Instance()->GetFilePathName();
            mConfig.loadingRes = mGameboy.loadCartridge(mConfig.currentRomPath);

            if (mConfig.loadingRes != CartridgeLoadingRes::Ok) {
                ImGui::OpenPopup("Loading failed");
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

void App::UIDrawControlWindow()
{
    ImGui::Begin("Emulation", nullptr, ImGuiWindowFlags_NoCollapse);

    // --------------------------------------------------------------------------------------------
    ImGui::SeparatorText("Controls");
    
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.71f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.f, 0.71f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.f, 0.81f, 0.8f));
    if (ImGui::Button("Stop")) { mGameboy.stop(); }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.33f, 0.71f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.33f, 0.71f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.33f, 0.81f, 0.8f));
    if (ImGui::Button("Start")) { mGameboy.play(); }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.16f, 0.71f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.16f, 0.71f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.16f, 0.81f, 0.8f));
    if (ImGui::Button("Pause")) { mGameboy.pause(); }
    ImGui::PopStyleColor(3);

    // next line

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.60f, 0.71f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.60f, 0.71f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.60f, 0.81f, 0.8f));
    if (ImGui::Button("Step in")) { mGameboy.step(); }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    if (mGameboy.cpu.callNesting() == 0)
        ImGui::BeginDisabled();

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.60f, 0.71f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.60f, 0.71f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.60f, 0.81f, 0.8f));
    if (ImGui::Button("Step out")) { mGameboy.stepReturn(); }
    ImGui::PopStyleColor(3);

    if (mGameboy.cpu.callNesting() == 0)
        ImGui::EndDisabled();

    ImGui::Text("Average step time: %u ns", (uint32_t)mGameboy.avgCycleTime().count());

    
    static const ImGuiComboFlags comboFlags = ImGuiComboFlags_None;
    static const char* speedItems[] = {
        emulationSpeedToStr(EmulationSpeed::Quarter),
        emulationSpeedToStr(EmulationSpeed::Half),
        emulationSpeedToStr(EmulationSpeed::Full),
        emulationSpeedToStr(EmulationSpeed::Unbound)
    };
    auto preview = speedItems[mEmulationSpeedComboIdx];

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

    // --------------------------------------------------------------------------------------------
    ImGui::SeparatorText("Cartridge info");

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
    ImGui::Text("Status: %s", GameBoyClassic::statusToStr(mGameboy.status));
    ImGui::Image((void*)(intptr_t)mGLDisplayTexture, ImVec2(320, 288));
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



#include "App.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"


App::App()
    : mDisplayBuffer(Display::w, Display::h)
{
    // create an OpenGL texture identifier for the display image
    glGenTextures(1, &mGLDisplayTexture);
}


// Simple helper function to load an image into a OpenGL texture with common settings
static bool LoadTextureFromMatrix(const Matrix& mat, GLuint& outTexture, RgbBuffer& buffer)
{
    glBindTexture(GL_TEXTURE_2D, outTexture);

    // turn the tile data into an RGB buffer
    mat.fillRgbBuffer(buffer);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mat.width(), mat.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.ptr());

    return true;
}

void App::update()
{
    mGameboy.emulate();

    UIDraw();
}




void App::UIDraw()
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
            mConfig.loadingRes = mGameboy.cartridge.loadRomFile(mConfig.currentRomPath);

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

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.60f, 0.71f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.60f, 0.71f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.60f, 0.81f, 0.8f));
    if (ImGui::Button("Step")) { mGameboy.step(); }
    ImGui::PopStyleColor(3);


    ImGui::Text("Average step time: %u ns", mGameboy.stepAvgTime().count());


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
    LoadTextureFromMatrix(mGameboy.ppu.display, mGLDisplayTexture, mDisplayBuffer);

    ImGui::Begin("GB Display");
    ImGui::Text("Status: %s", GameBoyClassic::statusToStr(mGameboy.status));
    ImGui::Image((void*)(intptr_t)mGLDisplayTexture, ImVec2(320, 288));
    ImGui::End();
}



struct RegTableEntry {
    const char* name;
    uint16_t val;
};

static void UIDrawSingleRegTable(const char* id, const char* title, const char* fmt, const RegTableEntry* entries, size_t count)
{
    static const ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable 
        | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

    if (!id || !title || !fmt || !entries)
        return;

    ImGui::SeparatorText(title);

    if (ImGui::BeginTable(id, 2, tableFlags))
    {
        ImGui::TableSetupColumn("Register");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        auto drawTableLine = [fmt](const char* name, uint16_t val) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(name);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(fmt, val);
        };

        for(uint32_t i =0;i<count;++i) 
            drawTableLine(entries[i].name, entries[i].val);

        ImGui::EndTable();
    }
}

void App::UIDrawRegsTables()
{
    ImGui::Begin("Registers");

    RegTableEntry cpuEntries[] = {
        { "AF", mGameboy.cpu.regs.AF() },
        { "BC", mGameboy.cpu.regs.BC() },
        { "DE", mGameboy.cpu.regs.DE() },
        { "HL", mGameboy.cpu.regs.HL() },
        { "SP", mGameboy.cpu.regs.SP },
        { "PC", mGameboy.cpu.regs.PC },
    };

    UIDrawSingleRegTable("cpuRegsTable", "CPU", "0x%04x", cpuEntries, IM_ARRAYSIZE(cpuEntries));

    RegTableEntry ppuEntries[] = {
        { "LCDC", mGameboy.ppu.readLCDC() },
        { "STAT", mGameboy.ppu.readSTAT() },
        { "SCY", mGameboy.ppu.readSCY() },
        { "SCX", mGameboy.ppu.readSCX() },
        { "LY", mGameboy.ppu.readLY() },
        { "LYC", mGameboy.ppu.readLYC() },
        { "BGP", mGameboy.ppu.readBGP() },
        { "OBP0", mGameboy.ppu.readOBP0() },
        { "OBP1", mGameboy.ppu.readOBP1() },
        { "WY", mGameboy.ppu.readWY() },
        { "WX", mGameboy.ppu.readWX() }
    };

    UIDrawSingleRegTable("ppuRegsTable", "PPU", "0x%02x", ppuEntries, IM_ARRAYSIZE(ppuEntries));


    ImGui::End();
}

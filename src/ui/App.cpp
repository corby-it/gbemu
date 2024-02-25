

#include "App.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"


App::App()
    : mDisplayBuffer(Display::w, Display::h)
{}


// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromMatrix(const Matrix& mat, GLuint* out_texture, RgbBuffer& buffer)
{
    // turn the tile data into an RGB buffer
    mat.fillRgbBuffer(buffer);

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

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

    *out_texture = image_texture;

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
            if (ImGui::MenuItem("Quit", "Alt+F4")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Todo...", false, false)) {} 
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


    // --------------------------------------------------------------------------------------------
    ImGui::SeparatorText("Cartridge info");
    
    ImGui::LabelText("Title", "%s", mGameboy.cartridge.header.title().c_str());
    ImGui::LabelText("CGB Flag", "%s", CGBFlagToStr(mGameboy.cartridge.header.cgbFlag()));
    ImGui::LabelText("New Licensee Code", "%s", mGameboy.cartridge.header.newLicenseeCode());
    ImGui::LabelText("SGB Flag", "%s", SGBFlagToStr(mGameboy.cartridge.header.sgbFlag()));
    ImGui::LabelText("Cartridge Type", "%s", cartTypeToStr(mGameboy.cartridge.header.cartType()));
    ImGui::LabelText("ROM Size", "%u", mGameboy.cartridge.header.romSize());
    ImGui::LabelText("RAM Size", "%u", mGameboy.cartridge.header.ramSize());
    ImGui::LabelText("Destination code", "%s", destCodeToStr(mGameboy.cartridge.header.destCode()));
    ImGui::LabelText("Old Licensee Code", "%s", mGameboy.cartridge.header.oldLicenseeCode());
    ImGui::LabelText("Mask ROM version", "%x", mGameboy.cartridge.header.maskRomVersionNum());
    ImGui::LabelText("Header checksum", "0x%02x", mGameboy.cartridge.header.headerChecksum());
    ImGui::LabelText("Global checksum", "0x%04x", mGameboy.cartridge.header.globalChecksum());


    ImGui::End();
}

void App::UIDrawEmulationWindow()
{

    GLuint displayTexture;
    LoadTextureFromMatrix(mGameboy.ppu.display, &displayTexture, mDisplayBuffer);


    ImGui::Begin("GB Display");
    ImGui::Text("Status: %s", GameBoyClassic::statusToStr(mGameboy.status));
    ImGui::Image((void*)(intptr_t)displayTexture, ImVec2(320, 288));
    ImGui::End();
}
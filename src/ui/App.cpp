#include "App.h"



App::App()
    : p(bus)
    , mDisplayBuffer(Display::w, Display::h)
{
    // set all palettes to default so that color id corresponds to color value
    p.regs.BGP.setToDefault();
    p.regs.OBP0.setToDefault();
    p.regs.OBP1.setToDefault();

    // set the ppu to share the same vram area for both bg/win and objects
    bool bgTileAreaFlag = true;
    p.regs.LCDC.bgWinTileDataArea = bgTileAreaFlag;
    p.regs.LCDC.objDoubleH = false;

    uint8_t bgColor = 0;
    uint8_t objColors[3] = { 1, 2, 3 };

    // background will use only one tile
    auto bgTile = p.vram.getBgTile(0, bgTileAreaFlag);
    for (uint32_t y = 0; y < TileData::h; ++y) {
        for (uint32_t x = 0; x < TileData::w; ++x) {
            bgTile.set(x, y, bgColor);
        }
    }

    // setup object tiles
    for (uint8_t i = 0; i < 3; i++) {
        auto colorVal = objColors[i];
        auto objTile = p.vram.getObjTile(i + 1, false);

        for (uint32_t y = 0; y < TileData::h; ++y) {
            for (uint32_t x = 0; x < TileData::w; ++x) {
                objTile.set(x, y, colorVal);
            }
        }
    }

    // put all oams at the bottom of the screen (y = 160)
    for (uint8_t i = 0; i < OAMRam::oamCount; ++i) {
        auto oam = p.oamRam.getOAMData(i);

        oam.tileId() = i;
        oam.x() = 0;
        oam.y() = 160;
    }

    // put oam 0 at (30, 50) in the display
    auto obj1Data = p.oamRam.getOAMData(0);
    obj1Data.x() = 58;
    obj1Data.y() = 46;
    obj1Data.tileId() = 1;

    // put oam 1 at (2, 2) in the display
    auto obj2Data = p.oamRam.getOAMData(1);
    obj2Data.x() = 10;
    obj2Data.y() = 18;
    obj2Data.tileId() = 2;

    // put oam 2 at (100, 80) in the display
    auto obj3Data = p.oamRam.getOAMData(2);
    obj3Data.x() = 108;
    obj3Data.y() = 96;
    obj3Data.tileId() = 3;

    // draw one frame
    p.stepFrame();
}


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
    GLuint displayTexture;
    LoadTextureFromMatrix(p.display, &displayTexture, mDisplayBuffer);


    ImGui::Begin("GB Display");
    ImGui::Text("size: %ux%u", p.display.width(), p.display.height());
    ImGui::Image((void*)(intptr_t)displayTexture, ImVec2(640, 576));
    ImGui::End();

}


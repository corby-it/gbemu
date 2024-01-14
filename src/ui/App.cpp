#include "App.h"



App::App()
    : mData{ 
        0xFF, 0x00,
        0x7E, 0xFF,
        0x85, 0x81,
        0x89, 0x83,
        0x93, 0x85,
        0xA5, 0x8B,
        0xC9, 0x97,
        0x7E, 0xFF }
    , mTile(0x2222, mData)
{}


static void TileDataToBuffer(const TileData& tile, uint8_t* data)
{
    uint32_t i = 0;
    for (uint8_t y = 0; y < TileData::h; ++y) {
        for (uint8_t x = 0; x < TileData::w; ++x) {
            switch (tile.get(x,y)) {
            default:
            case 0:
                data[i++] = 0;
                data[i++] = 0;
                data[i++] = 0;
                break;
            case 1:
                data[i++] = 80;
                data[i++] = 80;
                data[i++] = 80;
                break;
            case 2:
                data[i++] = 150;
                data[i++] = 150;
                data[i++] = 150;
                break;
            case 3:
                data[i++] = 255;
                data[i++] = 255;
                data[i++] = 255;
                break;
            }
        }
    }
}


// Simple helper function to load an image into a OpenGL texture with common settings
bool App::LoadTextureFromTile(const TileData tile, GLuint* out_texture)
{
    // turn the tile data into an RGB buffer
    uint8_t rgbData[TileData::w * TileData::h * 3];
    TileDataToBuffer(tile, rgbData);

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TileData::w, TileData::h, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbData);

    *out_texture = image_texture;

    return true;
}

void App::update()
{
    GLuint texture;
    LoadTextureFromTile(mTile, &texture);

    ImGui::Begin("TileData display");
    ImGui::Text("size: %ux%u", TileData::w, TileData::h);
    ImGui::Image((void*)(intptr_t)texture, ImVec2(256, 256));
    ImGui::End();
}


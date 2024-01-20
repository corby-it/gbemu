#include "App.h"



App::App()
    : mTileData{ 
        0xFF, 0x00,
        0x7E, 0xFF,
        0x85, 0x81,
        0x89, 0x83,
        0x93, 0x85,
        0xA5, 0x8B,
        0xC9, 0x97,
        0x7E, 0xFF }
    , mTile(0x2222, mTileData)
    , mTileMapData{ 0 }
    , mMap(0x2222, mTileMapData)
    , mTileRgbBuffer(TileData::w, TileData::h)
    , mMapRgbBuffer(TileMap::w, TileMap::h)
{
    uint32_t i = 0;
    for (uint32_t y = 0; y < mMap.h; ++y) {
        for (uint32_t x = 0; x < mMap.w; ++x) {
            mTileMapData[i++] = i % 256;
        }
    }
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
    GLuint tileTexture, mapTexture;
    LoadTextureFromMatrix(mTile, &tileTexture, mTileRgbBuffer);
    LoadTextureFromMatrix(mMap, &mapTexture, mMapRgbBuffer);


    ImGui::Begin("TileData display");
    ImGui::Text("size: %ux%u", TileData::w, TileData::h);
    ImGui::Image((void*)(intptr_t)tileTexture, ImVec2(256, 256));
    ImGui::End();

    ImGui::Begin("TileMap display");
    ImGui::Text("size: %ux%u", TileMap::w, TileMap::h);
    ImGui::Image((void*)(intptr_t)mapTexture, ImVec2(256, 256));
    ImGui::End();

}


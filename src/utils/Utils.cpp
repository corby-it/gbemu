
#include "Utils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize2.h>


namespace fs = std::filesystem;



void saveDisplayToFile(const GameBoyIf& gb, fs::path pngPath, uint32_t scaling)
{
    if (fs::exists(pngPath)) {
        fs::remove(pngPath);
    }
    else {
        fs::create_directories(pngPath.parent_path());
    }

    static const auto w = Display::w;
    static const auto h = Display::h;

    auto& buf = gb.ppu.display.getFrontBuf();

    if (scaling <= 1) {
        stbi_write_png(pngPath.string().c_str(), w, h, 4, static_cast<const void*>(buf.ptr()), w * 4);
    }
    else {
        if (scaling > 5)
            scaling = 5;

        void* upscaled = stbir_resize(static_cast<const void*>(buf.ptr()), w, h, w * 4,
            nullptr, w * scaling, h * scaling, w * scaling * 4,
            STBIR_RGBA, STBIR_TYPE_UINT8, STBIR_EDGE_CLAMP, STBIR_FILTER_BOX);

        stbi_write_png(pngPath.string().c_str(), w * scaling, h * scaling, 4, upscaled, w * scaling * 4);

        free(upscaled);
    }
}


bool compareDisplayWithFile(const GameBoyIf& gb, std::filesystem::path pngPath)
{
    static const auto dispW = Display::w;
    static const auto dispH = Display::h;

    int pngW, pngH, ch;
    unsigned char* pngData = stbi_load(pngPath.string().c_str(), &pngW, &pngH, &ch, 0);
    if (!pngData)
        return false;

    if (dispW != pngW || dispH != pngH || ch < 3) {
        stbi_image_free(pngData);
        return false;
    }

    auto& buf = gb.ppu.display.getFrontBuf();

    bool res = true;

    auto pngPtr = pngData;
    for (uint32_t y = 0; y < buf.h() && res; ++y) {
        for (uint32_t x = 0; x < buf.w() && res; ++x) {
            auto pngR = static_cast<uint8_t>(pngPtr[0]);
            auto pngG = static_cast<uint8_t>(pngPtr[1]);
            auto pngB = static_cast<uint8_t>(pngPtr[2]);

            auto dispPix = buf(x, y);

            if (dispPix.R != pngR || dispPix.G != pngG || dispPix.B != pngB)
                res = false;

            pngPtr += ch;
        }
    }

    stbi_image_free(pngData);

    return res;
}
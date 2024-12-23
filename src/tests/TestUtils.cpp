
#include "TestUtils.h"
#include <fstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>



namespace fs = std::filesystem;

fs::path getTestRoot()
{
    auto curr = fs::current_path();
    fs::path ret;

    for (const auto& sub : curr) {
        ret /= sub;

        if (sub == "gbemu")
            break;
    }

    ret /= "test-files";

    return ret;
}


std::vector<uint8_t> readFile(const fs::path& path)
{
    if (fs::exists(path) && fs::is_regular_file(path)) {
        auto size = fs::file_size(path);

        std::vector<uint8_t> data;
        data.resize(size);

        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        ifs.read((char*)data.data(), size);

        return data;
    }
    else {
        return {};
    }
}

void audioVecToFileStereo(const std::vector<float>& data,
    const std::string& testName,
    const std::string& ext)
{
    auto outputPath = getAudioTestFile(testName, ext);
    fs::create_directories(outputPath.parent_path());

    std::ofstream ofs(outputPath);
    if (ofs) {
        for (uint32_t i = 0; i < data.size(); i += 2)
            ofs << data[i] << ' ' << data[i + 1] << '\n';
    }
}

std::vector<float> audioFileToVecStereo(const std::string& testName, uint32_t sampleCount)
{
    std::vector<float> data;

    std::ifstream ifs(getAudioTestFile(testName));
    if (ifs) {
        data.reserve(sampleCount * 2);

        for (uint32_t i = 0; i < sampleCount; ++i) {
            float sampleL, sampleR;
            ifs >> sampleL >> sampleR;
            if (!ifs)
                break;

            data.push_back(sampleL);
            data.push_back(sampleR);
        }
    }

    return data;
}


void saveDisplayToFile(const GameBoyClassic& gb, fs::path pngPath)
{
    if (fs::exists(pngPath)) {
        fs::remove(pngPath);
    }
    else {
        fs::create_directories(pngPath.parent_path());
    }

    static const auto w = Display::w;
    static const auto h = Display::h;

    RgbaBuffer buf(w, h);
    gb.ppu.display.getFrontBuf().fillRgbaBuffer(buf);

    stbi_write_png(pngPath.string().c_str(), w, h, 4, static_cast<const void*>(buf.ptr()), w * 4);
}


bool compareDisplayWithFile(const GameBoyClassic& gb, std::filesystem::path pngPath)
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

    RgbaBuffer buf(dispW, dispH);
    gb.ppu.display.getFrontBuf().fillRgbaBuffer(buf);

    bool res = true;

    auto pngPtr = pngData;
    for (uint32_t y = 0; y < buf.h() && res; ++y) {
        for (uint32_t x = 0; x < buf.w() && res; ++x) {
            auto pngR = static_cast<uint8_t>(pngPtr[0]);
            auto pngG = static_cast<uint8_t>(pngPtr[1]);
            auto pngB = static_cast<uint8_t>(pngPtr[2]);

            auto dispPix = buf(x, y);

            if (dispPix.R() != pngR || dispPix.G() != pngG || dispPix.B() != pngB)
                res = false;

            pngPtr += ch;
        }
    }

    stbi_image_free(pngData);

    return res;
}
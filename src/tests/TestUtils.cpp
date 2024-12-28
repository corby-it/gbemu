
#include "TestUtils.h"
#include <fstream>



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


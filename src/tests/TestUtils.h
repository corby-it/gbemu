

#ifndef GBEMU_SRC_TESTS_TESTUTILS_H_
#define GBEMU_SRC_TESTS_TESTUTILS_H_

#include "gb/Ppu.h"
#include "gb/GameBoyCore.h"
#include <vector>
#include <filesystem>
#include <functional>
#include <fstream>

std::filesystem::path getTestRoot();

std::vector<uint8_t> readFile(const std::filesystem::path& path);




inline std::filesystem::path getAudioTestRoot()
{
    return getTestRoot() / "audio";
}

inline std::filesystem::path getAudioTestFile(const std::string& testName, const std::string& ext = ".sample")
{
    auto fname = testName + ext;
    return getAudioTestRoot() / fname;
}


inline float convertForAudacity(uint8_t sample) {
    // samples come in in the range 0-15 and must be mapped between -1.f and 1.f
    float val = static_cast<float>(sample);
    return (val - 7.5f) / 7.5f;
}


template<typename SampleT>
void audioVecToFileMono(const std::vector<uint8_t>& data, 
        const std::string& testName, 
        const std::string& ext = ".sample",
        std::function<SampleT(uint8_t)> op = std::function<SampleT(uint8_t)>())
{
    auto outputPath = getAudioTestFile(testName, ext);
    std::filesystem::create_directories(outputPath.parent_path());

    std::ofstream ofs(outputPath);
    if (ofs) {
        for (const auto& sample : data) {
            if(op)
                ofs << op(sample) << "\n";
            else 
                ofs << (unsigned int)sample << "\n";
        }
    }
}


void audioVecToFileStereo(const std::vector<float>& data,
    const std::string& testName,
    const std::string& ext = ".sample");




template<typename SampleT>
std::vector<SampleT> audioFileToVecMono(const std::string& testName, uint32_t sampleCount)
{
    std::vector<SampleT> data;

    std::ifstream ifs(getAudioTestFile(testName));
    if (ifs) {
        data.reserve(sampleCount);

        for (uint32_t i = 0; i < sampleCount; ++i) {
            SampleT val;
            ifs >> val;
            if (!ifs)
                break;

            data.push_back((SampleT)val);
        }
    }

    return data;
}

std::vector<float> audioFileToVecStereo(const std::string& testName, uint32_t sampleCount);




void saveDisplayToFile(const GameBoyClassic& gb, std::filesystem::path pngPath);

bool compareDisplayWithFile(const GameBoyClassic& gb, std::filesystem::path pngPath);


#endif // GBEMU_SRC_TESTS_TESTUTILS_H_
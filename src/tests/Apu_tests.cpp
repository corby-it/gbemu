
#include "gb/Apu.h"
#include "gb/GbCommons.h"
#include "TestUtils.h"
#include <vector>
#include <algorithm>
#include <doctest/doctest.h>

namespace fs = std::filesystem;
namespace aregs = mmap::regs::audio;

// set the sample rate at 44.1khz for the tests
// for each test generate 2 seconds of stereo audio
static constexpr uint32_t sampleRate = 44100;
static constexpr uint32_t sampleCount = sampleRate * 2;
static constexpr uint32_t sampleSize = sampleCount * 2;


TEST_CASE("Square wave channel")
{
    std::string testName;
    std::vector<float> generatedAudio;
    generatedAudio.resize(sampleSize);


    APU apu(sampleRate);
    
    
    SUBCASE("Generate two square waves at different frequencies pannel L and R") {
        testName = "apu01-682hz-left-170hz-right";

        // enable the apu
        apu.write(aregs::NR52, 0x80);
        // set channel 1 to the left and channel 2 to the right
        apu.write(aregs::NR51, 0x12);
        // set volumes to 7, both L and R
        apu.write(aregs::NR50, 0x77);

        // setup channel 1 to produce a 682hz square wave no sweep
        apu.write(aregs::NR10, 0x00);
        // write to reg NR11 to select 50% duty cycle, timer length is 0
        apu.write(aregs::NR11, 0x80);
        // write to reg 2 to set the initial volume at F with no envelope and enable the dac
        apu.write(aregs::NR12, 0xf0);
        // write to reg 3 and 4 to set the period and trigger the channel (period is 0x740)
        apu.write(aregs::NR13, 0x40);
        apu.write(aregs::NR14, 0x87);

        // setup channel 2 to produce a 170hz square wave
        // write to reg 1 to select 50% duty cycle, timer length is 0
        apu.write(aregs::NR21, 0x80);
        // write to reg 2 to set the initial volume at F with no envelope and enable the dac
        apu.write(aregs::NR22, 0xf0);
        // write to reg 3 and 4 to set the period and trigger the channel (period is 0x500)
        apu.write(aregs::NR23, 0x00);
        apu.write(aregs::NR24, 0x85);
    }


    for (uint32_t i = 0; i < sampleCount; ++i) {
        while (!apu.step(1)) {}
        generatedAudio[i * 2] = apu.getOutputL();
        generatedAudio[i * 2 + 1] = apu.getOutputR();
    }

    audioVecToFileStereo(generatedAudio, testName);

}
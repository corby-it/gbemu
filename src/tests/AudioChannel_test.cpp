
#include "gb/AudioChannel.h"
#include "TestUtils.h"
#include <vector>
#include <fstream>
#include <algorithm>
#include <doctest/doctest.h>

namespace fs = std::filesystem;


// when this macro is defined the samples audio files will be generated
#define GENERATE_AUDIO_FILES


// set the sample rate at 44.1khz for the tests and
// for each test generate 2 seconds of mono audio
static constexpr uint32_t sampleRate = 44100;
static constexpr uint32_t sampleCount = sampleRate * 2;




TEST_CASE("Square wave channel")
{
    std::string testName;
    std::vector<uint8_t> generatedAudio;
    generatedAudio.resize(sampleCount);

    SquareWaveChannel sq;
    // use the internal frame sequencer for the tests
    sq.setDownsamplingFreq(sampleRate);
    sq.enableInternalFrameSequencer(true);

    SUBCASE("With sweeping modulation disabled") {

        SUBCASE("Generate fixed tone 682 HZ") {
            testName = "sq01-fixed-tone-682hz";

            // write to reg 1 to select 50% duty cycle, timer length is 0
            sq.writeReg1(0b10000000);
            // write to reg 2 to set the initial volume at F with no envelope and enable the dac
            sq.writeReg2(0xf0);
            // write to reg 3 and 4 to set the period and trigger the channel (period is 0x740)
            sq.writeReg3(0x40);
            sq.writeReg4(0x87);
        }

        SUBCASE("Generate fixed tone 170 HZ") {
            testName = "sq02-fixed-tone-170hz";

            // write to reg 1 to select 50% duty cycle, timer length is 0
            sq.writeReg1(0b10000000);
            // write to reg 2 to set the initial volume at F with no envelope and enable the dac
            sq.writeReg2(0xf0);
            // write to reg 3 and 4 to set the period and trigger the channel (period is 0x500)
            sq.writeReg3(0x00);
            sq.writeReg4(0x85);
        }

        SUBCASE("Generate fixed tone 170 HZ with slow decreasing envelope") {
            testName = "sq03-fixed-tone-170hz-env-down-slow";

            // write to reg 1 to select 50% duty cycle, timer length is 0
            sq.writeReg1(0b10000000);
            // write to reg 2 to set the initial volume at F with envelope pace 7 (down) and enable the dac
            sq.writeReg2(0xf7);
            // write to reg 3 and 4 to set the period and trigger the channel (period is 0x500)
            sq.writeReg3(0x00);
            sq.writeReg4(0x85);
        }

        SUBCASE("Generate fixed tone 170 HZ with fast decreasing envelope") {
            testName = "sq04-fixed-tone-170hz-env-down-fast";

            // write to reg 1 to select 50% duty cycle, timer length is 0
            sq.writeReg1(0b10000000);
            // write to reg 2 to set the initial volume at F with envelope pace 2 (down) and enable the dac
            sq.writeReg2(0xf2);
            // write to reg 3 and 4 to set the period and trigger the channel (period is 0x500)
            sq.writeReg3(0x00);
            sq.writeReg4(0x85);
        }

        SUBCASE("Generate fixed tone 170 HZ with slow increasing envelope") {
            testName = "sq05-fixed-tone-170hz-env-up-slow";

            // write to reg 1 to select 50% duty cycle, timer length is 0
            sq.writeReg1(0b10000000);
            // write to reg 2 to set the initial volume at 0 with envelope pace 7 (up) and enable the dac
            sq.writeReg2(0x0f);
            // write to reg 3 and 4 to set the period and trigger the channel (period is 0x500)
            sq.writeReg3(0x00);
            sq.writeReg4(0x85);
        }

        SUBCASE("Generate fixed tone 170 HZ with fast increasing envelope") {
            testName = "sq06-fixed-tone-170hz-env-up-fast";

            // write to reg 1 to select 50% duty cycle, timer length is 0
            sq.writeReg1(0b10000000);
            // write to reg 2 to set the initial volume at 0 with envelope pace 2 (up) and enable the dac
            sq.writeReg2(0x0a);
            // write to reg 3 and 4 to set the period and trigger the channel (period is 0x500)
            sq.writeReg3(0x00);
            sq.writeReg4(0x85);
        }

        SUBCASE("Generate fixed tone 170 HZ with long length timer") {
            testName = "sq07-fixed-tone-170hz-timer-long";

            // the length timer is clocked at 256hz so with an initial length timer value of 0, the timer 
            // will tick for 0,25s (from 0 to 64)

            // write to reg 1 to select 50% duty cycle, timer length is 0x00
            sq.writeReg1(0x80);
            // write to reg 2 to set the initial volume at F with no envelope and enable the dac
            sq.writeReg2(0xf0);
            // write to reg 3 and 4 to set the period and trigger the channel and the timer (period is 0x500)
            sq.writeReg3(0x00);
            sq.writeReg4(0xc5);
        }

        SUBCASE("Generate fixed tone 170 HZ with short length timer") {
            testName = "sq08-fixed-tone-170hz-timer-short";

            // the length timer is clocked at 256hz so with an initial length timer value of 53, the timer 
            // will tick for 0,04s (from 53 to 64)

            // write to reg 1 to select 50% duty cycle, timer length is 0x35
            sq.writeReg1(0xb5);
            // write to reg 2 to set the initial volume at F with no envelope and enable the dac
            sq.writeReg2(0xf0);
            // write to reg 3 and 4 to set the period and trigger the channel and the timer (period is 0x500)
            sq.writeReg3(0x00);
            sq.writeReg4(0xc5);
        }

        SUBCASE("Generate fixed tone 170 HZ with long length timer and long envelope") {
            testName = "sq09-fixed-tone-170hz-timer-long-env-down-slow";

            // the length timer is clocked at 256hz so with an initial length timer value of 0, the timer 
            // will tick for 0,25s (from 0 to 64)

            // write to reg 1 to select 50% duty cycle, timer length is 0x00
            sq.writeReg1(0x80);
            // write to reg 2 to set the initial volume at F with envelope pace 7 (down) and enable the dac
            sq.writeReg2(0xf7);
            // write to reg 3 and 4 to set the period and trigger the channel and the timer (period is 0x500)
            sq.writeReg3(0x00);
            sq.writeReg4(0xc5);
        }

        SUBCASE("Generate fixed tone 170 HZ and try sweeping up") {
            testName = "sq10-fixed-tone-170hz-try-sweep";

            // try to enable the sweep modulation (nothing should happen though)
            sq.writeReg0(0x2a);
            // write to reg 1 to select 50% duty cycle, timer length is 0
            sq.writeReg1(0b10000000);
            // write to reg 2 to set the initial volume at F with no envelope and enable the dac
            sq.writeReg2(0xf0);
            // write to reg 3 and 4 to set the period and trigger the channel (period is 0x500)
            sq.writeReg3(0x00);
            sq.writeReg4(0x85);
        }
    }


    SUBCASE("With sweeping modulation enabled") {
        // enable sweep modulation in the channel
        sq.enableSweepModulation(true);

        SUBCASE("Generate fixed tone 170 HZ with sweep up slow") {
            testName = "sq11-fixed-tone-170hz-sweep-up-slow";

            // enable sweep up slowly
            sq.writeReg0(0x27);
            // write to reg 1 to select 50% duty cycle, timer length is 0
            sq.writeReg1(0b10000000);
            // write to reg 2 to set the initial volume at F with no envelope and enable the dac
            sq.writeReg2(0xf0);
            // write to reg 3 and 4 to set the period and trigger the channel (period is 0x500)
            sq.writeReg3(0x00);
            sq.writeReg4(0x85);
        }

        SUBCASE("Generate fixed tone 682 HZ with sweep down slow") {
            testName = "sq12-fixed-tone-682hz-sweep-down-slow";

            // enable sweep down slowly
            sq.writeReg0(0x2d);
            // write to reg 1 to select 50% duty cycle, timer length is 0
            sq.writeReg1(0b10000000);
            // write to reg 2 to set the initial volume at F with no envelope and enable the dac
            sq.writeReg2(0xf0);
            // write to reg 3 and 4 to set the period and trigger the channel (period is 0x740)
            sq.writeReg3(0x40);
            sq.writeReg4(0x87);
        }

        SUBCASE("Generate fixed tone 682 HZ with sweep down slow and envelope down slow") {
            testName = "sq13-fixed-tone-682hz-sweep-down-slow-env-down-slow";

            // enable sweep down slowly
            sq.writeReg0(0x2d);
            // write to reg 1 to select 50% duty cycle, timer length is 0
            sq.writeReg1(0b10000000);
            // write to reg 2 to set the initial volume at F with no envelope and enable the dac
            sq.writeReg2(0xf7);
            // write to reg 3 and 4 to set the period and trigger the channel (period is 0x740)
            sq.writeReg3(0x40);
            sq.writeReg4(0x87);
        }
    }


    // generate audio
    for (uint32_t i = 0; i < generatedAudio.size(); ++i) {
        while (!sq.step()) {}
        generatedAudio[i] = sq.getOutput();
    }

#ifdef GENERATE_AUDIO_FILES
    //audioVecToFileMono<uint8_t>(generatedAudio, testName);
    //audioVecToFileMono<float>(generatedAudio, testName, ".txt", convertForAudacity);
#endif // GENERATE_AUDIO_FILES


    // read the corresponding test file and compare the results

    auto expectedAudio = audioFileToVecMono<uint32_t>(testName, sampleCount);

    REQUIRE(expectedAudio.size() == sampleCount);

    CHECK(std::equal(generatedAudio.begin(), generatedAudio.end(), expectedAudio.begin()));
}



TEST_CASE("Noise channel")
{
    std::string testName;
    std::vector<uint8_t> generatedAudio;
    generatedAudio.resize(sampleCount);

    NoiseChannel noise;
    // use the internal frame sequencer for the tests
    noise.setDownsamplingFreq(sampleRate);
    noise.enableInternalFrameSequencer(true);


    SUBCASE("Generate noise at constant volume") {
        testName = "ns01-constant-volume";

        // set a random value in reg 1 for the timer length
        noise.writeReg1(0x04);
        // write to reg 2 to set the initial volume at A with no envelope and enable the dac
        noise.writeReg2(0xA0);
        // write to reg 3 to set shift = 3, divider = 2 and width 16
        noise.writeReg3(0x32);
        // write to reg 4 to trigger the channel, no length timer
        noise.writeReg4(0x80);
    }

    SUBCASE("Generate noise at constant volume at higher freq") {
        testName = "ns02-constant-volume-higher";

        // set a random value in reg 1 for the timer length
        noise.writeReg1(0x04);
        // write to reg 2 to set the initial volume at A with no envelope and enable the dac
        noise.writeReg2(0xA0);
        // write to reg 3 to set shift = 1, divider = 1 and width 16
        noise.writeReg3(0x11);
        // write to reg 4 to trigger the channel, no length timer
        noise.writeReg4(0x80);
    }

    SUBCASE("Generate noise with envelope down") {
        testName = "ns03-env-down";

        // set a random value in reg 1 for the timer length
        noise.writeReg1(0x04);
        // write to reg 2 to set the initial volume at A with no envelope and enable the dac
        noise.writeReg2(0xf7);
        // write to reg 3 to set shift = 1, divider = 1 and width 16
        noise.writeReg3(0x11);
        // write to reg 4 to trigger the channel, no length timer
        noise.writeReg4(0x80);
    }

    SUBCASE("Generate noise with long length timer") {
        testName = "ns03-length-timer-long";

        // set a value of 0 in reg 1 for the timer length
        noise.writeReg1(0x00);
        // write to reg 2 to set the initial volume at A with no envelope and enable the dac
        noise.writeReg2(0xA0);
        // write to reg 3 to set shift = 1, divider = 1 and width 16
        noise.writeReg3(0x11);
        // write to reg 4 to trigger the channel, enable length timer
        noise.writeReg4(0xC0);
    }


    // generate audio
    for (uint32_t i = 0; i < generatedAudio.size(); ++i) {
        while (!noise.step()) {}
        generatedAudio[i] = noise.getOutput();
    }

#ifdef GENERATE_AUDIO_FILES
    //audioVecToFileMono<uint8_t>(generatedAudio, testName);
    //audioVecToFileMono<float>(generatedAudio, testName, ".txt", convertForAudacity);
#endif // GENERATE_AUDIO_FILES

    // read the corresponding test file and compare the results
    auto expectedAudio = audioFileToVecMono<uint32_t>(testName, sampleCount);

    REQUIRE(expectedAudio.size() == sampleCount);

    CHECK(std::equal(generatedAudio.begin(), generatedAudio.end(), expectedAudio.begin()));
}


TEST_CASE("User wave channel")
{
    std::string testName;
    std::vector<uint8_t> generatedAudio;
    generatedAudio.resize(sampleCount);

    auto ramStart = mmap::regs::audio::wave_ram::start;

    UserWaveChannel uw;
    // use the internal frame sequencer for the tests
    uw.setDownsamplingFreq(sampleRate);
    uw.enableInternalFrameSequencer(true);

    // wave ram data from super mario land 
    uw.writeWaveRam(ramStart + 0x0, 0x01);
    uw.writeWaveRam(ramStart + 0x1, 0x23);
    uw.writeWaveRam(ramStart + 0x2, 0x56);
    uw.writeWaveRam(ramStart + 0x3, 0x78);
    uw.writeWaveRam(ramStart + 0x4, 0x99);
    uw.writeWaveRam(ramStart + 0x5, 0x98);
    uw.writeWaveRam(ramStart + 0x6, 0x76);
    uw.writeWaveRam(ramStart + 0x7, 0x67);
    uw.writeWaveRam(ramStart + 0x8, 0x9A);
    uw.writeWaveRam(ramStart + 0x9, 0xDF);
    uw.writeWaveRam(ramStart + 0xA, 0xFE);
    uw.writeWaveRam(ramStart + 0xB, 0xC9);
    uw.writeWaveRam(ramStart + 0xC, 0x85);
    uw.writeWaveRam(ramStart + 0xD, 0x42);
    uw.writeWaveRam(ramStart + 0xE, 0x11);
    uw.writeWaveRam(ramStart + 0xF, 0x00);


    SUBCASE("PLay at 85hz - 100% volume") {
        testName = "uw01-85hz-volume-100";

        // enable the dac
        uw.writeReg0(0x80);
        // set a random value in reg 1 for the timer length
        uw.writeReg1(0x04);
        // write to reg 2 to set the volume to 100%
        uw.writeReg2(0x20);
        // write to reg 3 and 4 to set the period to 500 and trigger the channel (no length timer)
        uw.writeReg3(0x00);
        uw.writeReg4(0x85);
    }

    SUBCASE("PLay at 341hz - 100% volume") {
        testName = "uw01-341hz-volume-100";

        // enable the dac
        uw.writeReg0(0x80);
        // set a random value in reg 1 for the timer length
        uw.writeReg1(0x04);
        // write to reg 2 to set the volume to 100%
        uw.writeReg2(0x20);
        // write to reg 3 and 4 to set the period to 740 and trigger the channel (no length timer)
        uw.writeReg3(0x40);
        uw.writeReg4(0x87);
    }



    // generate audio
    for (uint32_t i = 0; i < generatedAudio.size(); ++i) {
        while (!uw.step()) {}
        generatedAudio[i] = uw.getOutput();
    }

#ifdef GENERATE_AUDIO_FILES
    //audioVecToFileMono<uint8_t>(generatedAudio, testName);
    //audioVecToFileMono<float>(generatedAudio, testName, ".txt", convertForAudacity);
#endif // GENERATE_AUDIO_FILES


    // read the corresponding test file and compare the results
    auto expectedAudio = audioFileToVecMono<uint32_t>(testName, sampleCount);

    REQUIRE(expectedAudio.size() == sampleCount);

    CHECK(std::equal(generatedAudio.begin(), generatedAudio.end(), expectedAudio.begin()));
}
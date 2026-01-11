
#include "TestUtils.h"
#include "utils/Utils.h"
#include "gb/GameBoyCore.h"
#include "doctest/doctest.h"
#include <chrono>
#include <functional>
#include <cstdio>
#include <array>


namespace fs = std::filesystem;
using namespace std::chrono;
using namespace std::placeholders;
using hr_clock = std::chrono::high_resolution_clock;



static const fs::path testFilesRoot = getTestRoot();
static const fs::path romsRoot = testFilesRoot / "cgb-dmg-acid";


TEST_CASE("cgb-acid2 rom") {
    // cgb-acid2 can only be verified by comparing the screen 
    // with the screenshot of a successful test

    // for a description of what the test checks see:
    // https://github.com/mattcurrie/cgb-acid2

    GameBoy gb;
    gb.setType(GbType::CGB);


    fs::path romPath = romsRoot / "cgb-acid2.gbc";
    fs::path screenshotPath = romsRoot / "cgb-acid2.png";


    REQUIRE(gb.loadCartridge(romPath) == CartridgeLoadingRes::Ok);

    gb.play();

    // running for 1 emulated second should be enough to run the test

    uint64_t cycles = 0;
    auto startTp = hr_clock::now();
    while (cycles < GameBoy::timeToCyclesBase(1s)) {
        auto [stillGoing, stepsRes] = gb.emulate();
        cycles += stepsRes.cpuRes.cycles;
    }
    auto elapsed = duration_cast<microseconds>(hr_clock::now() - startTp).count();

    auto result = compareDisplayWithFile(gb, screenshotPath);

    INFO("Test name: cgb-acid2.gbc");
    INFO("This test took ", elapsed, "us");
    INFO("Executed ", cycles, " cycles");

    CHECK(result);

}
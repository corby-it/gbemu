
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
    // cgb-acid2 test can only be verified by comparing the screen 
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


TEST_CASE("dmg-acid2 roms") {
    // dmg-acid2 tests can only be verified by comparing the screen 
    // with the screenshot of a successful test

    // for a description of what the tests check see:
    // https://github.com/mattcurrie/dmg-acid2

    GameBoy gb;
    gb.setType(GbType::CGB);

    fs::path romRelPath = "";
    fs::path screenRelPath = "";

    SUBCASE("") {
        gb.setType(GbType::DMG);
        romRelPath = "dmg-acid2.gb";
        screenRelPath = "dmg-acid2.png";
    }
    SUBCASE("") {
        gb.setType(GbType::CGB);
        romRelPath = "dmg-acid2.gb";
        screenRelPath = "dmg-acid2-cgb-compat-mode.png";
    }


    fs::path romPath = romsRoot / romRelPath;
    fs::path screenshotPath = romsRoot / screenRelPath;


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

    INFO("Test name: ", romRelPath.string());
    INFO("Expected screenshot name: ", screenRelPath.string());
    INFO("This test took ", elapsed, "us");
    INFO("Executed ", cycles, " cycles");

    CHECK(result);

}

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
static const fs::path romsRoot = testFilesRoot / "MagenTests-0.5.0/roms";
static const fs::path resultsRoot = testFilesRoot / "MagenTests-0.5.0/expected-results";


TEST_CASE("MagenTests roms") {
    // MagenTests can only be verified by comparing the screen 
    // with the screenshot of a successful test

    // for a description of what each test does check out the MagenTests README
    // see also: https://github.com/alloncm/MagenTests

    GameBoy gb;
    gb.setType(GbType::CGB);

    fs::path romRelPath = "";
    fs::path screenRelPath = "";
    
    SUBCASE("") { 
        romRelPath = "bg_oam_priority.gbc";
        screenRelPath = "bg_oam_priority.png";
    }
    SUBCASE("") {
        romRelPath = "hblank_vram_dma.gbc";
        screenRelPath = "hblank_vram_dma.png";
    }
    SUBCASE("") {
        romRelPath = "key0_lock_after_boot.gbc";
        screenRelPath = "key0_lock_after_boot.png";
    }
    SUBCASE("") {
        romRelPath = "mbc_oob_sram_mbc1.gbc";
        screenRelPath = "mbc_oob_sram_mbc1.png";
    }
    SUBCASE("") {
        romRelPath = "mbc_oob_sram_mbc3.gbc";
        screenRelPath = "mbc_oob_sram_mbc1.png";
    }
    SUBCASE("") {
        romRelPath = "mbc_oob_sram_mbc5.gbc";
        screenRelPath = "mbc_oob_sram_mbc5.png";
    }
    SUBCASE("") {
        romRelPath = "oam_internal_priority.gbc";
        screenRelPath = "oam_internal_priority.png";
    }
    SUBCASE("") {
        romRelPath = "ppu_disabled_state.gbc";
        screenRelPath = "ppu_disabled_state.png";
    }
    

    fs::path romPath = romsRoot / romRelPath;
    fs::path screenshotPath = resultsRoot / screenRelPath;

    
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
    INFO("This test took ", elapsed, "us");
    INFO("Executed ", cycles, " cycles");

    CHECK(result);

}



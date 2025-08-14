
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
static const fs::path blarggsFilesRoot = testFilesRoot / "blarggs-test-roms";


// Blargg's test shouldn't run for more than 60 emulated seconds:
// source: https://github.com/c-sp/game-boy-test-roms/blob/master/src/howto/blargg.md

static constexpr uint64_t cyclesLimit = GameBoyIf::timeToCyclesBase(60s);


// Blargg's test roms don't have a single way to detect success or failure
// so each case is different, sometimes results are transmitted over the serial port
// while some other times a memory region can be monitored to get the result


enum class TestStatus {
    NotStarted,
    Running,
    Success,
    Failed
};

class SerialResultChecker {
public:
    SerialResultChecker()
        : status(TestStatus::Running)
        , failedTest(0)
    {}


    void onData(uint8_t data) {
        // some test roms print human-readable results to the display
        // but also transmit the same text on the serial port 

        // at the end of a successful test, the rom prints "Passed\n"
        // at the end of a failed test, the rom prints "Failed #nn\n"
        // where 'nn' is the number of the failed test
        // some roms can also fail with just "Failed\n" and no test number

        tmp.push_back(data);

        if (data == '\n') {
            if (tmp == "Passed\n") {
                status = TestStatus::Success;
            }
            else if (tmp == "Failed\n") {
                status = TestStatus::Failed;
                failedTest = -1;
            }
            else if (sscanf(tmp.c_str(), "Failed #%d\n", &failedTest) == 1) {
                status = TestStatus::Failed;
            }
            else {
                msg.append(tmp);
                tmp.clear();
            }
        }

    }


    TestStatus status;
    int failedTest;
    std::string msg;
    std::string tmp;

};


class MemoryResultChecker {
public:
    MemoryResultChecker(GameBoyClassic& gameboy)
        : gb(gameboy)
        , status(TestStatus::NotStarted)
        , failedTest(0)
    {}

    void check() {
        // Some test roms use the memory at 0xA000 to store the status of the test

        // data is written starting at address 0xA000:
        // - the first byte is the status, at the beginning it's 0x00,
        //      while the test is running the value is 0x80,
        //      at the end the value is the result of the test, 0x00 means success while
        //      any other value is the number of the test that failed
        // - the following 3 bytes are a signature, the values are always DE B0 61
        // - after the signature there is a C string with the same message written on the screen

        std::array<uint8_t, 4> mem = {
            gb.read8(0xA000),
            gb.read8(0xA001),
            gb.read8(0xA002),
            gb.read8(0xA003),
        };

        switch (status) {
        case TestStatus::NotStarted:
            // while the test is not yet started we wait for the first byte to switch from 0x00 to 0x80
            // and for the signature to be there
            if (mem == std::array<uint8_t, 4>{ 0x80, 0xDE, 0xB0, 0x61 })
                status = TestStatus::Running;
            break;
        
        case TestStatus::Running:
            // while the test is running we wait for the first 4 bytes to change 
            if (mem == std::array<uint8_t, 4>{ 0x80, 0xDE, 0xB0, 0x61 }) {
                status = TestStatus::Running;
            }
            else if (mem == std::array<uint8_t, 4>{ 0x00, 0xDE, 0xB0, 0x61 }) {
                status = TestStatus::Success;
            }
            else {
                status = TestStatus::Failed;
                failedTest = mem[0];

                uint8_t b = '0';
                for (uint16_t addr = 0xA004; b != 0; ++addr) {
                    b = gb.read8(addr);
                    if (b != 0)
                        msg.push_back(b);
                }
            }
            break;

        case TestStatus::Success: break;
        case TestStatus::Failed: break;
        default:
            break;
        }
    }


    GameBoyClassic& gb;
    TestStatus status;
    int failedTest;
    std::string msg;
};



TEST_CASE("Blargg's test roms - Serial monitor roms") {

    fs::path romRelPath = "";

    // CPU instructions test roms 
    SUBCASE("") { romRelPath = "cpu_instrs/individual/01-special.gb"; }
    SUBCASE("") { romRelPath = "cpu_instrs/individual/02-interrupts.gb"; }
    SUBCASE("") { romRelPath = "cpu_instrs/individual/03-op sp,hl.gb"; }
    SUBCASE("") { romRelPath = "cpu_instrs/individual/04-op r,imm.gb"; }
    SUBCASE("") { romRelPath = "cpu_instrs/individual/05-op rp.gb"; }
    SUBCASE("") { romRelPath = "cpu_instrs/individual/06-ld r,r.gb"; }
    SUBCASE("") { romRelPath = "cpu_instrs/individual/07-jr,jp,call,ret,rst.gb"; }
    SUBCASE("") { romRelPath = "cpu_instrs/individual/08-misc instrs.gb"; }
    SUBCASE("") { romRelPath = "cpu_instrs/individual/09-op r,r.gb"; }
    SUBCASE("") { romRelPath = "cpu_instrs/individual/10-bit ops.gb"; }
    SUBCASE("") { romRelPath = "cpu_instrs/individual/11-op a,(hl).gb"; }

    // Instruction timing test roms
    SUBCASE("") { romRelPath = "instr_timing/instr_timing.gb"; }

    // Memory timing test roms
    //SUBCASE("") { romRelPath = "mem_timing/individual/01-read_timing.gb"; }
    //SUBCASE("") { romRelPath = "mem_timing/individual/02-write_timing.gb"; }
    //SUBCASE("") { romRelPath = "mem_timing/individual/03-modify_timing.gb"; }


    fs::path romPath = blarggsFilesRoot / romRelPath;

    SerialResultChecker checker;

    GameBoyClassic gb;
    REQUIRE(gb.loadCartridge(romPath) == CartridgeLoadingRes::Ok);

    gb.serial.setSerialDataReadyCb(std::bind(&SerialResultChecker::onData, &checker, _1));
    gb.play();


    uint64_t cycles = 0;
    auto startTp = hr_clock::now();
    while (cycles < cyclesLimit && checker.status == TestStatus::Running) {
        auto [stillGoing, stepsRes] = gb.emulate();
        cycles += stepsRes.cpuRes.cycles;
    }
    auto elapsed = duration_cast<microseconds>(hr_clock::now() - startTp).count();

    
    INFO("Test name: ", romRelPath.string());
    INFO("This test took ", elapsed, "us");
    INFO("Executed ", cycles, " cycles");
    INFO("Failed test: ", checker.failedTest);
    INFO("On-screen message: ", checker.msg);

    CHECK(cycles < cyclesLimit);
    CHECK(checker.status == TestStatus::Success);
}


TEST_CASE("Blargg's test roms - Memory monitor roms") {

    fs::path romRelPath = "";

    // OAM bug roms
    //SUBCASE("") { romRelPath = "oam_bug/rom_singles/1-lcd_sync.gb"; }
    //SUBCASE("") { romRelPath = "oam_bug/rom_singles/2-causes.gb"; }
    SUBCASE("") { romRelPath = "oam_bug/rom_singles/3-non_causes.gb"; }
    //SUBCASE("") { romRelPath = "oam_bug/rom_singles/4-scanline_timing.gb"; }
    //SUBCASE("") { romRelPath = "oam_bug/rom_singles/5-timing_bug.gb"; }
    SUBCASE("") { romRelPath = "oam_bug/rom_singles/6-timing_no_bug.gb"; }
    //SUBCASE("") { romRelPath = "oam_bug/rom_singles/7-timing_effect.gb"; }
    //SUBCASE("") { romRelPath = "oam_bug/rom_singles/8-instr_effect.gb"; }

    // audio roms
    SUBCASE("") { romRelPath = "dmg_sound/rom_singles/01-registers.gb"; }
    SUBCASE("") { romRelPath = "dmg_sound/rom_singles/02-len ctr.gb"; }
    //SUBCASE("") { romRelPath = "dmg_sound/rom_singles/03-trigger.gb"; }
    SUBCASE("") { romRelPath = "dmg_sound/rom_singles/04-sweep.gb"; }
    SUBCASE("") { romRelPath = "dmg_sound/rom_singles/05-sweep details.gb"; }
    SUBCASE("") { romRelPath = "dmg_sound/rom_singles/06-overflow on trigger.gb"; }
    SUBCASE("") { romRelPath = "dmg_sound/rom_singles/07-len sweep period sync.gb"; }
    //SUBCASE("") { romRelPath = "dmg_sound/rom_singles/08-len ctr during power.gb"; }
    //SUBCASE("") { romRelPath = "dmg_sound/rom_singles/09-wave read while on.gb"; }
    //SUBCASE("") { romRelPath = "dmg_sound/rom_singles/10-wave trigger while on.gb"; }
    SUBCASE("") { romRelPath = "dmg_sound/rom_singles/11-regs after power.gb"; }
    //SUBCASE("") { romRelPath = "dmg_sound/rom_singles/12-wave write while on.gb"; }

    // Memory timing test roms
    //SUBCASE("") { romRelPath = "mem_timing-2/rom_singles/01-read_timing.gb"; }
    //SUBCASE("") { romRelPath = "mem_timing-2/rom_singles/02-write_timing.gb"; }
    //SUBCASE("") { romRelPath = "mem_timing-2/rom_singles/03-modify_timing.gb"; }

    fs::path romPath = blarggsFilesRoot / romRelPath;


    GameBoyClassic gb;
    MemoryResultChecker checker(gb);

    REQUIRE(gb.loadCartridge(romPath) == CartridgeLoadingRes::Ok);

    gb.play();


    uint64_t cycles = 0;
    auto startTp = hr_clock::now();
    while (cycles < cyclesLimit && (checker.status == TestStatus::NotStarted || checker.status == TestStatus::Running)) {
        auto [stillGoing, stepsRes] = gb.emulate();
        cycles += stepsRes.cpuRes.cycles;

        checker.check();
    }
    auto elapsed = duration_cast<microseconds>(hr_clock::now() - startTp).count();


    INFO("Test name: ", romRelPath.string());
    INFO("This test took ", elapsed, "us");
    INFO("Executed ", cycles, " cycles");
    INFO("Failed test: ", checker.failedTest);
    INFO("On-screen message: ", checker.msg);

    CHECK(cycles < cyclesLimit);
    CHECK(checker.status == TestStatus::Success);
}



TEST_CASE("Blargg's test roms - HALT bug") {
    // The result of the HALT bug rom can only be checked by comparing 
    // the content of the screen with a screenshot of a successful test

    fs::path romRelPath = "halt_bug.gb";
    fs::path romPath = blarggsFilesRoot / romRelPath;

    GameBoyClassic gb;
    REQUIRE(gb.loadCartridge(romPath) == CartridgeLoadingRes::Ok);

    gb.play();

    // running for 5 emulated seconds should be enough to run the test

    uint64_t cycles = 0;
    auto startTp = hr_clock::now();
    while (cycles < gb.timeToCyclesBase(5s)) {
        auto [stillGoing, stepsRes] = gb.emulate();
        cycles += stepsRes.cpuRes.cycles;
    }
    auto elapsed = duration_cast<microseconds>(hr_clock::now() - startTp).count();

    auto result = compareDisplayWithFile(gb, blarggsFilesRoot / "halt_bug-dmg-cgb.png");

    INFO("Test name: ", romRelPath.string());
    INFO("This test took ", elapsed, "us");
    INFO("Executed ", cycles, " cycles");
    
    CHECK(result);

}
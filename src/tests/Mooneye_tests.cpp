

#include "TestUtils.h"
#include "gb/GameBoyCore.h"
#include "doctest/doctest.h"
#include <chrono>


namespace fs = std::filesystem;
using namespace std::chrono;
using hr_clock = std::chrono::high_resolution_clock;


static const fs::path mooneyeFilesRoot = getTestRoot() / "mts-20240127";


static constexpr uint32_t stepLimit = 1000000; // 1 million


enum class MooneyeRes : int32_t {
    Pass = 0,
    Fail = 1,
    Unrecognized = -1
};

static MooneyeRes checkMooneyeResult(const GameBoyClassic& gb)
{
    // before executing the LD B,B instruction, all mooneye tests write 
    // specific values in the registers to report the test result

    // a successful test rom will write the fibonacci numbers 
    // 3, 5, 8, 13, 21, 34 to registers B, C, D, E, H, L
    // a failed test rom will write 0x42 to B, C, D, E, H, L

    const auto& regs = gb.cpu.regs;

    if (regs.B == 0x42 && regs.C == 0x42 && regs.D == 0x42 && regs.E == 0x42 && regs.H == 0x42 && regs.L == 0x42)
        return MooneyeRes::Fail;
    else if (regs.B == 3 && regs.C == 5 && regs.D == 8 && regs.E == 13 && regs.H == 21 && regs.L == 34)
        return MooneyeRes::Pass;
    else 
        return MooneyeRes::Unrecognized;
}


#define CREATE_MOONEYE_TEST(path)   \
TEST_CASE("Mooneye tests - " path){  \
    auto romPath = mooneyeFilesRoot / path ;\
\
    GameBoyClassic gb;\
    REQUIRE(gb.loadCartridge(romPath) == CartridgeLoadingRes::Ok);\
\
    gb.breakOnLdbb = true;\
    gb.play();\
\
    uint32_t steps = 0;\
    auto startTp = hr_clock::now();\
    for (steps = 0; steps < stepLimit && gb.status == GameBoyClassic::Status::Playing; ++steps) {\
        gb.emulate();\
    }\
    auto elapsed = duration_cast<microseconds>(hr_clock::now() - startTp).count();\
\
    INFO("This test took ", elapsed, "us");\
    INFO("Executed ", steps, " steps");\
\
    CHECK(steps < stepLimit);\
    CHECK(checkMooneyeResult(gb) == MooneyeRes::Pass);\
}\


CREATE_MOONEYE_TEST("acceptance/boot_hwio-dmg0.gb")
CREATE_MOONEYE_TEST("acceptance/boot_regs-dmg0.gb")




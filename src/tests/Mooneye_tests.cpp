

#include "TestUtils.h"
#include "gb/GameBoyCore.h"
#include "doctest/doctest.h"
#include <chrono>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>



namespace fs = std::filesystem;
using namespace std::chrono;
using hr_clock = std::chrono::high_resolution_clock;


static const fs::path testFilesRoot = getTestRoot();
static const fs::path mooneyeFilesRoot = testFilesRoot / "mts-20240127";
static const fs::path mooneyeResFilesRoot = testFilesRoot / "mts-results";


static constexpr uint32_t stepLimit = 1000000; // 1 million


enum class MooneyeRes : int32_t {
    Pass = 0,
    Fail = 1,
    Unrecognized = -1
};

MooneyeRes checkMooneyeResult(const GameBoyClassic& gb)
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

void saveDisplayToFile(const GameBoyClassic& gb, fs::path romRelPath)
{
    auto pngPath = (mooneyeResFilesRoot / romRelPath).replace_extension("png");
    
    if (fs::exists(pngPath)) {
        fs::remove(pngPath);
    }
    else {
        fs::create_directories(pngPath.parent_path());
    }

    fs::create_directories(pngPath.parent_path());

    static const auto w = Display::w;
    static const auto h = Display::h;

    RgbaBuffer buf(w, h);
    gb.ppu.display.getFrontBuf().fillRgbaBuffer(buf);

    stbi_write_png(pngPath.string().c_str(), w, h, 4, static_cast<const void*>(buf.ptr()), w * 3);
}


#define CREATE_MOONEYE_TEST(cartPath)   \
TEST_CASE("Mooneye tests - " cartPath){  \
    fs::path romRelPath = cartPath;\
    auto romPath = mooneyeFilesRoot / romRelPath;\
\
    GameBoyClassic gb;\
    REQUIRE(gb.loadCartridge(romPath) == CartridgeLoadingRes::Ok);\
\
    gb.dbg.breakOnLdbb = true;\
    gb.play();\
\
    uint32_t steps = 0;\
    auto startTp = hr_clock::now();\
    for (steps = 0; steps < stepLimit && gb.status == GameBoyClassic::Status::Running; ++steps) {\
        gb.emulate();\
    }\
    auto elapsed = duration_cast<microseconds>(hr_clock::now() - startTp).count();\
\
    auto result = checkMooneyeResult(gb);\
\
    INFO("This test took ", elapsed, "us");\
    INFO("Executed ", steps, " steps");\
\
    CHECK(steps < stepLimit);\
    CHECK(result == MooneyeRes::Pass);\
\
    saveDisplayToFile(gb, romRelPath);\
}\


//CREATE_MOONEYE_TEST("acceptance/add_sp_e_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/boot_div-dmg0.gb")
//CREATE_MOONEYE_TEST("acceptance/boot_div-dmgABCmgb.gb")
//CREATE_MOONEYE_TEST("acceptance/boot_hwio-dmg0.gb")
//CREATE_MOONEYE_TEST("acceptance/boot_hwio-dmgABCmgb.gb")
//CREATE_MOONEYE_TEST("acceptance/boot_regs-dmg0.gb")
//CREATE_MOONEYE_TEST("acceptance/boot_regs-dmgABC.gb")
//CREATE_MOONEYE_TEST("acceptance/call_cc_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/call_cc_timing2.gb")
//CREATE_MOONEYE_TEST("acceptance/call_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/call_timing2.gb")
//CREATE_MOONEYE_TEST("acceptance/di_timing-GS.gb")
CREATE_MOONEYE_TEST("acceptance/div_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/ei_sequence.gb")
//CREATE_MOONEYE_TEST("acceptance/ei_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/halt_ime0_ei.gb")
//CREATE_MOONEYE_TEST("acceptance/halt_ime0_nointr_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/halt_ime1_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/halt_ime1_timing2-GS.gb")
//CREATE_MOONEYE_TEST("acceptance/if_ie_registers.gb")
//CREATE_MOONEYE_TEST("acceptance/intr_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/jp_cc_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/jp_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/ld_hl_sp_e_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/oam_dma_start.gb")
//CREATE_MOONEYE_TEST("acceptance/oam_dma_restart.gb")
//CREATE_MOONEYE_TEST("acceptance/oam_dma_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/pop_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/push_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/rapid_di_ei.gb")
//CREATE_MOONEYE_TEST("acceptance/ret_cc_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/ret_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/reti_intr_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/reti_timing.gb")
//CREATE_MOONEYE_TEST("acceptance/rst_timing.gb")



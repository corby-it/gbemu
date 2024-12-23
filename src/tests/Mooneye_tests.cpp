

#include "TestUtils.h"
#include "gb/GameBoyCore.h"
#include "doctest/doctest.h"
#include <chrono>



namespace fs = std::filesystem;
using namespace std::chrono;
using hr_clock = std::chrono::high_resolution_clock;


static const fs::path testFilesRoot = getTestRoot();
static const fs::path mooneyeFilesRoot = testFilesRoot / "mts-20240926-1737-443f6e1";
static const fs::path mooneyeResFilesRoot = testFilesRoot / "mts-results";

// mooneye test shouldn't run for more than 120 emulated seconds:
// source: https://github.com/Gekkio/mooneye-gb/blob/master/core/tests/mooneye_suite.rs

static constexpr uint64_t cyclesLimit = GameBoyClassic::timeToCycles(120s);


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




TEST_CASE("Mooneye test roms") {
    
    fs::path romRelPath = "";

    //SUBCASE("") { romRelPath = "acceptance/add_sp_e_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/boot_div-dmg0.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/boot_div-dmgABCmgb.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/boot_hwio-dmg0.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/boot_hwio-dmgABCmgb.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/boot_regs-dmg0.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/boot_regs-dmgABC.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/call_cc_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/call_cc_timing2.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/call_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/call_timing2.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/di_timing-GS.gb"; }
    SUBCASE("") { romRelPath = "acceptance/div_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/ei_sequence.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/ei_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/halt_ime0_ei.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/halt_ime0_nointr_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/halt_ime1_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/halt_ime1_timing2-GS.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/if_ie_registers.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/intr_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/jp_cc_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/jp_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/ld_hl_sp_e_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/oam_dma_start.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/oam_dma_restart.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/oam_dma_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/pop_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/push_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/rapid_di_ei.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/ret_cc_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/ret_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/reti_intr_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/reti_timing.gb"; }
    //SUBCASE("") { romRelPath = "acceptance/rst_timing.gb"; }
    
    // Mooneye tests for MBC1
    SUBCASE("") { romRelPath = "emulator-only/mbc1/bits_bank1.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc1/bits_bank2.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc1/bits_mode.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc1/bits_ramg.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc1/ram_64kb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc1/ram_256kb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc1/rom_512kb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc1/rom_1Mb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc1/rom_2Mb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc1/rom_4Mb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc1/rom_8Mb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc1/rom_16Mb.gb"; }

    // Mooneye tests for MBC2
    SUBCASE("") { romRelPath = "emulator-only/mbc2/bits_ramg.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc2/bits_romb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc2/bits_unused.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc2/ram.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc2/rom_512kb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc2/rom_1Mb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc2/rom_2Mb.gb"; }

    // Mooneye tests for MBC5
    SUBCASE("") { romRelPath = "emulator-only/mbc5/rom_512kb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc5/rom_1Mb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc5/rom_2Mb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc5/rom_4Mb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc5/rom_8Mb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc5/rom_16Mb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc5/rom_32Mb.gb"; }
    SUBCASE("") { romRelPath = "emulator-only/mbc5/rom_64Mb.gb"; }


    auto romPath = mooneyeFilesRoot / romRelPath; 
        
    GameBoyClassic gb; 
    REQUIRE(gb.loadCartridge(romPath) == CartridgeLoadingRes::Ok); 
        
    gb.dbg.breakOnLdbb = true; 
    gb.play(); 
        
    uint64_t cycles = 0; 
    auto startTp = hr_clock::now(); 
    while (cycles < cyclesLimit && gb.status == GameBoyClassic::Status::Running) {    
        auto [stillGoing, stepsRes] = gb.emulate();
        cycles += stepsRes.cpuRes.cycles;
    }
    auto elapsed = duration_cast<microseconds>(hr_clock::now() - startTp).count(); 
                
    auto result = checkMooneyeResult(gb); 
    
    INFO("Test name: ", romRelPath.string());
    INFO("This test took ", elapsed, "us"); 
    INFO("Executed ", cycles, " cycles"); 
                
    CHECK(cycles < cyclesLimit); 
    CHECK(result == MooneyeRes::Pass); 

    auto pngPath = (mooneyeResFilesRoot / romRelPath).replace_extension("png");

    saveDisplayToFile(gb, pngPath); 
}



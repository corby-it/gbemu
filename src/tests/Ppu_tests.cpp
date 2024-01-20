
#include "gb/Ppu.h"
#include "gb/GbCommons.h"
#include "doctest/doctest.h"


TEST_CASE("PPU test LY and dot increase")
{
    TestBus bus;
    PPU p(bus);

    // to draw one frame we need 70224 clock cycles (17556 machine cycles)
    // so, to draw at least two frames we need at least 35112 machine cycles

    uint32_t steps = 1;
    for (; steps < 36000; steps++) 
        p.step(1);

    // revert steps to the last actually used value
    steps--;

    CHECK(p.getDotCounter() == (steps * 4) % 456);
    CHECK(p.regs.LY == ((steps * 4) / 456) % 154);

}

TEST_CASE("PPU test PPU mode progression")
{
    TestBus bus;
    PPU p(bus);

    // at the beginning the ppu must be in mode 2 (oam scan)
    CHECK(p.regs.LY == 0);
    CHECK(p.regs.STAT.ppuMode == PPUMode::OAMScan);

    // after 80 dots (20 m-cycles) it must be in mode 3 (draw)
    p.step(20);
    CHECK(p.regs.STAT.ppuMode == PPUMode::Draw);

    // after 172 dots (43 m-cycles) it must be in mode 0 (h-blank)
    p.step(43);
    CHECK(p.regs.STAT.ppuMode == PPUMode::HBlank);

    // after 204 dots (51 m-cycles) it must be again in mode 2 (oam scan)
    // and the line register should be 1 now 
    p.step(51);
    CHECK(p.regs.STAT.ppuMode == PPUMode::OAMScan);
    CHECK(p.regs.LY == 1);

    // step for another 80+172+204 = 456 dots (114 m-cycles) to go to line 2
    p.step(114);
    CHECK(p.regs.STAT.ppuMode == PPUMode::OAMScan);
    CHECK(p.regs.LY == 2);

    // step for another 456 dots (114 m-cycles) * 142 lines to end in the v-blank mode on line 144
    p.step(114 * 142);
    CHECK(p.regs.STAT.ppuMode == PPUMode::VBlank);
    CHECK(p.regs.LY == 144);

    // step for another 9 lines (114 m-cycles each) to end at the beginning of line 153
    // still in the v-blank mode
    p.step(114 * 9);
    CHECK(p.regs.STAT.ppuMode == PPUMode::VBlank);
    CHECK(p.regs.LY == 153);

    // step for another line to restart from the beginning
    p.step(114);
    CHECK(p.regs.STAT.ppuMode == PPUMode::OAMScan);
    CHECK(p.regs.LY == 0);

}
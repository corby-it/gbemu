
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

TEST_CASE("PPU test PPU mode progression and memory locking")
{
    TestBus bus;
    PPU p(bus);

    // at the beginning the ppu must be in mode 2 (oam scan)
    CHECK(p.regs.LY == 0);
    CHECK(p.regs.STAT.ppuMode == PPUMode::OAMScan);
    CHECK(p.oamRam.isLocked());
    CHECK_FALSE(p.vram.isLocked());

    // after 80 dots (20 m-cycles) it must be in mode 3 (draw)
    p.step(20);
    CHECK(p.regs.STAT.ppuMode == PPUMode::Draw);
    CHECK(p.oamRam.isLocked());
    CHECK(p.vram.isLocked());

    // after 172 dots (43 m-cycles) it must be in mode 0 (h-blank)
    p.step(43);
    CHECK(p.regs.STAT.ppuMode == PPUMode::HBlank);
    CHECK_FALSE(p.oamRam.isLocked());
    CHECK_FALSE(p.vram.isLocked());

    // after 204 dots (51 m-cycles) it must be again in mode 2 (oam scan)
    // and the line register should be 1 now 
    p.step(51);
    CHECK(p.regs.STAT.ppuMode == PPUMode::OAMScan);
    CHECK(p.regs.LY == 1);
    CHECK(p.oamRam.isLocked());
    CHECK_FALSE(p.vram.isLocked());

    // step for another 80+172+204 = 456 dots (114 m-cycles) to go to line 2
    p.step(114);
    CHECK(p.regs.STAT.ppuMode == PPUMode::OAMScan);
    CHECK(p.regs.LY == 2);
    CHECK(p.oamRam.isLocked());
    CHECK_FALSE(p.vram.isLocked());

    // step for another 456 dots (114 m-cycles) * 142 lines to end in the v-blank mode on line 144
    p.step(114 * 142);
    CHECK(p.regs.STAT.ppuMode == PPUMode::VBlank);
    CHECK(p.regs.LY == 144);
    CHECK_FALSE(p.oamRam.isLocked());
    CHECK_FALSE(p.vram.isLocked());

    // step for another 9 lines (114 m-cycles each) to end at the beginning of line 153
    // still in the v-blank mode
    p.step(114 * 9);
    CHECK(p.regs.STAT.ppuMode == PPUMode::VBlank);
    CHECK(p.regs.LY == 153);
    CHECK_FALSE(p.oamRam.isLocked());
    CHECK_FALSE(p.vram.isLocked());

    // step for another line to restart from the beginning
    p.step(114);
    CHECK(p.regs.STAT.ppuMode == PPUMode::OAMScan);
    CHECK(p.regs.LY == 0);
    CHECK(p.oamRam.isLocked());
    CHECK_FALSE(p.vram.isLocked());

}


TEST_CASE("PPU test OAM scan function")
{
    TestBus bus;
    PPU p(bus);

    // temporarily put all oams at the bottom of the screen (y = 160)
    for (uint8_t i = 0; i < OAMRam::oamCount; ++i) {
        auto oam = p.oamRam.getOAMData(i);

        oam.tileId() = i;
        oam.x() = 0;
        oam.y() = 160;
    }

    // move 4 oams so that they intersect the first line (the actual y coordinate is oam.y() - 16)
    p.oamRam.getOAMData(3).y() = 14;
    p.oamRam.getOAMData(8).y() = 14;
    p.oamRam.getOAMData(23).y() = 14;
    p.oamRam.getOAMData(34).y() = 14;

    // move other 3 oams so that they intersect with line 100
    p.oamRam.getOAMData(6).y() = 114;
    p.oamRam.getOAMData(19).y() = 114;
    p.oamRam.getOAMData(30).y() = 114;


    // step so that the oam scan function is executed
    p.step(5);

    // check if we have the first 4 oams in the register
    auto& scanReg = p.getOamScanRegister();

    CHECK(scanReg.size() == 4);
    
    CHECK(scanReg[0].tileId() == 3);
    CHECK(scanReg[1].tileId() == 8);
    CHECK(scanReg[2].tileId() == 23);
    CHECK(scanReg[3].tileId() == 34);

    // step to line 100
    p.stepLine(100);

    // check if the current oams are correct
    CHECK(scanReg.size() == 3);

    CHECK(scanReg[0].tileId() == 6);
    CHECK(scanReg[1].tileId() == 19);
    CHECK(scanReg[2].tileId() == 30);
}
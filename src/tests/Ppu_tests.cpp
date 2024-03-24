
#include "gb/Ppu.h"
#include "gb/Dma.h"
#include "gb/GbCommons.h"
#include "gb/Irqs.h"
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

TEST_CASE("PPU test V-Blank interrupt generation")
{
    TestBus bus;
    PPU p(bus);

    auto vblankIrqMask = Irqs::mask(Irqs::Type::VBlank);

    // the vblank irq is triggered every time the ppu enters vblank mode
    p.stepLine(143);

    CHECK_FALSE(vblankIrqMask == (bus.read8(mmap::regs::IF) & vblankIrqMask));

    p.stepLine();

    CHECK(vblankIrqMask == (bus.read8(mmap::regs::IF) & vblankIrqMask));
}


TEST_CASE("PPU test STAT interrupt generation")
{
    TestBus bus;
    PPU p(bus);

    auto lcdIrqMask = Irqs::mask(Irqs::Type::Lcd);

    SUBCASE("Test LY == LYC interrupt generation") {
        p.regs.LYC = 50;
        p.regs.STAT.lycIrqEnable = true;

        // this interrupt is triggered when the internal LY register value is equal
        // to the value of the LYC register
        p.stepLine(49);

        CHECK_FALSE(lcdIrqMask == (bus.read8(mmap::regs::IF) & lcdIrqMask));

        p.stepLine();

        CHECK(lcdIrqMask == (bus.read8(mmap::regs::IF) & lcdIrqMask));
    }

    SUBCASE("Test Mode 0 interrupt generation (H-Blank)") {
        p.regs.STAT.mode0IrqEnable = true;

        // this interrupt is triggered when the ppu enters H-Blank mode
        // it takes 63 m-cycles to reach h-blank mode

        p.step(62);

        CHECK_FALSE(lcdIrqMask == (bus.read8(mmap::regs::IF) & lcdIrqMask));

        p.step(1);

        CHECK(lcdIrqMask == (bus.read8(mmap::regs::IF) & lcdIrqMask));
    }

    SUBCASE("Test Mode 1 interrupt generation (V-Blank)") {
        p.regs.STAT.mode1IrqEnable = true;

        // the vblank irq is triggered every time the ppu enters vblank mode
        // after 143 lines
        p.stepLine(143);

        CHECK_FALSE(lcdIrqMask == (bus.read8(mmap::regs::IF) & lcdIrqMask));

        p.stepLine();

        CHECK(lcdIrqMask == (bus.read8(mmap::regs::IF) & lcdIrqMask));
    }

    SUBCASE("Test Mode 2 interrupt generation (OAM Scan)") {
        // don't trigger the irq right away, wait until the start of the next line
        p.step(30);

        p.regs.STAT.mode2IrqEnable = true;

        CHECK_FALSE(lcdIrqMask == (bus.read8(mmap::regs::IF) & lcdIrqMask));

        p.stepLine();

        CHECK(lcdIrqMask == (bus.read8(mmap::regs::IF) & lcdIrqMask));
    }
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

    REQUIRE(scanReg.size() == 4);
    
    CHECK(scanReg[0].tileId() == 3);
    CHECK(scanReg[1].tileId() == 8);
    CHECK(scanReg[2].tileId() == 23);
    CHECK(scanReg[3].tileId() == 34);

    // step to line 100
    p.stepLine(100);

    // check if the current oams are correct
    REQUIRE(scanReg.size() == 3);

    CHECK(scanReg[0].tileId() == 6);
    CHECK(scanReg[1].tileId() == 19);
    CHECK(scanReg[2].tileId() == 30);
}


TEST_CASE("PPU test drawing simple objects, no window, no overlaps")
{
    TestBus bus;
    PPU p(bus);

    // set all palettes to default so that color id corresponds to color value
    p.regs.BGP.setToDefault();
    p.regs.OBP0.setToDefault();
    p.regs.OBP1.setToDefault();

    // set the ppu to share the same vram area for both bg/win and objects
    bool bgTileAreaFlag = true;
    p.regs.LCDC.bgWinTileDataArea = bgTileAreaFlag;
    p.regs.LCDC.objDoubleH = false;

    uint8_t bgColor = 0;
    uint8_t objColors[3] = { 1, 2, 3 };
    
    // background will use only one tile
    auto bgTile = p.vram.getBgTile(0, bgTileAreaFlag);
    for (uint32_t y = 0; y < TileData::h; ++y) {
        for (uint32_t x = 0; x < TileData::w; ++x) {
            bgTile.set(x, y, bgColor);
        }
    }

    // setup object tiles
    for (uint8_t i = 0; i < 3; i++) {
        auto colorVal = objColors[i];
        auto objTile = p.vram.getObjTile(i + 1, false);

        for (uint32_t y = 0; y < TileData::h; ++y) {
            for (uint32_t x = 0; x < TileData::w; ++x) {
                objTile.set(x, y, colorVal);
            }
        }
    }

    // put all oams at the bottom of the screen (y = 160)
    for (uint8_t i = 0; i < OAMRam::oamCount; ++i) {
        auto oam = p.oamRam.getOAMData(i);

        oam.tileId() = i;
        oam.x() = 0;
        oam.y() = 160;
    }

    std::vector<OAMData> objs = {
        p.oamRam.getOAMData(0),
        p.oamRam.getOAMData(1),
        p.oamRam.getOAMData(2),
    };

    // put oam 0 at (30, 50) in the display
    objs[0].x() = 38;
    objs[0].y() = 46;
    objs[0].tileId() = 1;

    // put oam 1 at (2, 2) in the display
    objs[1].x() = 10;
    objs[1].y() = 18;
    objs[1].tileId() = 2;

    // put oam 2 at (100, 80) in the display
    objs[2].x() = 108;
    objs[2].y() = 96;
    objs[2].tileId() = 3;

    // draw one frame
    p.stepFrame();

    // check if the areas corresponding the objects are of the right color
    auto checkObjDisplayArea = [&](OAMData& oam, uint8_t color) {
        for (uint32_t y = oam.y() - 16; (int32_t)y < oam.y() - 8; ++y) {
            for (uint32_t x = oam.x() - 8; x < oam.x(); ++x) {
                if (p.display.get(x, y) != color)
                    return false;
            }
        }
        return true;
    };

    CHECK(checkObjDisplayArea(objs[0], objColors[0]));
    CHECK(checkObjDisplayArea(objs[1], objColors[1]));
    CHECK(checkObjDisplayArea(objs[2], objColors[2]));

    // check if the background is of the right color
    auto checkBgDisplayArea = [&]() {
        for (uint32_t y = 0; y < Display::h; ++y) {
            for (uint32_t x = 0; x < Display::w; ++x) {
                bool insideObj = false;
                for (const auto& obj : objs) {
                    if (obj.isInside(x, y)) {
                        insideObj = true;
                        break;
                    }
                }

                if (!insideObj && p.display.get(x, y) != bgColor)
                    return false;
            }
        }
        return true;
    };

    CHECK(checkBgDisplayArea());
}

TEST_CASE("PPU test object overlaps (different x coordinates)")
{
    TestBus bus;
    PPU p(bus);

    // set all palettes to default so that color id corresponds to color value
    p.regs.BGP.setToDefault();
    p.regs.OBP0.setToDefault();
    p.regs.OBP1.setToDefault();

    // set the ppu to share the same vram area for both bg/win and objects
    bool bgTileAreaFlag = true;
    p.regs.LCDC.bgWinTileDataArea = bgTileAreaFlag;
    p.regs.LCDC.objDoubleH = false;

    uint8_t bgColor = 0;
    uint8_t objColors[3] = { 1, 2, 3 };

    // background will use only one tile
    auto bgTile = p.vram.getBgTile(0, bgTileAreaFlag);
    for (uint32_t y = 0; y < TileData::h; ++y) {
        for (uint32_t x = 0; x < TileData::w; ++x) {
            bgTile.set(x, y, bgColor);
        }
    }

    // setup object tiles
    for (uint8_t i = 0; i < 3; i++) {
        auto colorVal = objColors[i];
        auto objTile = p.vram.getObjTile(i + 1, false);

        for (uint32_t y = 0; y < TileData::h; ++y) {
            for (uint32_t x = 0; x < TileData::w; ++x) {
                objTile.set(x, y, colorVal);
            }
        }
    }

    // put all oams at the bottom of the screen (y = 160)
    for (uint8_t i = 0; i < OAMRam::oamCount; ++i) {
        auto oam = p.oamRam.getOAMData(i);

        oam.tileId() = i;
        oam.x() = 0;
        oam.y() = 160;
    }

    std::vector<OAMData> objs = {
        p.oamRam.getOAMData(0),
        p.oamRam.getOAMData(1),
        p.oamRam.getOAMData(2),
    };

    // put oam 0 at (30, 50) in the display
    objs[0].x() = 38;
    objs[0].y() = 66;
    objs[0].tileId() = 1;

    // put oam 1 at (35, 50) in the display
    objs[1].x() = 43;
    objs[1].y() = 66;
    objs[1].tileId() = 2;

    // put oam 2 at (40, 50) in the display
    objs[2].x() = 48;
    objs[2].y() = 66;
    objs[2].tileId() = 3;

    // draw one frame
    p.stepFrame();

    // check if the areas corresponding the objects are of the right color,
    auto checkObjDisplayArea = [&](uint32_t xl, uint32_t xr, uint32_t yt, uint32_t yb, uint8_t color)
    {
        for (uint32_t y = yt; y < yb; ++y) {
            for (uint32_t x = xl; x < xr; ++x) {
                if (p.display.get(x, y) != color)
                    return false;
            }
        }
        return true;
    };

    CHECK(checkObjDisplayArea(30, 38, 50, 58, objColors[0]));
    CHECK(checkObjDisplayArea(38, 43, 50, 58, objColors[1]));
    CHECK(checkObjDisplayArea(43, 48, 50, 58, objColors[2]));

    // check if the background is of the right color
    auto checkBgDisplayArea = [&]() {
        for (uint32_t y = 0; y < Display::h; ++y) {
            for (uint32_t x = 0; x < Display::w; ++x) {
                bool insideObj = false;
                for (const auto& obj : objs) {
                    if (obj.isInside(x, y)) {
                        insideObj = true;
                        break;
                    }
                }

                if (!insideObj && p.display.get(x, y) != bgColor)
                    return false;
            }
        }
        return true;
    };

    CHECK(checkBgDisplayArea());
}


TEST_CASE("PPU test object overlaps (same x coordinates)")
{
    TestBus bus;
    PPU p(bus);

    // set all palettes to default so that color id corresponds to color value
    p.regs.BGP.setToDefault();
    p.regs.OBP0.setToDefault();
    p.regs.OBP1.setToDefault();

    // set the ppu to share the same vram area for both bg/win and objects
    bool bgTileAreaFlag = true;
    p.regs.LCDC.bgWinTileDataArea = bgTileAreaFlag;
    p.regs.LCDC.objDoubleH = false;

    uint8_t bgColor = 0;
    uint8_t objColors[3] = { 1, 2, 3 };

    // background will use only one tile
    auto bgTile = p.vram.getBgTile(0, bgTileAreaFlag);
    for (uint32_t y = 0; y < TileData::h; ++y) {
        for (uint32_t x = 0; x < TileData::w; ++x) {
            bgTile.set(x, y, bgColor);
        }
    }

    // setup object tiles
    for (uint8_t i = 0; i < 3; i++) {
        auto colorVal = objColors[i];
        auto objTile = p.vram.getObjTile(i + 1, false);

        for (uint32_t y = 0; y < TileData::h; ++y) {
            for (uint32_t x = 0; x < TileData::w; ++x) {
                objTile.set(x, y, colorVal);
            }
        }
    }

    // put all oams at the bottom of the screen (y = 160)
    for (uint8_t i = 0; i < OAMRam::oamCount; ++i) {
        auto oam = p.oamRam.getOAMData(i);

        oam.tileId() = i;
        oam.x() = 0;
        oam.y() = 160;
    }

    std::vector<OAMData> objs = {
        p.oamRam.getOAMData(0),
        p.oamRam.getOAMData(1),
        p.oamRam.getOAMData(2),
    };

    // put oam 0 at (30, 50) in the display
    objs[0].x() = 38;
    objs[0].y() = 66;
    objs[0].tileId() = 1;

    // put oam 1 at (30, 55) in the display
    objs[1].x() = 38;
    objs[1].y() = 71;
    objs[1].tileId() = 2;

    // put oam 2 at (30, 60) in the display
    objs[2].x() = 38;
    objs[2].y() = 76;
    objs[2].tileId() = 3;

    // draw one frame
    p.stepFrame();

    // check if the areas corresponding the objects are of the right color,
    auto checkObjDisplayArea = [&](uint32_t xl, uint32_t xr, uint32_t yt, uint32_t yb, uint8_t color)
    {
        for (uint32_t y = yt; y < yb; ++y) {
            for (uint32_t x = xl; x < xr; ++x) {
                if (p.display.get(x, y) != color)
                    return false;
            }
        }
        return true;
    };

    CHECK(checkObjDisplayArea(30, 38, 50, 58, objColors[0]));
    CHECK(checkObjDisplayArea(30, 38, 58, 63, objColors[1]));
    CHECK(checkObjDisplayArea(30, 38, 63, 68, objColors[2]));

    // check if the background is of the right color
    auto checkBgDisplayArea = [&]() {
        for (uint32_t y = 0; y < Display::h; ++y) {
            for (uint32_t x = 0; x < Display::w; ++x) {
                bool insideObj = false;
                for (const auto& obj : objs) {
                    if (obj.isInside(x, y)) {
                        insideObj = true;
                        break;
                    }
                }

                if (!insideObj && p.display.get(x, y) != bgColor)
                    return false;
            }
        }
        return true;
    };

    CHECK(checkBgDisplayArea());
}


TEST_CASE("PPU test disabling the display")
{
    TestBus bus;
    PPU p(bus);

    auto lcdc = p.readLCDC();
    lcdc &= ~(1 << 7);
    p.writeLCDC(lcdc);

    // step for a random number of lines and steps
    p.stepLine(45);
    p.step(43);

    auto checkDisplay = [](const Display& disp) {
        for (uint32_t y = 0; y < Display::h; ++y) {
            for (uint32_t x = 0; x < Display::w; ++x) {
                if (disp.get(x, y) != 0)
                    return false;
            }
        }
        return true;
    };

    CHECK(checkDisplay(p.display));

    CHECK(p.getDotCounter() == 0);

}


TEST_CASE("DMA transfer")
{
    TestBus bus;
    DMA dma(bus);

    SUBCASE("Check read/write and scheduled/transferring flags") {
        CHECK_FALSE(dma.isTransferring());
        
        dma.write(0x80);

        CHECK(dma.read() == 0x80);
        // the transfer does not begin immediately
        CHECK_FALSE(dma.isTransferring());
        CHECK(dma.isScheduled());

        // there must be a 1 m-cycle delay before the transfer begins
        dma.step(1);
        CHECK_FALSE(dma.isTransferring());
        CHECK_FALSE(dma.isScheduled());

        // now the tranfer must be started
        dma.step(1);
        CHECK(dma.isTransferring());
        CHECK_FALSE(dma.isScheduled());

    }

    SUBCASE("Check data transfer") {
        uint16_t startAddr = 0xC000;

        // fill the read area with the value of i, write one additional value 
        // (the 161th value) to check that it has not been copied
        for (uint16_t i = 0; i < 161; ++i)
            bus.write8(startAddr + i, (uint8_t)i);

        // also write another known value one byte after the end of the oam area
        // to be able to check for off-by-one errors
        bus.write8(mmap::oam::end + 1, 0xFE);
        
        // start the transfer
        dma.write(0xC0);

        CHECK(dma.read() == 0xC0);
        CHECK_FALSE(dma.isTransferring());
        CHECK(dma.isScheduled());

        // step through the transfer process
        dma.step(80);

        CHECK(dma.read() == 0xC0);
        CHECK(dma.isTransferring());
        CHECK_FALSE(dma.isScheduled());

        dma.step(82);

        CHECK(dma.read() == 0xC0);
        CHECK_FALSE(dma.isTransferring());

        // verify if the data has been copied correctly
        for (uint16_t i = 0; i < 160; ++i) {
            if (bus.read8(mmap::oam::start + i) != i) {
                INFO("mem[mmap::oam::start + i] is different from i", i);
                CHECK(false);
            }
        }

        CHECK_MESSAGE(bus.read8(mmap::oam::start + 160) == 0xFE, "mem[mmap::oam::start + 160] is different from 0xFE");
    }

    SUBCASE("Check transfer restart") {
        uint16_t startAddr1 = 0xC000;
        uint16_t startAddr2 = 0xD000;

        // fill the 1st read area with the value of i, write one additional value 
        // (the 161th value) to check that it has not been copied
        for (uint16_t i = 0; i < 161; ++i)
            bus.write8(startAddr1 + i, (uint8_t)i);

        // fill the 2nd read area with the value of i+10, write one additional value 
        // (the 161th value) to check that it has not been copied
        for (uint16_t i = 0; i < 161; ++i)
            bus.write8(startAddr2 + i, (uint8_t)(i + 10));

        // also write another known value one byte after the end of the oam area
        // to be able to check for off-by-one errors
        bus.write8(mmap::oam::end + 1, 0xFE);

        // start the transfer
        dma.write(0xC0);

        CHECK(dma.read() == 0xC0);
        CHECK_FALSE(dma.isTransferring());
        CHECK(dma.isScheduled());

        // step through the transfer process
        dma.step(80);

        CHECK(dma.read() == 0xC0);
        CHECK(dma.isTransferring());
        CHECK_FALSE(dma.isScheduled());

        // check if the first 79 bytes have been transferred correctly
        for (uint16_t i = 0; i < 79; ++i) {
            if (bus.read8(mmap::oam::start + i) != i) {
                INFO("mem[mmap::oam::start + i] is different from i", i);
                CHECK(false);
            }
        }

        // start a new transfer
        dma.write(0xD0);

        CHECK(dma.read() == 0xD0);
        CHECK(dma.isTransferring());
        CHECK(dma.isScheduled());

        dma.step(1);

        // after 1 m-cycle the new transfer hasn't started yet but another byte 
        // from the old transfer must have been transferred
        CHECK_MESSAGE(bus.read8(mmap::oam::start + 79) == 79, "mem[mmap::oam::start + 79] is different from 79");
        CHECK(dma.isTransferring());
        CHECK_FALSE(dma.isScheduled());

        // step through the end of the transfer
        dma.step(160);

        CHECK(dma.read() == 0xD0);
        CHECK_FALSE(dma.isTransferring());

        // verify if the data has been copied correctly
        for (uint16_t i = 0; i < 160; ++i) {
            if (bus.read8(mmap::oam::start + i) != i + 10) {
                INFO("mem[mmap::oam::start + i] is different from i + 10", i);
                CHECK(false);
            }
        }

        CHECK_MESSAGE(bus.read8(mmap::oam::start + 160) == 0xFE, "mem[mmap::oam::start + 160] is different from 0xFE");
    }

}
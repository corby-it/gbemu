

#include "Ppu.h"



// ------------------------------------------------------------------------------------------------
// PPURegs
// ------------------------------------------------------------------------------------------------

uint8_t LCDCReg::asU8() const
{
    uint8_t val = (uint8_t)bgWinEnable
        | objEnable << 1
        | objDoubleH << 2
        | bgTileMapArea << 3
        | bgWinTileDataArea << 4
        | winEnable << 5
        | winTileMapArea << 6
        | lcdEnable << 7;

    return val;
}

void LCDCReg::fromU8(uint8_t b)
{
    bgWinEnable = b & 0x01;
    objEnable = (b & 0x02) >> 1;
    objDoubleH = (b & 0x04) >> 2;
    bgTileMapArea = (b & 0x08) >> 3;
    bgWinTileDataArea = (b & 0x10) >> 4;
    winEnable = (b & 0x20) >> 5;
    winTileMapArea = (b & 0x40) >> 6;
    lcdEnable = (b & 0x80) >> 7;
}


uint8_t STATReg::asU8() const
{
    uint8_t val = ppuMode
        | lycEqual << 2
        | mode0IrqEnable << 3
        | mode1IrqEnable << 4
        | mode2IrqEnable << 5
        | lycIrqEnable << 6;

    return val;
}

void STATReg::fromU8(uint8_t b)
{
    // PPU mode and lyc==ly are read-only
    mode0IrqEnable = (b & 0x08u) << 3;
    mode1IrqEnable = (b & 0x10u) << 4;
    mode2IrqEnable = (b & 0x20u) << 5;
    lycIrqEnable = (b & 0x40u) << 6;
}


uint8_t PaletteReg::asU8() const
{
    uint8_t val = id0
        | id1 << 2
        | id2 << 4
        | id3 << 6;

    return val;
}

void PaletteReg::fromU8(uint8_t b)
{
    id0 = b & 0x03;
    id1 = (b & 0x0C) >> 2;
    id2 = (b & 0x30) >> 4;
    id3 = (b & 0xC0) >> 6;
}



void PPURegs::reset()
{
    // initialize the PPU registers with their default values
    // from: https://gbdev.gg8.se/wiki/articles/Power_Up_Sequence

    LCDC.fromU8(0x91);
    STAT.fromU8(0x00);

    SCY = 0;
    SCX = 0;
    
    LY = 0;
    LYC = 0;

    BGP.fromU8(0xFC);

    OBP0.fromU8(0xFF);
    OBP1.fromU8(0xFF);

    WY = 0;
    WX = 0;
}





// ------------------------------------------------------------------------------------------------
// PPU
// ------------------------------------------------------------------------------------------------

PPU::PPU(Bus& bus)
    : mBus(bus)
{
    reset();
}

void PPU::reset()
{
    mDotCounter = 0;

    regs.reset();

    // at reset update the STAT register to actually reflect the current
    // status of the PPU
    updateSTAT();
}

void PPU::step(uint16_t mCycles)
{
    // the PPU goes through a cycle of its own, separate from that of the CPU.
    // it draws 153 lines, top to bottom and left to right, from line 0 to 143 it 
    // draws the lines seen on the display, from lines 155 to 153 its in the vblank mode
    // 
    // the PPU it goes through 4 modes:
    // mode 2: OAM scan (checks the OAM memory to determine wich objects must be drawn)
    // mode 3: drawing pixels (access VRAM to get pixel values)
    // mode 0: hblank
    // mode 1: vblank

    // in each mode some memory areas might not be accessible to the CPU because they're 
    // locked by the PPU
    // mode 2: OAM memory not accessible
    // mode 3: VRAM and OAM memory not accessible
    // mode 0: everything accessible
    // mode 1: everything accessible

    // every part of this process takes a predefined amount of clock cycles (or machine cycles),
    // here 1 dot == 1 clock cycles (so 4 dots == 1 machine cycle):
    // mode 2: OAM scan -> 80 dots (20 m-cycles)
    // mode 3: draw -> 172 dots (43 m-cycles)
    // mode 0: hblank -> 204 dots (51 m-cycles)
    // mode 1: vblank -> 4560 dots (1140 m-cycles)
    // mode 3 might get longer and bleed into mode 0 but the sum of their dots will still be 
    // 172 + 204 = 376 dots (94 m-cycles)

    // in this function we advance the rendering cycle based on how many machine cycles the cpu just executed
    
    // mDotCounter counts the dots for the current line while the LY register counts the lines,
    // each line has 456 dots

    // before doing anything we have to unlock the memory, the PPU can always access it
    mOamRam.lock(false);
    mVram.lock(false);

    // since the last call 'cCycles' have passed, act accordingly
    uint32_t cCycles = mCycles * 4;

    while (cCycles--) {
        mDotCounter = (mDotCounter + 1) % 456;

        if (mDotCounter == 0) {
            // if the new value of the dot counter is zero it means it wrapped around 
            // and a new line just started
            regs.LY = (regs.LY + 1) % 154;

            // at the beginning of a non-vblank new line the ppu enters mode 2 so we scan the OAM now
            if (regs.LY < 144) {
                // TODO actually do the OAM scan
            }
        }

        // updated the STAT register to reflect the current status of the PPU
        updateSTAT();
        
        // if we are in mode 3 we have to draw the corresponding pixels
        // if we are in a different mode there is nothing to do as OAM scan
        // is handled all at once at the beginning of a new line
        if (regs.STAT.ppuMode == PPUMode::Draw) {
            // TODO handle drawing
        }
    }

    // before returning control to the main loop we have to lock 
    // or unlock video related memory depending on the current PPU mode
    switch (regs.STAT.ppuMode) {
    default:
    case PPUMode::HBlank:
    case PPUMode::VBlank:
        mOamRam.lock(false);
        mVram.lock(false);
        break;
    case PPUMode::OAMScan:
        mOamRam.lock(true);
        mVram.lock(false);
        break;
    case PPUMode::Draw:
        mOamRam.lock(true);
        mVram.lock(true);
        break;
    }

}

void PPU::OAMScan(std::array<OAMData, 10>& oams, size_t& count)
{
    // during the OAM scan phase the ppu checks which of the 40 possible OAMs
    // should be drawn on the current line LY

    // OAMs are scanned sequentially and up to 10 OAMs can be selected for each scanline

    count = 0;

    for (uint8_t id = 0; id < OAMRam::oamCount && count < 10; ++id) {
        auto oam = mOamRam.getOAMData(id);

        // yTop is the first line of the object
        // yBottom is the first line AFTER the object
        auto yTop = oam.y();
        auto yBottom = oam.y() + (regs.LCDC.objDoubleH ? 16 : 8);

        if (regs.LY >= yTop && regs.LY < yBottom) 
            oams[count++] = oam;
    }
}

void PPU::updateSTAT()
{
    // the first 3 bits of the STAT register are updated depending on the 
    // internal status of the PPU
    regs.STAT.lycEqual = (regs.LY == regs.LYC);

    if (regs.LY >= 144) {
        regs.STAT.ppuMode = PPUMode::VBlank;
    }
    else {
        // TODO handle mode 3 length differences
        if (mDotCounter < 80)
            regs.STAT.ppuMode = PPUMode::OAMScan;
        if (mDotCounter >= 80 && mDotCounter < 252)
            regs.STAT.ppuMode = PPUMode::Draw;
        if (mDotCounter >= 252)
            regs.STAT.ppuMode = PPUMode::HBlank;
    }
}

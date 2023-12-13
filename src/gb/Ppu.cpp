

#include "Ppu.h"



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
    mode0IrqEnable = (b & 0x08) << 3;
    mode1IrqEnable = (b & 0x10) << 4;
    mode2IrqEnable = (b & 0x20) << 5;
    lycIrqEnable = (b & 0x40) << 6;
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

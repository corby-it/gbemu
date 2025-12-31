

#include "Ppu.h"
#include "Irqs.h"
#include <tracy/Tracy.hpp>
#include <algorithm>
#include <array>
#include <cstdlib>


const char* const plotDotCounter = "PPU_DotCounter";
const char* const plotLY = "PPU_LY";
const char* const plotPpuMode = "PPU_Mode";



// ------------------------------------------------------------------------------------------------
// LCDCReg
// ------------------------------------------------------------------------------------------------

LCDCReg::LCDCReg()
    : bgWinEnable(false)
    , objEnable(false)
    , objDoubleH(false)
    , bgTileMapArea(false)
    , bgWinTileDataArea(false)
    , winEnable(false)
    , winTileMapArea(false)
    , lcdEnable(false)
{}

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



// ------------------------------------------------------------------------------------------------
// STATReg
// ------------------------------------------------------------------------------------------------

STATReg::STATReg()
    : ppuMode(PPUMode::OAMScan)
    , lycEqual(false)
    , mode0IrqEnable(false)
    , mode1IrqEnable(false)
    , mode2IrqEnable(false)
    , lycIrqEnable(false)
{}

uint8_t STATReg::asU8() const
{
    uint8_t val = (static_cast<uint8_t>(ppuMode) & 0x03)
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



// ------------------------------------------------------------------------------------------------
// DMGPaletteReg
// ------------------------------------------------------------------------------------------------

DMGPaletteReg::DMGPaletteReg()
    : valForId0(0)
    , valForId1(1)
    , valForId2(2)
    , valForId3(3)
{}

uint8_t DMGPaletteReg::asU8() const
{
    uint8_t val = valForId0
        | valForId1 << 2
        | valForId2 << 4
        | valForId3 << 6;

    return val;
}

void DMGPaletteReg::fromU8(uint8_t b)
{
    valForId0 = b & 0x03;
    valForId1 = (b & 0x0C) >> 2;
    valForId2 = (b & 0x30) >> 4;
    valForId3 = (b & 0xC0) >> 6;
}



// ------------------------------------------------------------------------------------------------
// CGBPalettes
// ------------------------------------------------------------------------------------------------

void CGBColor::setR(uint8_t r) {
    if (!ptr)
        return;

    r >>= 3;
    *ptr = (*ptr & ~maskR) | r;
}

void CGBColor::setG(uint8_t g) {
    if (!ptr)
        return;

    g >>= 3;
    uint8_t gLo = g << 5;
    uint8_t gHi = g >> 3;
    *ptr = (*ptr & ~0xE0) | gLo;
    *(ptr + 1) = (*(ptr + 1) & ~0x03) | gHi;
}

void CGBColor::setB(uint8_t b) {
    if (!ptr)
        return;

    b >>= 3;
    *(ptr + 1) = (*(ptr + 1) & ~0x7C) | (b << 2);
}

void CGBColor::set(uint8_t r, uint8_t g, uint8_t b) {
    if (!ptr)
        return;

    r >>= 3;
    g >>= 3;
    b >>= 3;

    uint16_t raw = r | (g << 5) | (b << 10);

    ptr[0] = uint8_t(raw);
    ptr[1] = uint8_t(raw >> 8);
}




void CGBPaletteData::resetRandom()
{
    std::srand((unsigned)std::time(nullptr)); // use current time as seed for random generator

    for (auto& v : raw) {
        v = uint8_t(std::abs(std::rand()) % 256);
    }
}



CGBPalettes::CGBPalettes()
    : mIsCgb(false)
{
    reset();
}

void CGBPalettes::reset()
{
    mDataRegsLocked = false;
    mBGPIReg.fromU8(0);
    mOBPIReg.fromU8(0);

    // all background colors are initialized as white 
    // all object colors are not initialized (random)
    // see: https://gbdev.io/pandocs/Palettes.html#lcd-color-palettes-cgb-only

    mBgData.resetWhite();
    mObjData.resetRandom();
}


namespace cgbpal = mmap::regs::col_palette;

uint8_t CGBPalettes::read8(uint16_t addr) const
{
    if (!mIsCgb)
        return 0xff;

    switch (addr) {
    case cgbpal::bgpi: return mBGPIReg.asU8();
    case cgbpal::bgpd:
        if (mDataRegsLocked)
            return 0xff;
        else 
            return mBgData.raw[mBGPIReg.index];

    case cgbpal::obpi: return mOBPIReg.asU8();
    case cgbpal::obpd: 
        if (mDataRegsLocked)
            return 0xff;
        else 
            return mObjData.raw[mOBPIReg.index];

    default:
        return 0xff;
    }
}

void CGBPalettes::write8(uint16_t addr, uint8_t val)
{
    if (!mIsCgb)
        return;

    switch (addr) {
    case cgbpal::bgpi: mBGPIReg.fromU8(val); break;
    
    case cgbpal::bgpd:
        if (!mDataRegsLocked) {
            mBgData.raw[mBGPIReg.index] = val;
            mBGPIReg.tryIncIndex();
        }
        break;

    case cgbpal::obpi: mOBPIReg.fromU8(val); break;

    case cgbpal::obpd:
        if (!mDataRegsLocked) {
            mObjData.raw[mOBPIReg.index] = val;
            mOBPIReg.tryIncIndex();
        }
        break;

    default:
        break;
    }
}




// ------------------------------------------------------------------------------------------------
// PPURegs
// ------------------------------------------------------------------------------------------------

PPURegs::PPURegs()
    : SCY(0)
    , SCX(0)
    , LY(0)
    , LYC(0)
    , WY(0)
    , WX(0)
{}

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
// BgHelper
// ------------------------------------------------------------------------------------------------

const char* bgHelperTileMapToStr(BgHelperTileMap bghtm)
{
    switch (bghtm) {
    case BgHelperTileMap::Active: return "Active";
    case BgHelperTileMap::At9800: return "At 0x9800";
    case BgHelperTileMap::At9C00: return "At 0x9C00";
    default:
        return "unknown";
    }
}

const char* bgHelperTileAddressingToStr(BgHelperTileAddressing bghta)
{
    switch (bghta) {
    case BgHelperTileAddressing::Active: return "Active";
    case BgHelperTileAddressing::At8000: return "At 0x8000";
    case BgHelperTileAddressing::At8800: return "At 0x8800";
    default:
        return "unknown";
    }
}




// ------------------------------------------------------------------------------------------------
// PPU
// ------------------------------------------------------------------------------------------------

namespace lcdreg = mmap::regs::lcd;



PPU::PPU(Bus& bus)
    : mBus(&bus)
    , mIsCgb(false)
    , hdma(bus)
{
    reset();
}

void PPU::reset()
{
    mUseDmgCompatMode = false;
    mDotCounter = 0;
    mOamScanRegister.reset();
    mFirstStep = true;

    regs.reset();
    colors.reset();
    hdma.reset();
    vram.reset();
    oamRam.reset();
    display.clear();
    
    // at reset update the STAT register to actually reflect the current
    // status of the PPU
    updateSTAT();

    // lock ram to correctly reflect the current ppu mode and the lcd enable status
    lockRamAreas(regs.LCDC.lcdEnable);
}

void PPU::setIsCgb(bool val)
{
    mIsCgb = val;
    colors.setIsCgb(val);
    vram.setIsCgb(val);
    hdma.setIsCgb(val);
}


uint8_t PPU::read8(uint16_t addr) const
{
    switch (addr) {
    case lcdreg::lcdc: return regs.LCDC.asU8();
    case lcdreg::stat: return regs.STAT.asU8();
    case lcdreg::scy: return regs.SCY;
    case lcdreg::scx: return regs.SCX;
    case lcdreg::ly: return regs.LY;
    case lcdreg::lyc: return regs.LYC;
    case lcdreg::bgp: return regs.BGP.asU8();
    case lcdreg::obp0: return regs.OBP0.asU8();
    case lcdreg::obp1: return regs.OBP1.asU8();
    case lcdreg::wy: return regs.WY;
    case lcdreg::wx: return regs.WX;
    default:
        return 0xff;
    }
}

void PPU::write8(uint16_t addr, uint8_t val)
{
    switch (addr) {
    case lcdreg::lcdc: writeLCDC(val); break;
    case lcdreg::stat: regs.STAT.fromU8(val); break;
    case lcdreg::scx: regs.SCX = val; break;
    case lcdreg::scy: regs.SCY = val; break;
    case lcdreg::ly: break; // LY is read-only
    case lcdreg::lyc: regs.LYC = val; break;
    case lcdreg::bgp: regs.BGP.fromU8(val); break;
    case lcdreg::obp0: regs.OBP0.fromU8(val); break;
    case lcdreg::obp1: regs.OBP1.fromU8(val); break;
    case lcdreg::wy: regs.WY = val; break;
    case lcdreg::wx: regs.WX = val; break;
    default:
        break;
    }
}



static const std::array<uint8_t, 94> paletteIndexesAndFlags = {
    0x7C, 0x08, 0x12, 0xA3, 0xA2, 0x07, 0x87, 0x4B, 0x20, 0x12, 0x65, 0xA8, 0x16, 0xA9, 0x86, 0xB1,
    0x68, 0xA0, 0x87, 0x66, 0x12, 0xA1, 0x30, 0x3C, 0x12, 0x85, 0x12, 0x64, 0x1B, 0x07, 0x06, 0x6F,
    0x6E, 0x6E, 0xAE, 0xAF, 0x6F, 0xB2, 0xAF, 0xB2, 0xA8, 0xAB, 0x6F, 0xAF, 0x86, 0xAE, 0xA2, 0xA2,
    0x12, 0xAF, 0x13, 0x12, 0xA1, 0x6E, 0xAF, 0xAF, 0xAD, 0x06, 0x4C, 0x6E, 0xAF, 0xAF, 0x12, 0x7C,
    0xAC, 0xA8, 0x6A, 0x6E, 0x13, 0xA0, 0x2D, 0xA8, 0x2B, 0xAC, 0x64, 0xAC, 0x6D, 0x87, 0xBC, 0x60,
    0xB4, 0x13, 0x72, 0x7C, 0xB5, 0xAE, 0xAE, 0x7C, 0x7C, 0x65, 0xA2, 0x6C, 0x64, 0x85
};

static const std::array<uint8_t, 29 * 3> paletteIndexes = {
    0x80, 0xB0, 0x40,
    0x88, 0x20, 0x68,
    0xDE, 0x00, 0x70,
    0xDE, 0x20, 0x78,
    0x20, 0x20, 0x38,
    0x20, 0xB0, 0x90,
    0x20, 0xB0, 0xA0,
    0xE0, 0xB0, 0xC0,
    0x98, 0xB6, 0x48,
    0x80, 0xE0, 0x50,
    0x1E, 0x1E, 0x58,
    0x20, 0xB8, 0xE0,
    0x88, 0xB0, 0x10,
    0x20, 0x00, 0x10,
    0x20, 0xE0, 0x18,
    0xE0, 0x18, 0x00,
    0x18, 0xE0, 0x20,
    0xA8, 0xE0, 0x20,
    0x18, 0xE0, 0x00,
    0x20, 0x18, 0xD8,
    0xC8, 0x18, 0xE0,
    0x00, 0xE0, 0x40,
    0x28, 0x28, 0x28,
    0x18, 0xE0, 0x60,
    0x20, 0x18, 0xE0,
    0x00, 0x00, 0x08,
    0xE0, 0x18, 0x30,
    0xD0, 0xD0, 0xD0,
    0x20, 0xE0, 0xE8,
};

#define RGB555(valR, valG, valB)    uint16_t((valR & 0x1F) | ((valG & 0x1F) << 5) | ((valB & 0x1F) << 10))

static const std::array<uint16_t, 30 * 4> compatPalettes = {
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x1F, 0x15, 0x0C), RGB555(0x10, 0x06, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1C, 0x18), RGB555(0x19, 0x13, 0x10), RGB555(0x10, 0x0D, 0x05), RGB555(0x0B, 0x06, 0x01),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x11, 0x11, 0x1B), RGB555(0x0A, 0x0A, 0x11), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x0F, 0x1F, 0x06), RGB555(0x00, 0x10, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x1F, 0x10, 0x10), RGB555(0x12, 0x07, 0x07), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x14, 0x14, 0x14), RGB555(0x0A, 0x0A, 0x0A), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x1F, 0x1F, 0x00), RGB555(0x0F, 0x09, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x0F, 0x1F, 0x00), RGB555(0x16, 0x0E, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x15, 0x15, 0x10), RGB555(0x08, 0x0E, 0x0F), RGB555(0x00, 0x00, 0x00),
    RGB555(0x14, 0x13, 0x1F), RGB555(0x1F, 0x1F, 0x00), RGB555(0x00, 0x0C, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x19), RGB555(0x0C, 0x1D, 0x1D), RGB555(0x13, 0x10, 0x06), RGB555(0x0B, 0x0B, 0x0B),
    RGB555(0x16, 0x16, 0x1F), RGB555(0x1F, 0x1F, 0x12), RGB555(0x15, 0x0B, 0x08), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x14), RGB555(0x1F, 0x12, 0x12), RGB555(0x12, 0x12, 0x1F), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x13), RGB555(0x12, 0x16, 0x1F), RGB555(0x0C, 0x12, 0x0E), RGB555(0x00, 0x07, 0x07),
    RGB555(0x0D, 0x1F, 0x00), RGB555(0x1F, 0x1F, 0x1F), RGB555(0x1F, 0x0A, 0x09), RGB555(0x00, 0x00, 0x00),
    RGB555(0x0A, 0x1B, 0x00), RGB555(0x1F, 0x10, 0x00), RGB555(0x1F, 0x1F, 0x00), RGB555(0x1F, 0x1F, 0x1F),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x1F, 0x0E, 0x00), RGB555(0x12, 0x08, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x18, 0x08), RGB555(0x1F, 0x1A, 0x00), RGB555(0x12, 0x07, 0x00), RGB555(0x09, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x0A, 0x1F, 0x00), RGB555(0x1F, 0x08, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x0C, 0x0A), RGB555(0x1A, 0x00, 0x00), RGB555(0x0C, 0x00, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x1F, 0x13, 0x00), RGB555(0x1F, 0x00, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x00, 0x1F, 0x00), RGB555(0x06, 0x10, 0x00), RGB555(0x00, 0x09, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x0B, 0x17, 0x1F), RGB555(0x1F, 0x00, 0x00), RGB555(0x00, 0x00, 0x1F),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x1F, 0x1F, 0x0F), RGB555(0x00, 0x10, 0x1F), RGB555(0x1F, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x1F, 0x1F, 0x00), RGB555(0x1F, 0x00, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x00), RGB555(0x1F, 0x00, 0x00), RGB555(0x0C, 0x00, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x1F, 0x19, 0x00), RGB555(0x13, 0x0C, 0x00), RGB555(0x00, 0x00, 0x00),
    RGB555(0x00, 0x00, 0x00), RGB555(0x00, 0x10, 0x10), RGB555(0x1F, 0x1B, 0x00), RGB555(0x1F, 0x1F, 0x1F),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x0C, 0x14, 0x1F), RGB555(0x00, 0x00, 0x1F), RGB555(0x00, 0x00, 0x00),
    RGB555(0x1F, 0x1F, 0x1F), RGB555(0x0F, 0x1F, 0x06), RGB555(0x00, 0x0C, 0x18), RGB555(0x00, 0x00, 0x00),
};

void PPU::setupDmgCompatMode(uint8_t paletteId)
{
    if (!mIsCgb)
        return;

    // the algorithm to pick a default palette is hard to understand and it has been 
    // copied (sort of) from the CGB boot rom from:
    // https://codeberg.org/ISSOtm/gb-bootroms/src/commit/443d7f057ae06e8d1d76fa8083650cf0be2cd0ae/src/cgb.asm
    // there are probably more efficient ways to get the same results but i won't bother
    // examples of games and associated palettes can be found at:
    // https://tcrf.net/Notes:Game_Boy_Color_Bootstrap_ROM#Assigned_Palette_Configurations

    // the paletteId is computed using a checksum of the title and a table,
    // the paletteId is then used to pick a set of flags from another table
    if (paletteId >= paletteIndexesAndFlags.size())
        return;

    uint8_t indexAndFlags = paletteIndexesAndFlags[paletteId];

    // the lower 5 bits identify a triplet
    // the upper 3 bits identify the flags
    uint8_t tripletIdx = indexAndFlags & 0x1F;
    uint8_t shuffleFlags = (indexAndFlags & 0xE0) >> 5;

    // using the flags we have to shuffle the hardcoded paletteIndexes array into the shuffled array,
    // the rules for the shuffling are:
    // - 1st entry (OBP0) is set to 3rd elem if bit 0 is set, otherwise to 1st element
    // - 2nd entry (OBP1) is set to 2nd elem if bit 2 is set, otherwise 3rd element if bit 1 is set, otherwise 1st element
    // - 3rd entry (BGP0) is always set to 3rd element
    // 
    // NOTE: apparently reversing rule 1 gives results that are more consistent with 
    //      real hardware examples and other well known emulators, who knows...

    std::array<uint8_t, 29 * 3> shuffled;

    for (uint32_t i = 0; i < shuffled.size(); i += 3) {
        // 1st entry OBP0
        shuffled[i] = shuffleFlags & 0x01 ? paletteIndexes[i] : paletteIndexes[i + 2];
        
        // 2nd entry OBP1
        if (shuffleFlags & 0x04)
            shuffled[i + 1] = paletteIndexes[i + 1];
        else if (shuffleFlags & 0x02)
            shuffled[i + 1] = paletteIndexes[i + 2];
        else 
            shuffled[i + 1] = paletteIndexes[i];

        // 3rd entry BGP
        shuffled[i + 2] = paletteIndexes[i + 2];
    }

    // now we have to create a palette table using the shuffled palette indexes,
    // each value in the shuffled array is used as an offset to pick a palette 
    // (4 colors = 8 bytes) in the hardcoded compatPalettes array
    // 
    // the index in the shuffled array is used as a byte offset so it must be 
    // divided by 2 in order to be used as offset for the compatPalettes array 
    // which stores u16 values for colors
    std::array<uint16_t, 29 * 3 * 4> paletteValues;

    for (uint32_t i = 0; i < shuffled.size(); ++i) {
        auto offset = shuffled[i] / 2;

        // we extract 4 u16 values (4 colors) from the compatPalettes array
        for (uint32_t k = 0; k < 4; ++k) {
            // make sure we don't read outside the array
            if (offset + k >= compatPalettes.size())
                paletteValues[i * 4 + k] = 0;
            else
                paletteValues[i * 4 + k] = compatPalettes[offset + k];
        }
    }

    // now, with the tripletIdx used as offset into the paletteValues array,
    // a set of 3 palettes is picked
    // tripletIdx is used to pick a triplet of palettes so:
    // - 0 picks the first three palettes (u16 values from 0 to 11)
    // - 1 picks the second threes palettes (u16 values from 12 to 23)
    // - and so on
    
    // tripletIdx is a 5-bit uint, make sure it's not greater than 28, which is the last
    // available palette triplet in the paletteValues array
    if (tripletIdx > 28)
        tripletIdx = 28;

    uint16_t obp0[4], obp1[4], bgp0[4];

    for (uint32_t i = 0; i < 4; ++i) {
        obp0[i] = paletteValues[tripletIdx * 12 + 0 + i];
        obp1[i] = paletteValues[tripletIdx * 12 + 4 + i];
        bgp0[i] = paletteValues[tripletIdx * 12 + 8 + i];
    }

    // now finally setup the values in the actual palette memory of the PPU
    for (uint8_t i = 0; i < 4; ++i) {
        colors.getObjPalette(0).setColor(i, obp0[i]);
        colors.getObjPalette(1).setColor(i, obp1[i]);
        colors.getBgPalette(0).setColor(i, bgp0[i]);
    }

    mUseDmgCompatMode = true;
}




void PPU::stepLine(uint32_t n)
{
    // step to the next line(s)
    while (n--) {
        step((456 - mDotCounter) / 4);
    }
}

void PPU::stepFrame(uint32_t n)
{
    // step to the next frame(s)
    while (n--) {
        stepLine(154 - regs.LY);
    }
}


bool PPU::step(uint32_t mCycles)
{
    ZoneScoped;

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

    bool frameReady = false;
    
    // do nothing if the lcd and ppu are not enabled 
    if(regs.LCDC.lcdEnable) {

        // before doing anything we have to unlock the memory, the PPU can always access it
        lockRamAreas(false);

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
                    oamScan();

                    // check if we have to trigger mode 2 (OAM Scan) STAT irq
                    if (regs.STAT.mode2IrqEnable) {
                        auto currIF = mBus->read8(mmap::regs::IF);
                        mBus->write8(mmap::regs::IF, Irqs::mask(Irqs::Type::Lcd) | currIF);
                    }
                }
                
                // as soon as we enter v-blank mode the ppu triggers the v-blank interrupt in the cpu
                if (regs.LY == 144) {
                    auto currIF = mBus->read8(mmap::regs::IF);
                    uint8_t newIrqMask = Irqs::mask(Irqs::Type::VBlank);

                    // check if we also have to trigger mode 1 (V-Blank) STAT irq
                    if (regs.STAT.mode1IrqEnable)
                        newIrqMask |= Irqs::mask(Irqs::Type::Lcd);

                    mBus->write8(mmap::regs::IF, newIrqMask | currIF);

                    // when we enter the v-blank mode it means the PPU is done drawing the current frame:
                    // - swap the display buffers top bring the complete frame on the front
                    // - return frameReady = true to tell the rest of the app that the frame is ready
                    display.swapBufs();
                    frameReady = true;
                }

                // check if we have to trigger the LY==LYC irq
                if (regs.STAT.lycIrqEnable && regs.LY == regs.LYC) {
                    auto currIF = mBus->read8(mmap::regs::IF);
                    mBus->write8(mmap::regs::IF, Irqs::mask(Irqs::Type::Lcd) | currIF);
                }
            }
            // check if we have to trigger mode 0 (H-Blank) STAT irq
            if (regs.STAT.mode0IrqEnable && mDotCounter == 252) {
                auto currIF = mBus->read8(mmap::regs::IF);
                mBus->write8(mmap::regs::IF, Irqs::mask(Irqs::Type::Lcd) | currIF);
            }
            if (mFirstStep) {
                mFirstStep = false;
                oamScan();
            }

            // updated the STAT register to reflect the current status of the PPU
            updateSTAT();
        
            // if we are in mode 3 we have to draw the corresponding pixels
            // if we are in a different mode there is nothing to do as OAM scan
            // is handled all at once at the beginning of a new line
            // 
            // the draw mode is 172 dots long, rendering starts after the first 12 dots
            // during which the gpu fetches stuff, after that, the 160 screen dots are actually rendered
            if (regs.STAT.ppuMode == PPUMode::Draw && mDotCounter >= 80 + 12) {
                uint32_t currX = mDotCounter - (80 + 12);
                
                if (mIsCgb)
                    renderPixelCGB(currX);
                else 
                    renderPixelDMG(currX);
            }
        }
    }
    else {
        // update the STAT register even if the ppu is disabled
        updateSTAT();
    }

    // before returning control to the main loop we have to lock 
    // or unlock video related memory depending on the current PPU mode
    // and on the lcd enable flag
    lockRamAreas(regs.LCDC.lcdEnable);


    TracyPlot(plotDotCounter, (int64_t)mDotCounter);
    TracyPlot(plotLY, (int64_t)regs.LY);
    TracyPlot(plotPpuMode, (int64_t)regs.STAT.ppuMode);

    
    // handle HDMA transfers
    // hdma hblank transfers, as the name suggests, only happen during the hblank
    // mode of the ppu
    hdma.step(regs.LCDC.lcdEnable && regs.STAT.ppuMode == PPUMode::HBlank);

    return frameReady;
}

void PPU::writeLCDC(uint8_t val)
{
    LCDCReg old = regs.LCDC;

    regs.LCDC.fromU8(val);


    if (old.lcdEnable != regs.LCDC.lcdEnable) {
        // when the lcd is turned on or off dot counter and LY are reset to 0 
        // and STAT is updated
        mDotCounter = 0;
        regs.LY = 0;

        if (!regs.LCDC.lcdEnable) {
            mOamScanRegister.reset();
            mFirstStep = true;
            display.clear();
        }

        updateSTAT();
    }

}


void PPU::lockRamAreas(bool lock)
{
    if (lock) {
        // ram areas are locked depending on the current ppu mode
        switch (regs.STAT.ppuMode) {
        default:
        case PPUMode::HBlank:
        case PPUMode::VBlank:
            oamRam.lock(false);
            vram.lock(false);
            colors.lockIndexRegs(false);
            break;
        case PPUMode::OAMScan:
            oamRam.lock(true);
            vram.lock(false);
            colors.lockIndexRegs(true);
            break;
        case PPUMode::Draw:
            oamRam.lock(true);
            vram.lock(true);
            colors.lockIndexRegs(true);
            break;
        }
    }
    else {
        oamRam.lock(false);
        vram.lock(false);
        colors.lockIndexRegs(false);
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

    // ppu mode is always hblank when the ppu is disabled
    if (!regs.LCDC.lcdEnable)
        regs.STAT.ppuMode = PPUMode::HBlank;
}

void PPU::oamScan()
{
    // during the OAM scan phase the ppu checks which of the 40 possible OAMs
    // should be drawn on the current line LY

    // the 40 OAMs are scanned sequentially and of those 40, up to 10 OAMs can be 
    // selected for each scanline, priority is given to the oams with the lowest id

    // the oam scan register is always sorted by oam id

    mOamScanRegister.reset();

    int32_t currY = regs.LY;

    for (uint8_t id = 0; id < OAMRam::oamCount && !mOamScanRegister.full(); ++id) {
        auto oam = oamRam.getOAMData(id);

        int32_t objY = oam.y() - 16;

        // yTop is the first line of the object
        // yBottom is the first line AFTER the object
        auto yTop = objY;
        auto yBottom = objY + (regs.LCDC.objDoubleH ? 16 : 8);

        if (currY >= yTop && currY < yBottom)
            mOamScanRegister.add(oam);
    }
}


PPU::OAMDataPtrList PPU::findCurrOams(uint32_t currX) const
{
    // we scan the register to find if we have objects whose x-range corresponds the
    // the currX position of the screen

    // the oam scan register is already sorted by oam id so the ptr list
    // returned by this function is sorted by oam id as well

    OAMDataPtrList oams;

    for (auto& currOam : mOamScanRegister) {
        auto xLeft = currOam.x() - 8;
        auto xRight = xLeft + 8;

        if ((int32_t)currX >= xLeft && (int32_t)currX < xRight)
            oams.add(&currOam);
    }

    return oams;
}



void PPU::renderPixelDMG(uint32_t dispX)
{
    uint8_t bgColorId = 0;
    auto bgColorVal = whiteA;
  
    // get background info for this pixel
    bool hasWindow = renderPixelDMGGetWinVal(dispX, bgColorId);
    if (!hasWindow) {
        // if the window must not be displayed or if the current pixel 
        // is not involved with the window we have to get the regular bg pixel
        bgColorId = renderPixelDMGGetBgVal(dispX);
    }

    bgColorVal = dmgVal2RGB(regs.BGP.id2val(bgColorId));

    // get the back buffer for the display
    auto& dispBuf = display.getBackBuf();
    
    // get objects info for this pixel
    auto objsPixInfo = renderPixelGetObjsValues(dispX);

    if (objsPixInfo.empty()) {
        // no objects for this pixel, draw the background color
        dispBuf(dispX, regs.LY) = bgColorVal;
    }
    else {
        // we have a list of colors and associated information:
        // - colors with priority == false must be drawn above the background, unless they are 0 (transparent)
        // - colors with priority == true must be drawn behind the background, only if the background color is 0 (transparent)
        // we start from top to bottom, from the first object, if the object is transparent then we go down and check the next object
        // then, when objects above the background are over we check the background color, if it's 0 we also check for objects that must
        // be drawn behind the background, etc.

        auto objIt = objsPixInfo.begin();
        
        // check object colors above the background (priority == false)
        while (objIt != objsPixInfo.end() && !objIt->priority) {

            if (objIt->colorId != 0) {
                dispBuf(dispX, regs.LY) = objIt->colorVal;
                return;
            }
            ++objIt;
        }

        // done with objects above the background, check if the background is 0 and objects might be drawn behind it
        if (bgColorId != 0) {
            dispBuf(dispX, regs.LY) = bgColorVal;
            return;
        }

        // draw the color of the first object behind the background (if any), otherwise draw the background color
        if (objIt != objsPixInfo.end())
            dispBuf(dispX, regs.LY) = objIt->colorVal;
        else
            dispBuf(dispX, regs.LY) = bgColorVal;
    }
}

uint8_t PPU::renderPixelDMGGetBgVal(uint32_t dispX)
{
    // if bit 0 of the LCDC reg is false the background and window will be blank (white)
    if (!regs.LCDC.bgWinEnable)
        return 0;

    // we are rendering the display pixel with coordinates (dispX, dispY)
    uint32_t dispY = regs.LY;

    // get the coordinates of the bg corresponding to the current display coordinates
    uint32_t bgX = (dispX + regs.SCX) % 256;
    uint32_t bgY = (dispY + regs.SCY) % 256;
        
    // get the current background tile map
    auto bgTileMap = vram.getTileMap(regs.LCDC.bgTileMapArea);
    // get the current tile id 
    auto bgTileId = bgTileMap.get(bgX / 8, bgY / 8);
    // get tile data
    auto bgTile = vram.getBgTile(bgTileId, regs.LCDC.bgWinTileDataArea);

    return bgTile.get(bgX % 8, bgY % 8);
}

bool PPU::renderPixelDMGGetWinVal(uint32_t dispX, uint8_t& colorId)
{
    // if bit 0 of the LCDC reg is false the background and window will be blank (white)
    // if bit 5 of the LCDC reg is false the window is disabled
    if (!regs.LCDC.bgWinEnable || !regs.LCDC.winEnable) {
        colorId = 0;
        return false;
    }

    // check if the current display coordinate is inside the window
    uint32_t dispY = regs.LY;
    uint32_t winX = regs.WX - 7; // x coord of the window must always be shifted by 7
    uint32_t winY = regs.WY;

    if (dispY < regs.WY || dispX < winX) {
        colorId = 0;
        return false;
    }

    // find the coordinates in the background space
    uint32_t bgX = dispX - winX;
    uint32_t bgY = dispY - winY;

    // get the current background tile map
    auto bgTileMap = vram.getTileMap(regs.LCDC.winTileMapArea);
    // get the current tile id 
    auto bgTileId = bgTileMap.get(bgX / 8, bgY / 8);
    // get tile data
    auto bgTile = vram.getBgTile(bgTileId, regs.LCDC.bgWinTileDataArea);

    colorId = bgTile.get(bgX % 8, bgY % 8);

    return true;
}

PPU::OAMPixelInfoList PPU::renderPixelGetObjsValues(uint32_t currX)
{
    // to find the color ids of the objects on the current pixel we first have to find from which
    // objects we have to extract color data

    OAMPixelInfoList pixInfo;

    // get the objects involved in the current pixel
    for (auto oam : findCurrOams(currX)) {

        // TODO check signed/unsigned math
        auto oamAttr = oam->attr();

        // find coordinates inside the object
        uint32_t objX = currX - (oam->x() - 8);
        uint32_t objY = regs.LY - (oam->y() - 16);

        // flip x and y coordinates if needed
        if (oamAttr.hFlip())
            objX = 7 - objX;

        if (oamAttr.vFlip())
            objY = (regs.LCDC.objDoubleH ? 15 : 7) - objY;

        // get tile data from vram
        auto tile = vram.getObjTile(oam->tileId(), regs.LCDC.objDoubleH);

        PixelInfo info;
        info.oam = oam;
        info.colorId = tile.get(objX, objY);
        info.priority = oamAttr.priority();

        if (mIsCgb) {
            // in the CGB the palette is picked considering the cgb palette value in 
            // the oam attribute
            auto palette = colors.getObjPalette(oamAttr.cgbObjPalette());
            info.colorVal = palette.getColor(info.colorId);
        }
        else {
            // in the DMG only one of the two OBP0 or OBP1 palettes can be used
            auto& obp = oamAttr.dmgPalette() ? regs.OBP1 : regs.OBP0;
            info.colorVal = dmgVal2RGB(obp.id2val(info.colorId));
        }

        pixInfo.add(info);
    }

    // sort the objects so that the first is also the one with the highest priority

    if (mIsCgb) {
        // in the CGB priority is always given to the object with the lowest id, regardless of coordinates

        std::sort(pixInfo.begin(), pixInfo.end(), [](const PixelInfo& lhs, const PixelInfo& rhs) {
            return lhs.oam->tileId() < rhs.oam->tileId();
        });
    }
    else {
        // in the DMG different objects will be given priority as follows:
        // - if objects have different X coordinates: priority is given to the one with the lowest X
        // - if objects have the same X coordinates: priority is given to the one with the lowest ID
        // in this sorting we put the objects with the priority flag == false first

        std::sort(pixInfo.begin(), pixInfo.end(), [](const PixelInfo& lhs, const PixelInfo& rhs) {
            if (lhs.priority != rhs.priority)
                return lhs.priority ? false : true;
            else if (lhs.oam->x() == rhs.oam->x())
                return lhs.oam->tileId() < rhs.oam->tileId();
            else
                return lhs.oam->x() < rhs.oam->x();
        });
    }

    return pixInfo;
}

std::optional<PPU::PixelInfo> PPU::renderPixelGetObjInfo(uint32_t currX)
{
    // to find the color ids of the objects on the current pixel we first have to find from which
    // objects we have to extract color

    // the list returned by findCurrOams() is sorted by oam id so the pixel info list
    // built here will be sorted by oam id as well

    OAMPixelInfoList pixInfo;

    // get the objects involved in the current pixel
    for (auto oam : findCurrOams(currX)) {

        // TODO check signed/unsigned math
        auto oamAttr = oam->attr();

        // find coordinates inside the object
        uint32_t objX = currX - (oam->x() - 8);
        uint32_t objY = regs.LY - (oam->y() - 16);

        // flip x and y coordinates if needed
        if (oamAttr.hFlip())
            objX = 7 - objX;

        if (oamAttr.vFlip())
            objY = (regs.LCDC.objDoubleH ? 15 : 7) - objY;

        // get tile data from vram
        auto tile = vram.getObjTile(oam->tileId(), regs.LCDC.objDoubleH, oamAttr.vramBank());
        auto colorId = tile.get(objX, objY);

        // if the color id of the current object is 0 (transparent) we can skip it 
        if (colorId == 0)
            continue;


        PixelInfo info;
        info.oam = oam;
        info.colorId = colorId;
        info.priority = oamAttr.priority();

        if (mIsCgb) {
            // in the CGB the palette is picked considering the cgb palette value in 
            // the oam attribute
            // BUT, when in DMG compatibility mode, the old monochrome OBP0 and OBP1 are
            // still used to index into the color palette OBP0 and OBP1
            if (mUseDmgCompatMode) {
                auto& obp = oamAttr.dmgPalette() ? regs.OBP1 : regs.OBP0;
                auto palette = colors.getObjPalette(oamAttr.dmgPalette() ? 1 : 0);
                info.colorVal = palette.getColor(obp.id2val(info.colorId));
            }
            else {
                auto palette = colors.getObjPalette(oamAttr.cgbObjPalette());
                info.colorVal = palette.getColor(info.colorId);
            }
        }
        else {
            // in the DMG only one of the two OBP0 or OBP1 palettes can be used
            auto& obp = oamAttr.dmgPalette() ? regs.OBP1 : regs.OBP0;
            info.colorVal = dmgVal2RGB(obp.id2val(info.colorId));
        }

        pixInfo.add(info);
    }

    if (pixInfo.empty())
        return {};


    // sort the objects so that the first is also the one with the highest priority

    if (mIsCgb) {
        // in the CGB priority is always given to the object with the lowest oam id, regardless of coordinates
        // so, considering that the list is already sorted by oam id there is no need to sort it
    }
    else {
        // in the DMG different objects will be given priority as follows:
        // - if objects have different X coordinates: priority is given to the one with the lowest X
        // - if objects have the same X coordinates: priority is given to the one with the lowest ID
        // in this sorting we put the objects with the priority flag == false first

        std::sort(pixInfo.begin(), pixInfo.end(), [](const PixelInfo& lhs, const PixelInfo& rhs) {
            if (lhs.priority != rhs.priority)
                return lhs.priority ? false : true;
            else if (lhs.oam->x() == rhs.oam->x())
                return lhs.oam->tileId() < rhs.oam->tileId();
            else
                return lhs.oam->x() < rhs.oam->x();
        });
    }


    return *pixInfo.begin();
}

void PPU::renderPixelCGB(uint32_t dispX)
{
    // get background info for this pixel
    auto bg = renderPixelCGBGetBgVal(dispX);

    // get the back buffer for the display
    auto& dispBuf = display.getBackBuf();

    // if objects are not enabled in LCDC draw the bg color and return
    if (!regs.LCDC.objEnable) {
        dispBuf(dispX, regs.LY) = bg.colorVal;
        return;
    }

    // get objects info for this pixel
    
    if (auto objInfo = renderPixelGetObjInfo(dispX); objInfo.has_value()) {
        // find which color must be drawn on screen, it's not really easy to understand,
        // use the table shown here: https://gbdev.io/pandocs/Tile_Maps.html#bg-to-obj-priority-in-cgb-mode

        auto obj = objInfo.value();
        bool drawObj = false;

        if (!regs.LCDC.bgWinEnable) {
            // if bgWinEnable is 0, priority always goes to the object
            drawObj = true;
        }
        else if (!bg.priority && !obj.priority) {
            // otherwise, if both bgPriority and obj priority are 0, priority goes to the obj
            drawObj = true;
        }
        else if (bg.colorId == 0) {
            // otherwise, priority goes to the bg UNLESS the color id is 0, in that case 
            // priority still goes to the obj
            drawObj = true;
        }

        dispBuf(dispX, regs.LY) = drawObj
            ? obj.colorVal 
            : bg.colorVal;
    }
    else {
        // no objects for this pixel, draw the background color
        dispBuf(dispX, regs.LY) = bg.colorVal;
    }
}


PPU::PixelInfo PPU::renderPixelCGBGetBgVal(uint32_t dispX)
{
    // we are rendering the display pixel with coordinates (dispX, dispY)
    uint32_t dispY = regs.LY;
    uint32_t bgX = 0;
    uint32_t bgY = 0;
    bool tileArea = false;

    // first check if it's in the window
    // the x coord of the window must always be shifted by 7
    uint32_t winX = regs.WX - 7; 
    
    if (!regs.LCDC.winEnable || dispY < regs.WY || dispX < winX) {
        // no, pick from the background
        bgX = (dispX + regs.SCX) % 256;
        bgY = (dispY + regs.SCY) % 256;
        tileArea = regs.LCDC.bgTileMapArea;
    }
    else {
        // yes, use window coordinates
        bgX = dispX - winX;
        bgY = dispY - regs.WY;
        tileArea = regs.LCDC.winTileMapArea;
    }

    // get the current tile id and background attributes
    auto bgTileId = vram.getTileMap(tileArea).get(bgX / 8, bgY / 8);
    auto bgAttr = vram.getAttrMap(tileArea).getBgMapAttr(bgX / 8, bgY / 8);

    // get tile data
    auto bgTile = vram.getBgTile(bgTileId, regs.LCDC.bgWinTileDataArea, bgAttr.vramBank());

    // get current palette
    // when in DMG compatibility mode make sure that only BGP0 is used
    auto palette = colors.getBgPalette(mUseDmgCompatMode ? 0 : bgAttr.cgbBgPalette());

    uint32_t tileX = bgX % 8;
    uint32_t tileY = bgY % 8;

    if (bgAttr.hFlip())
        tileX = 7 - tileX;
    if (bgAttr.vFlip())
        tileY = 7 - tileY;

    PixelInfo bgInfo;
    bgInfo.oam = nullptr;
    bgInfo.colorId = bgTile.get(tileX, tileY);
    bgInfo.colorVal = palette.getColor(bgInfo.colorId);
    bgInfo.priority = bgAttr.priority();

    return bgInfo;
}




BgHelper PPU::getBgHelper(BgHelperTileMap mapSelection, BgHelperTileAddressing tileAddressing)
{
    BgHelperConfig config;
    config.tileMapSelection = mapSelection;
    config.tileAddressing = tileAddressing;
    config.lcdcTileMapBit = regs.LCDC.bgTileMapArea;
    config.lcdcTileAddressingBit = regs.LCDC.bgWinTileDataArea;

    return BgHelper(vram, colors, regs.BGP, config);
}


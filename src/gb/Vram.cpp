

#include "Vram.h"
#include <cassert>
#include <algorithm>


// ------------------------------------------------------------------------------------------------
// TileData
// ------------------------------------------------------------------------------------------------

uint8_t TileData::getImpl(uint32_t x, uint32_t y) const
{
    // see https://gbdev.io/pandocs/Tile_Data.html for an explanation of 
    // how pixel data is stored in memory

    auto* data = ptr + (y * 2);

    // pixel data is stored in "reverse", e.g: for x == 0 the data is in bit 7
    uint32_t i = 7 - x;

    uint8_t bitLo = (data[0] >> i) & 1;
    uint8_t bitHi = (data[1] >> i) & 1;

    return bitLo | (bitHi << 1);
}

void TileData::setImpl(uint32_t x, uint32_t y, uint8_t val)
{
    if (val > 3)
        val = 3;

    auto* data = ptr + (y * 2);
    uint32_t i = 7 - x;

    uint8_t byteHi = (val >> 1) & 1;
    uint8_t byteLo = val & 1;

    data[0] |= byteLo << i;
    data[1] |= byteHi << i;
}




// ------------------------------------------------------------------------------------------------
// ObjTileData
// ------------------------------------------------------------------------------------------------

uint8_t ObjTileData::getImpl(uint32_t x, uint32_t y) const
{
    if (y >= TileData::h) {
        y -= TileData::h;
        return tdh.get(x, y);
    }
    else {
        return td.get(x, y);
    }
}

void ObjTileData::setImpl(uint32_t x, uint32_t y, uint8_t val)
{
    if (y >= TileData::h) {
        y -= TileData::h;
        tdh.set(x, y, val);
    }
    else {
        td.set(x, y, val);
    }
}




// ------------------------------------------------------------------------------------------------
// TileMap
// ------------------------------------------------------------------------------------------------

uint8_t TileMap::getImpl(uint32_t x, uint32_t y) const
{
    // a tile map is a 32x32 grid where each cell contains the id of 
    // a background tile

    return ptr[y * TileMap::w + x];
}

void TileMap::setImpl(uint32_t x, uint32_t y, uint8_t val)
{
    ptr[y * TileMap::w + x] = val;
}

void TileMap::fillRgbaBuffer(RgbaBufferIf& buf) const
{
    for (uint32_t y = 0; y < mHeight; ++y) {
        for (uint32_t x = 0; x < mWidth; ++x) {
            auto val = get(x, y);
            buf(x, y) = { val, val, val, 255 };
        }
    }
}



// ------------------------------------------------------------------------------------------------
// VRam
// ------------------------------------------------------------------------------------------------

VRam::VRam()
    : mIsCgb(false)
    , mBank0(mmap::vram::start)
    , mBank1(mmap::vram::start)
{
    reset();
}

void VRam::reset()
{
    lock(false);

    mVbkReg = 0;
    mBank0.reset();
    mBank1.reset();
}

uint8_t VRam::read8(uint16_t addr) const
{
    if (mIsCgb && addr == mmap::regs::vbk) {
        return mVbkReg | 0xFE; // only bit 0 is used
    }
    else if (addr >= mmap::vram::start && addr <= mmap::vram::end) {
        if (isLocked())
            return 0xff;

        auto& bank = currBank();

        return bank.read8(addr);
    }
    else {
        return 0xff;
    }
}

void VRam::write8(uint16_t addr, uint8_t val)
{
    if (mIsCgb && addr == mmap::regs::vbk) {
        mVbkReg = val | 0xFE; // only bit 0 is used
    }
    else if (addr >= mmap::vram::start && addr <= mmap::vram::end) {
        if (isLocked())
            return;

        auto& bank = currBank();

        bank.write8(addr, val);
    }
}


TileData VRam::getGenericTile(uint32_t id, uint8_t bank) const
{
    // get a generic tile in vram, this function is intended to be used for debugging
    // not by the ppu logic
    
    // vram is located between 0x8000 and 0x9800, this means that it can contain up to
    // 384 16-bytes tiles

    if (id >= maxTiles)
        id = maxTiles - 1;

    auto& vramBank = getBank(bank);

    uint16_t addr = vramBank.startAddr() + (uint16_t)id * 16;

    return TileData(addr, vramBank.getPtr(addr));
}



ObjTileData VRam::getObjTile(uint8_t id, bool doubleHeight, uint8_t bank) const
{
    // each tile is identified by an id between 0 and 255
    // OBJ tiles are all located between 0x8000 and 0x8FFF
    // the address of a specific tile is 0x8000 + (id * 16)

    // if we're using double height mode, an odd id is rounded down to the previous even id
    // (hence the bitwise AND with ~0x01)

    // CGB only: tile data can be fetched from either bank 0 or bank 1, depending on BG map attributes
    //          the DMG always reads from bank 0

    if (doubleHeight)
        id &= ~0x01;

    auto& vramBank = getBank(bank);

    uint16_t addr = vramBank.startAddr() + (id * 16);

    return ObjTileData(addr, vramBank.getPtr(addr));
}

TileData VRam::getBgTile(uint8_t id, bool addressingMode, uint8_t bank) const
{
    // the location in VRAM of a background tile is determined by its id and the addressing
    // mode (bit 4 of the LCDC register):
    // - when the bit is 1 the location is 0x8000 + (id * 16), the range is 0x8000 - 0x8FFF
    //      and bg/win tiles completely share the same address space as that of obj tiles
    // - when the bit is 0 the location is 0x9000 + ((int8)id * 16), the range is 0x8800 - 0x97FF 
    //      and bg/win tiles share the only half address space with obj tiles

    // background and window tiles can't be 8x16, only 8x8

    // CGB only: tile data can be fetched from either bank 0 or bank 1, depending on BG map attributes
    //          the DMG always reads from bank 0

    uint16_t addr;

    auto& vramBank = getBank(bank);

    if (addressingMode)
        addr = vramBank.startAddr() + id * 16;
    else 
        addr = (vramBank.startAddr() + 0x1000) + ((int8_t)id * 16);

    return TileData(addr, vramBank.getPtr(addr));
}

TileMap VRam::getTileMap(bool hi) const
{
    // there are two 32x32 tile maps in VRAM bank 0, one between 0x9800 and 0x9BFF
    // and one between 0x9C00 and 0x9FFF

    // which map is selected depends on LCDC bit 3

    // each one contains the IDs of the tiles used to compose the background
    // or the window
    // each map contains the IDs starting from the top left of the map

    uint16_t addr = hi ? 0x9C00 : 0x9800;

    return TileMap(addr, mBank0.getPtr(addr));
}

AttrMap VRam::getAttrMap(bool hi) const
{
    // CGB only
    // the attribute maps work just like the tile maps but they're located in VRAM bank 1

    uint16_t addr = hi ? 0x9C00 : 0x9800;

    return AttrMap(addr, mBank1.getPtr(addr));
}




// ------------------------------------------------------------------------------------------------
// OAMRam
// ------------------------------------------------------------------------------------------------

OAMData OAMRam::getOAMData(uint8_t id) const
{
    // OAM data is stored between 0xFE00 and 0xFEA0 (160 bytes in total),
    // each OAM record is 4-bytes long so we 40 OAM record that can be 
    // stored simultaneously

    assert(id < oamCount);

    uint16_t addr = mStartAddr + (id * OAMData::size);

    return OAMData(addr, getPtr(addr), id);
}



// ------------------------------------------------------------------------------------------------
// Display
// ------------------------------------------------------------------------------------------------

Display::Display()
    : mIsFrontA(true)
    , mBufA(w, h)
    , mBufB(w, h)
{}

void Display::clear()
{
    mBufA.fill(whiteA);
    mBufB.fill(whiteA);
}

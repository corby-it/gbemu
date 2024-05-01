

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

void TileMap::fillRgbBuffer(RgbBuffer& buf) const
{
    for (uint32_t y = 0; y < mHeight; ++y) {
        for (uint32_t x = 0; x < mWidth; ++x) {
            auto val = get(x, y);
            buf(x, y) = { val, val, val };
        }
    }
}



// ------------------------------------------------------------------------------------------------
// VRam
// ------------------------------------------------------------------------------------------------

ObjTileData VRam::getObjTile(uint8_t id, bool doubleHeight) const
{
    // each tile is identified by an id between 0 and 255
    // OBJ tiles are all located between 0x8000 and 0x8FFF
    // the address of a specific tile is 0x8000 + (id * 16)

    // if we're using double height mode, an odd id is rounded down to the previous even id
    // (hence the bitwise AND with 0xFE)
    
    if (doubleHeight)
        id &= 0xFE;

    uint16_t addr = mStartAddr + (id * 16);

    return ObjTileData(addr, getPtr(addr));
}

TileData VRam::getBgTile(uint8_t id, bool addressingMode) const
{
    // the location in VRAM of a background tile is determined by its id and the addressing
    // mode (bit 4 of the LCDC register):
    // - when the bit is 1 the location is 0x8000 + (id * 16), the range is 0x8000 - 0x8FFF
    //      and bg/win tiles completely share the same address space as that of obj tiles
    // - when the bit is 0 the location is 0x9000 + ((int8)id * 16), the range is 0x8800 - 0x97FF 
    //      and bg/win tiles share the only half address space with obj tiles

    // background and window tiles can't be 8x16, only 8x8

    uint16_t addr;

    if (addressingMode)
        addr = mStartAddr + id * 16;
    else 
        addr = (mStartAddr + 0x1000) + ((int8_t)id * 16);

    return TileData(addr, getPtr(addr));
}

TileMap VRam::getTileMap(bool hi) const
{
    // there are two 32x32 tile maps in VRAM, one between 0x9800 and 0x9BFF
    // and one between 0x9C00 and 0x9FFF

    // which map is selected depends on LCDC bit 3

    // each one contains the IDs of the tiles used to compose the background
    // or the window
    // each map contains the IDs starting from the top left of the map

    uint16_t addr = hi ? 0x9C00 : 0x9800;

    return TileMap(addr, getPtr(addr));
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

    return OAMData(addr, getPtr(addr));
}



// ------------------------------------------------------------------------------------------------
// Display
// ------------------------------------------------------------------------------------------------

DisplayBuf::DisplayBuf(uint32_t w, uint32_t h)
    : Matrix(w, h)
    , mData(std::make_unique<uint8_t[]>(w* h))
{}


uint8_t DisplayBuf::getImpl(uint32_t x, uint32_t y) const
{
    assert(x < mWidth);
    assert(y < mHeight);
    return mData[y * mWidth + x];
}

void DisplayBuf::setImpl(uint32_t x, uint32_t y, uint8_t val)
{
    assert(x < mWidth);
    assert(y < mHeight);
    
    if (val > 3)
        val = 3;

    mData[y * mWidth + x] = val;
}

void DisplayBuf::clear()
{
    // to clear the display we fill the memory with zeros (white)
    std::fill(mData.get(), mData.get() + (mWidth * mHeight), 0);
}



Display::Display()
    : mIsFrontA(true)
    , mBufA(w, h)
    , mBufB(w, h)
{}

void Display::clear()
{
    mBufA.clear();
    mBufB.clear();
}

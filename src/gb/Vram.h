

#ifndef GBEMU_SRC_GB_VRAM_H_
#define GBEMU_SRC_GB_VRAM_H_

#include "Ram.h"
#include "GbCommons.h"
#include "Matrix.h"
#include <cereal/cereal.hpp>


struct MemoryMappedObj {
    MemoryMappedObj(uint16_t gbAddr, uint8_t* p, size_t l)
        : gbAddr(gbAddr)
        , ptr(p)
        , len(l)
    {}

    uint16_t gbAddr;
    uint8_t* ptr;
    size_t len;
};



// ------------------------------------------------------------------------------------------------
// TileData
// ------------------------------------------------------------------------------------------------

struct TileData : public MemoryMappedObj, public Matrix {
    TileData(uint16_t gbAddr, uint8_t* p)
        : MemoryMappedObj(gbAddr, p, size)
        , Matrix(w, h)
    {}
    
    // the data for a tile is 16 bytes long, each tile is an 8x8 bitmap
    // where each pixel is 2 bits deep (8x8x2 = 128 bits = 16 bytes)

    static constexpr uint8_t w = 8;
    static constexpr uint8_t h = 8;

    static constexpr size_t size = 16;

private:
    uint8_t getImpl(uint32_t x, uint32_t y) const override;
    void setImpl(uint32_t x, uint32_t y, uint8_t val) override;

};




// ------------------------------------------------------------------------------------------------
// ObjTileData
// ------------------------------------------------------------------------------------------------

struct ObjTileData : public Matrix {
    ObjTileData(uint16_t gbAddr, uint8_t* p)
        : Matrix(TileData::w, TileData::h * 2)
        , td(gbAddr, p)
        , tdh(gbAddr + TileData::size , td.ptr + TileData::size)
    {}

    // since obj can be made of 1 or 2 tiles we always store two tile data
    // references here, the rest of the rendering logic will know when to use 
    // the additional tile

    TileData td;
    TileData tdh;

    static constexpr size_t size = TileData::size * 2;


private:
    uint8_t getImpl(uint32_t x, uint32_t y) const override;
    void setImpl(uint32_t x, uint32_t y, uint8_t val) override;

};



// ------------------------------------------------------------------------------------------------
// TileMap
// ------------------------------------------------------------------------------------------------

struct TileMap : public MemoryMappedObj, public Matrix {
    TileMap(uint16_t gbAddr, uint8_t* p)
        : MemoryMappedObj(gbAddr, p, size)
        , Matrix(w, h)
    {}

    void fillRgbaBuffer(RgbaBufferIf& buf) const override;


    static constexpr uint8_t w = 32;
    static constexpr uint8_t h = 32;

    static constexpr size_t size = 1024;


private:
    uint8_t getImpl(uint32_t x, uint32_t y) const override;
    void setImpl(uint32_t x, uint32_t y, uint8_t val) override;

};




// ------------------------------------------------------------------------------------------------
// OAMAttr
// ------------------------------------------------------------------------------------------------

struct OAMAttr : public MemoryMappedObj {
    // Display attributes for a an object
    // bit  function
    // 0..2 CGB palette: CGB-only
    // 3    Bank: CGB-only
    // 4    DMG palette:    0 = use OBP0
    //                      1 = use OBP1
    // 5    horizontal flip
    // 6    vertical flip
    // 7    priority:   0 = no
    //                  1 = BG and window colors 1 to 3 are drawn over this object

    OAMAttr(uint16_t gbAddr, uint8_t* p)
        : MemoryMappedObj(gbAddr, p, size)
    {}

    static constexpr size_t size = 1;


    bool dmgPalette() { return (*ptr & 0x10) >> 4; }
    bool hFlip() { return (*ptr & 0x20) >> 5; }
    bool vFlip() { return (*ptr & 0x40) >> 6; }
    bool priority() { return (*ptr & 0x80) >> 7; }

    void setDmgPalette(bool val) {
        if (val)
            *ptr |= 0x10;
        else 
            *ptr &= ~0x10;
    }

    void setHFlip(bool val) {
        if (val)
            *ptr |= 0x20;
        else
            *ptr &= ~0x20;
    }

    void setVFlip(bool val) {
        if (val)
            *ptr |= 0x40;
        else
            *ptr &= ~0x40;
    }

    void setPriority(bool val) {
        if (val)
            *ptr |= 0x80;
        else
            *ptr &= ~0x80;
    }
};



// ------------------------------------------------------------------------------------------------
// OAMData
// ------------------------------------------------------------------------------------------------

struct OAMData : public MemoryMappedObj {

    OAMData(uint16_t gbAddr, uint8_t* p)
        : MemoryMappedObj(gbAddr, p, size)
    {}

    static constexpr size_t size = 4;

    // Y position of the object -16
    uint8_t& y() const { return ptr[0]; }
    // X position of the object -8
    uint8_t& x() const { return ptr[1]; }
    // tile ID associated with this object 
    uint8_t& tileId() const { return ptr[2]; }
    // Attributes
    OAMAttr attr() const { return OAMAttr(gbAddr + 3, ptr + 3); }

    bool isInside(uint32_t dispX, uint32_t dispY, bool doubleHeight = false) const 
    {
        const int32_t xl = x() - 8;
        const int32_t xr = x();
        const int32_t yt = y() - 16;
        const int32_t yb = doubleHeight ? y() : y() - 8;

        return (int32_t)dispX >= xl && (int32_t)dispX < xr && (int32_t)dispY >= yt && (int32_t)dispY < yb;
    }
};




// ------------------------------------------------------------------------------------------------
// VRam
// ------------------------------------------------------------------------------------------------

class VRam : public LockableRam<8_KB> {
public:
    VRam() : LockableRam(mmap::vram::start) {}

    TileData getGenericTile(uint32_t id) const;

    ObjTileData getObjTile(uint8_t id, bool doubleHeight) const;
    TileData getBgTile(uint8_t id, bool addressingMode) const;

    TileMap getTileMap(bool hi) const;

    static constexpr size_t maxTiles = 384;

private:

};

CEREAL_CLASS_VERSION(VRam, 1);



// ------------------------------------------------------------------------------------------------
// OAMRam
// ------------------------------------------------------------------------------------------------

class OAMRam : public LockableRam<160> {
public:
    OAMRam() : LockableRam(mmap::oam::start) {}

    OAMData getOAMData(uint8_t id) const;

    static constexpr uint8_t oamCount = 40;

private:


};

CEREAL_CLASS_VERSION(OAMRam, 1);





// ------------------------------------------------------------------------------------------------
// Display
// ------------------------------------------------------------------------------------------------

class DisplayBuf : public Matrix {
public:
    DisplayBuf(uint32_t w, uint32_t h);

    void clear();

    uint8_t* data() { return mData.get(); }
    size_t size() const { return mWidth * mHeight; }

private:
    uint8_t getImpl(uint32_t x, uint32_t y) const override;
    void setImpl(uint32_t x, uint32_t y, uint8_t val) override;


    std::unique_ptr<uint8_t[]> mData;

};



class Display {
public:
    // use double buffering for drawing to the display

    Display();

    void clear();

    static constexpr uint8_t w = 160;
    static constexpr uint8_t h = 144;

    DisplayBuf& getFrontBuf() { return mIsFrontA ? mBufA : mBufB; }
    const DisplayBuf& getFrontBuf() const { return mIsFrontA ? mBufA : mBufB; }

    DisplayBuf& getBackBuf() { return mIsFrontA ? mBufB : mBufA; }
    const DisplayBuf& getBackBuf() const { return mIsFrontA ? mBufB : mBufA; }

    DisplayBuf& getA() { return mBufA; }
    DisplayBuf& getB() { return mBufB; }

    void swapBufs() { mIsFrontA = !mIsFrontA; }

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mIsFrontA,
            cereal::binary_data(mBufA.data(), mBufA.size()), 
            cereal::binary_data(mBufB.data(), mBufB.size())
        );
    }

private:

    bool mIsFrontA;
    DisplayBuf mBufA;
    DisplayBuf mBufB;

};

CEREAL_CLASS_VERSION(Display, 1);



#endif // GBEMU_SRC_GB_VRAM_H_


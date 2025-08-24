

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
// BGMapAttr
// ------------------------------------------------------------------------------------------------

struct BGMapAttr : public MemoryMappedObj {
    // CGB only
    // Defines attributes for one tile in the TileMap
    // bit  function
    // 0..2 Color palette: picks a BG palette from 0 to 7
    // 3    Bank: chooses from which VRam bank tile data must be loaded
    // 4    unused but r/w
    // 5    horizontal flip
    // 6    vertical flip
    // 7    priority

    BGMapAttr(uint16_t gbAddr, uint8_t* p)
        : MemoryMappedObj(gbAddr, p, size)
    {}

    static constexpr size_t size = 1;


    uint8_t cgbBgPalette() { return *ptr & 0x07; }
    uint8_t vramBank() { return (*ptr & 0x08) >> 3; }
    bool hFlip() { return (*ptr & 0x20) >> 5; }
    bool vFlip() { return (*ptr & 0x40) >> 6; }
    bool priority() { return (*ptr & 0x80) >> 7; }


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
// AttrMap
// ------------------------------------------------------------------------------------------------

struct AttrMap : public TileMap {
    AttrMap(uint16_t gbAddr, uint8_t* p)
        : TileMap(gbAddr, p)
    {}

    // CGB only
    // an attribute map is basically a TileMap (same size, same structure, etc.),
    // we only need to add a method that returns a BGMapAttr object instead of simply a uint8_t value

    BGMapAttr getBgMapAttr(uint32_t x, uint32_t y) const {

        assert(x < w && y < h);

        uint32_t offset = y * TileMap::w + x;
        
        return BGMapAttr(gbAddr + (uint16_t)offset, ptr + offset);
    }
};



// ------------------------------------------------------------------------------------------------
// OAMAttr
// ------------------------------------------------------------------------------------------------

struct OAMAttr : public MemoryMappedObj {
    // Display attributes for a an object
    // bit  function
    // 0..2 CGB palette: picks one of the object color palettes (CGB-only)
    // 3    Bank: chooses from which VRam bank this OAM data must be loaded (CGB-only)
    // 4    DMG palette:    0 = use OBP0 (DMG only)
    //                      1 = use OBP1
    // 5    horizontal flip
    // 6    vertical flip
    // 7    priority:   0 = no
    //                  1 = BG and window colors 1 to 3 are drawn over this object

    OAMAttr(uint16_t gbAddr, uint8_t* p)
        : MemoryMappedObj(gbAddr, p, size)
    {}

    static constexpr size_t size = 1;

    uint8_t cgbObjPalette() { return *ptr & 0x07; }
    uint8_t vramBank() { return (*ptr & 0x08) >> 3; }
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

    OAMData(uint16_t gbAddr, uint8_t* p, uint8_t id)
        : MemoryMappedObj(gbAddr, p, size)
        , oamId(id)
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


    // OAM number associated with this object (order in which the 
    // oam appears in oam memory)
    uint8_t oamId;

};




// ------------------------------------------------------------------------------------------------
// VRam
// ------------------------------------------------------------------------------------------------

class VRam : public ReadWriteIf, public Lockable {
public:
    VRam();

    void reset();
    void setIsCgb(bool val) { mIsCgb = val; }

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

    TileData getGenericTile(uint32_t id, uint8_t bank = 0) const;

    ObjTileData getObjTile(uint8_t id, bool doubleHeight, uint8_t bank = 0) const;
    TileData getBgTile(uint8_t id, bool addressingMode, uint8_t bank = 0) const;

    TileMap getTileMap(bool hi) const;
    AttrMap getAttrMap(bool hi) const;

    static constexpr uint16_t startAddr = mmap::vram::start;
    static constexpr size_t maxTiles = 384;


    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mVbkReg, mBank0, mBank1);
    }

    Ram<8_KB>& getBank(uint8_t n) {
        return n == 0 ? mBank0 : mBank1;
    }

    const Ram<8_KB>& getBank(uint8_t n) const {
        return n == 0 ? mBank0 : mBank1;
    }


private:

    Ram<8_KB>& currBank() {
        // only bit 0 is used to choose the memory bank
        return mVbkReg & 0x01 ? mBank1 : mBank0;
    }

    const Ram<8_KB>& currBank() const {
        // only bit 0 is used to choose the memory bank
        return mVbkReg & 0x01 ? mBank1 : mBank0;
    }


    bool mIsCgb;

    // in the DMG only bank 0 is available in the VRAM address space
    // in the CGB two 8KB banks are available in the same address space

    uint8_t mVbkReg;
    Ram<8_KB> mBank0;
    Ram<8_KB> mBank1;

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


class Display {
public:
    // use double buffering for drawing to the display

    Display();

    void clear();

    static constexpr uint8_t w = 160;
    static constexpr uint8_t h = 144;

    RgbaBuffer& getFrontBuf() { return mIsFrontA ? mBufA : mBufB; }
    const RgbaBuffer& getFrontBuf() const { return mIsFrontA ? mBufA : mBufB; }

    RgbaBuffer& getBackBuf() { return mIsFrontA ? mBufB : mBufA; }
    const RgbaBuffer& getBackBuf() const { return mIsFrontA ? mBufB : mBufA; }

    RgbaBuffer& getA() { return mBufA; }
    RgbaBuffer& getB() { return mBufB; }

    void swapBufs() { mIsFrontA = !mIsFrontA; }

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mIsFrontA, mBufA, mBufB);
    }

private:

    bool mIsFrontA;
    RgbaBuffer mBufA;
    RgbaBuffer mBufB;

};


CEREAL_CLASS_VERSION(Display, 1);



#endif // GBEMU_SRC_GB_VRAM_H_


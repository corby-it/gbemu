

#ifndef GBEMU_SRC_GB_VRAM_H_
#define GBEMU_SRC_GB_VRAM_H_

#include "Ram.h"
#include "GbCommons.h"


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


struct TileData : public MemoryMappedObj, public Matrix {
    TileData(uint16_t gbAddr, uint8_t* p)
        : MemoryMappedObj(gbAddr, p, size)
        , Matrix(w, h)
    {}
    
    // the data for a tile is 16 bytes long, each tile is an 8x8 bitmap
    // where each pixel is 2 bits deep (8x8x2 = 128 bits = 16 bytes)

    uint8_t getImpl(uint32_t x, uint32_t y) const override;
    void setImpl(uint32_t x, uint32_t y, uint8_t val) override;

    static constexpr uint8_t w = 8;
    static constexpr uint8_t h = 8;

    static constexpr size_t size = 16;
};



struct ObjTileData : public Matrix {
    ObjTileData(uint16_t gbAddr, uint8_t* p)
        : Matrix(TileData::w, TileData::h * 2)
        , td(gbAddr, p)
        , tdh(gbAddr + TileData::size , td.ptr + TileData::size)
    {}

    // since obj can be made of 1 or 2 tiles we always store two tile data
    // references here, the rest of the rendering logic will know when to use 
    // the additional tile

    uint8_t getImpl(uint32_t x, uint32_t y) const override;
    void setImpl(uint32_t x, uint32_t y, uint8_t val) override;

    TileData td;
    TileData tdh;

    static constexpr size_t size = TileData::size * 2;
};


struct TileMap : public MemoryMappedObj, public Matrix {
    TileMap(uint16_t gbAddr, uint8_t* p)
        : MemoryMappedObj(gbAddr, p, size)
        , Matrix(w, h)
    {}

    uint8_t getImpl(uint32_t x, uint32_t y) const override;
    void setImpl(uint32_t x, uint32_t y, uint8_t val) override;

    static constexpr uint8_t w = 32;
    static constexpr uint8_t h = 32;

    static constexpr size_t size = 1024;
};



struct OAMAttr {
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

    OAMAttr(uint8_t val) : val(val) {}

    bool dmgPalette() { return (val & 0x10) >> 4; }
    bool hFlip() { return (val & 0x20) >> 5; }
    bool vFlip() { return (val & 0x40) >> 6; }
    bool priority() { return (val & 0x80) >> 7; }

    uint8_t val;
};


struct OAMData : public MemoryMappedObj {
    OAMData(uint16_t gbAddr, uint8_t* p)
        : MemoryMappedObj(gbAddr, p, size)
    {}

    static constexpr size_t size = 4;

    uint8_t y() const { return ptr[0]; }
    uint8_t x() const { return ptr[1]; }
    uint8_t id() const { return ptr[2]; }
    OAMAttr attr() const { return OAMAttr(ptr[3]); }
};



class VRam : public LockableRam<8 * 1024> {
public:
    VRam() : LockableRam(mmap::vram::start) {}


    ObjTileData getObjTile(uint8_t id, bool doubleHeight) const;
    TileData getBgTile(uint8_t id, bool hiMemArea, bool doubleHeight) const;

    TileMap getTileMap(bool hi) const;

private:

};


class OAMRam : public LockableRam<160> {
public:
    OAMRam() : LockableRam(mmap::oam::start) {}

    OAMData getOAMData(uint8_t id) const;

private:


};



#endif // GBEMU_SRC_GB_VRAM_H_

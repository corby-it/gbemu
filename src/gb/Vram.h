

#ifndef GBEMU_SRC_GB_VRAM_H_
#define GBEMU_SRC_GB_VRAM_H_

#include "Ram.h"
#include "GbCommons.h"


struct MemoryMappedObj {
    MemoryMappedObj(uint16_t gbAddr, const uint8_t* p, size_t l)
        : addr(gbAddr)
        , ptr(p)
        , len(l)
    {}

    uint16_t addr;
    const uint8_t* ptr;
    size_t len;
};


struct TileData : public MemoryMappedObj {
    TileData(uint16_t gbAddr, const uint8_t* p, size_t len)
        : MemoryMappedObj(gbAddr, p, len)
    {}
    
};


struct TileMap : public MemoryMappedObj {
    TileMap(uint16_t gbAddr, const uint8_t* p)
        : MemoryMappedObj(gbAddr, p, size)
    {}

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
    OAMData(uint16_t gbAddr, const uint8_t* p)
        : MemoryMappedObj(gbAddr, p, size)
    {}

    static constexpr size_t size = 4;

    uint8_t y() const { return ptr[0]; }
    uint8_t x() const { return ptr[1]; }
    uint8_t id() const { return ptr[2]; }
    OAMAttr attr() const { return OAMAttr(ptr[3]); }
};



class VRam : public Ram<8 * 1024> {
public:
    VRam() : Ram(mmap::vram::start) {}


    TileData getObjTile(uint8_t id, bool doubleHeight) const;
    TileData getBgTile(uint8_t id, bool hiMemArea, bool doubleHeight) const;

    TileMap getTileMap(bool hi) const;

private:

};


class OAMRam : public Ram<160> {
public:
    OAMRam() : Ram(mmap::oam::start) {}

    OAMData getOAMData(uint8_t id) const;

private:


};



#endif // GBEMU_SRC_GB_VRAM_H_

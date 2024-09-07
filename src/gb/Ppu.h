

#ifndef GBEMU_SRC_GB_PPU_H_
#define GBEMU_SRC_GB_PPU_H_


#include "Bus.h"
#include "Vram.h"
#include "GbCommons.h"
#include <vector>
#include <cereal/cereal.hpp>





// ------------------------------------------------------------------------------------------------
// PPURegs
// ------------------------------------------------------------------------------------------------

struct LCDCReg : public RegU8 {
    LCDCReg();

    // 0xFF40
    // LCDC - LCD Control register
    // This is the main LCD control register. Its bits toggle what elements are displayed on the screen, and how

    // bit  function
    // 0    BG and window enable/priority: if 0 both background and window are blank (white)
    bool bgWinEnable;
    // 1    OBJ enable: if 0 objects are not rendered (can be changed mid scanline)
    bool objEnable;
    // 2    OBJ size: if 0 objs are 8x8, if 1 objs are 8x16
    bool objDoubleH;
    // 3    BG tile map area: if 0 tile map at 0x9800 will be used, if 1 tile map at 0x9C00 will be used
    bool bgTileMapArea;
    // 4    BG and window tile data area: addressing mode for BG and window, if 1 address will start at 0x8000,
    //          if 0 addresses will start at 0x9000 and the tile ID will be signed
    bool bgWinTileDataArea;
    // 5    Window enable: if 0 the window will not be rendered
    bool winEnable;
    // 6    Window tile map area: if 0 tile map at 0x9800 will be used, if 1 tile map at 0x9C00 will be used
    bool winTileMapArea;
    // 7    LCD enable: Enables or disable the LCD and the PPU, when disabled the screen is blank (white)
    bool lcdEnable;

    uint8_t asU8() const override;
    void fromU8(uint8_t b) override;


    template<class Archive>
    void save(Archive& archive, uint32_t const /*version*/) const {
        archive(asU8());
    }

    template<class Archive>
    void load(Archive& archive, uint32_t const /*version*/) {
        uint8_t val;
        archive(val);
        fromU8(val);
    }
};

CEREAL_CLASS_VERSION(LCDCReg, 1);




enum PPUMode : uint8_t {
    HBlank = 0,
    VBlank = 1,
    OAMScan = 2,
    Draw = 3
};

struct STATReg : public RegU8 {
    STATReg();

    // 0xFF41
    // STAT - LCD Status register
    // this register contains the status of the lcd and ppu

    // bit  function
    // 0..1 PPU mode (read-only): contains the PPU current status (modes 0, 1, 2 or 3)
    PPUMode ppuMode;
    // 2    LYC == LY (read-only): true if the current value of LYC is equal to the value of LY
    bool lycEqual;
    // 3    mode 0 interrupt selected: if set, selects mode 0 for interrupt triggering
    bool mode0IrqEnable;
    // 4    mode 1 interrupt selected: if set, selects mode 1 for interrupt triggering
    bool mode1IrqEnable;
    // 5    mode 2 interrupt selected: if set, selects mode 2 for interrupt triggering
    bool mode2IrqEnable;
    // 6    LYC interrupt select: if set, selects LYC==LY for interrupt triggering
    bool lycIrqEnable;
    // 7    unused

    uint8_t asU8() const override;
    void fromU8(uint8_t b) override;
    

    template<class Archive>
    void save(Archive& archive, uint32_t const /*version*/) const {
        archive(asU8());
    }

    template<class Archive>
    void load(Archive& archive, uint32_t const /*version*/) {
        uint8_t val;
        archive(val);
        fromU8(val);
    }
};

CEREAL_CLASS_VERSION(STATReg, 1);



struct PaletteReg : public RegU8 {
    PaletteReg();

    // this register assigns gray shades to the 3 possible colors for a pixel
    // bit  function
    // 0..1 Color value for color ID 0
    uint8_t valForId0 : 2;
    // 2..3 Color value for color ID 1
    uint8_t valForId1 : 2;
    // 4..5 Color value for color ID 2
    uint8_t valForId2 : 2;
    // 6..7 Color value for color ID 3
    uint8_t valForId3 : 2;

    uint8_t asU8() const override;
    void fromU8(uint8_t b) override;

    uint8_t id2val(uint8_t colorId)
    {
        switch (colorId) {
        default:
        case 0: return valForId0;
        case 1: return valForId1;
        case 2: return valForId2;
        case 3: return valForId3;
        }
    }

    void setValForId(uint8_t colorId, uint8_t val)
    {
        switch (colorId) {
        default:
        case 0: valForId0 = val; break;
        case 1: valForId1 = val; break;
        case 2: valForId2 = val; break;
        case 3: valForId3 = val; break;
        }
    }

    void setToDefault() {
        valForId0 = 0;
        valForId1 = 1;
        valForId2 = 2;
        valForId3 = 3;
    }


    template<class Archive>
    void save(Archive& archive, uint32_t const /*version*/) const {
        archive(asU8());
    }

    template<class Archive>
    void load(Archive& archive, uint32_t const /*version*/) {
        uint8_t val;
        archive(val);
        fromU8(val);
    }
};

CEREAL_CLASS_VERSION(PaletteReg, 1);




struct PPURegs {
    PPURegs();

    // 0xFF40
    LCDCReg LCDC;

    // 0xFF41
    STATReg STAT;

    // 0xFF42 and 0xFF43
    // Scrolling registers
    // these two registers specify the top left corner of the 160x144 visible background area
    // within the whole 256x256 background area, values in the range 0-255 are allowed
    // the bottom right coordinates are calculated as follows:
    // BRY = (SCY + 143) % 256
    // BRX = (SCX + 159) % 256
    // the visible area wraps around the background area
    uint8_t SCY;
    uint8_t SCX;

    // 0xFF44
    // LY - LCD Y coordinate
    // this read-only register indicates the current horizontal line of the lcd scan cycle.
    // it can hold values between 0 and 153, with values between 144 and 153 indicating the v-blank period
    uint8_t LY;

    // 0xFF45
    // LY Compare register
    // the gameby constantly compares the value of the LYC and LY registers. When both values are identical, 
    // the "LYC = LY" flag in the STAT register is set, and (if enabled) a STAT interrupt is requested.
    uint8_t LYC;

    // 0xFF47
    // background palette data
    // this register assigns gray shades to the 3 possible colors for background and window tiles
    PaletteReg BGP;

    // 0xFF48 and 0xFF49
    // OBJ palettes 0 and 1
    // they work the same as BGP but are used for objects except that color ID 0
    // is always transparent for objects
    PaletteReg OBP0;
    PaletteReg OBP1;

    // 0xFF4A and 0xFF4b
    // Window position, specified as the position of the top left pixel
    // The WY regsiter contains the Y coordinate in the 160x144 display area
    // The WX register contains the X + 7 coordinate in the 160x144 display area
    uint8_t WY;
    uint8_t WX;


    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(LCDC, STAT, SCY, SCX, LY, LYC, BGP, OBP0, OBP1, WY, WX);
    }

    void reset();
};

CEREAL_CLASS_VERSION(PPURegs, 1);



// ------------------------------------------------------------------------------------------------
// PPU
// ------------------------------------------------------------------------------------------------

struct OAMRegister {
    OAMRegister() {
        oams.reserve(maxCount);
    }

    static constexpr size_t maxCount = 10;

    void reset() {
        oams.clear();
        oams.reserve(10);
    }

    void add(const OAMData& oam) {
        if (oams.size() < maxCount)
            oams.push_back(oam);
    }

    size_t size() const { return oams.size(); }

    OAMData& operator[](size_t i) { return oams[i]; }
    const OAMData& operator[](size_t i) const { return oams[i]; }

    std::vector<OAMData>::iterator begin() { return oams.begin(); }
    std::vector<OAMData>::iterator end() { return oams.end(); }

    std::vector<OAMData>::const_iterator begin() const { return oams.begin(); }
    std::vector<OAMData>::const_iterator end() const { return oams.end(); }

private:
    std::vector<OAMData> oams;

};


class PPU {
public:
    PPU(Bus& bus);

    void reset();

    bool step(uint32_t mCycles);
    void stepLine(uint32_t n = 1);
    void stepFrame(uint32_t n = 1);

    uint32_t getDotCounter() const { return mDotCounter; }
    const OAMRegister& getOamScanRegister() const { return mOamScanRegister; }


    uint8_t readLCDC() const { return regs.LCDC.asU8(); }
    uint8_t readSTAT() const { return regs.STAT.asU8(); }
    uint8_t readSCY() const { return regs.SCY; }
    uint8_t readSCX() const { return regs.SCX; }
    uint8_t readLY() const { return regs.LY; }
    uint8_t readLYC() const { return regs.LYC; }
    uint8_t readBGP() const { return regs.BGP.asU8(); }
    uint8_t readOBP0() const { return regs.OBP0.asU8(); }
    uint8_t readOBP1() const { return regs.OBP1.asU8(); }
    uint8_t readWY() const { return regs.WY; }
    uint8_t readWX() const { return regs.WX; }

    void writeLCDC(uint8_t val);
    void writeSTAT(uint8_t val) { regs.STAT.fromU8(val); }
    void writeSCY(uint8_t val) { regs.SCY = val; }
    void writeSCX(uint8_t val) { regs.SCX = val; }
    void writeLY(uint8_t /*val*/) { } // LY is read-only
    void writeLYC(uint8_t val) { regs.LYC = val; }
    void writeBGP(uint8_t val) { regs.BGP.fromU8(val); }
    void writeOBP0(uint8_t val) { regs.OBP0.fromU8(val); }
    void writeOBP1(uint8_t val) { regs.OBP1.fromU8(val); }
    void writeWY(uint8_t val) { regs.WY = val; }
    void writeWX(uint8_t val) { regs.WX = val; }


    template<class Archive>
    void save(Archive& ar, uint32_t const /*version*/) const {
        ar(regs, vram, oamRam, display);
        ar(mDotCounter, mFirstStep);
    }

    template<class Archive>
    void load(Archive& ar, uint32_t const /*version*/) {
        ar(regs, vram, oamRam, display);
        ar(mDotCounter, mFirstStep);

        // since the oam scan register contains pointer to the memory it can't
        // be serialized directly, it's easier to do an oam scan now and fill it again
        // it's not a problem if the scan doesn't happen at the start of a line
        oamScan();
    }


    PPURegs regs;
    VRam vram;
    OAMRam oamRam;
    Display display;


private:

    struct OAMPixelInfo {
        const OAMData* oam;
        uint8_t colorId;
        uint8_t colorVal;
        bool palette;
        bool priority;
    };
    
    template<typename T, size_t N>
    class List {
    public:
        List()
            : mData()
            , mCount(0)
        {}

        bool empty() const { return mCount == 0; }

        void add(const T& info) {
            if (mCount < N) {
                mData[mCount] = info;
                ++mCount;
            }
        }

        auto begin() { return mData.begin(); }
        auto end() { return mData.begin() + mCount; }


    private:
        std::array<T, N> mData;
        size_t mCount;
    };


    typedef List<const OAMData*, OAMRegister::maxCount> OAMDataPtrList;
    typedef List<OAMPixelInfo, OAMRegister::maxCount>   OAMPixelInfoList;



    void lockRamAreas(bool lock);

    void updateSTAT();
    void oamScan();
    OAMDataPtrList findCurrOams(uint32_t currX) const;

    void onDisabled();

    void renderPixel(uint32_t dispX);
    uint8_t renderPixelGetBgVal(uint32_t dispX);
    bool renderPixelGetWinVal(uint32_t dispX, uint8_t& colorId);
    OAMPixelInfoList renderPixelGetObjsValues(uint32_t currX);

    Bus& mBus;

    uint32_t mDotCounter;
    OAMRegister mOamScanRegister;

    bool mFirstStep;
};

CEREAL_CLASS_VERSION(PPU, 1);




#endif // GBEMU_SRC_GB_PPU_H_

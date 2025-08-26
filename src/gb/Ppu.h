

#ifndef GBEMU_SRC_GB_PPU_H_
#define GBEMU_SRC_GB_PPU_H_


#include "Bus.h"
#include "Vram.h"
#include "GbCommons.h"
#include "Hdma.h"
#include <vector>
#include <optional>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>





// ------------------------------------------------------------------------------------------------
// LCDCReg
// ------------------------------------------------------------------------------------------------

struct LCDCReg : public RegU8 {
    LCDCReg();

    // 0xFF40
    // LCDC - LCD Control register
    // This is the main LCD control register. Its bits toggle what elements are displayed on the screen, and how

    // bit  function
    // 0    BG and window enable/priority: 
    //          DMG: if 0 both background and window are blank (white)
    //          CGB: if 0 bg and window lose their priority, objects will always be displayed on top of them
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



// ------------------------------------------------------------------------------------------------
// STATReg
// ------------------------------------------------------------------------------------------------

enum class PPUMode : uint8_t {
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



// ------------------------------------------------------------------------------------------------
// DMGPaletteReg
// ------------------------------------------------------------------------------------------------

struct DMGPaletteReg : public RegU8 {
    DMGPaletteReg();

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

    static constexpr uint8_t maxIds = 4;


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

CEREAL_CLASS_VERSION(DMGPaletteReg, 1);



// ------------------------------------------------------------------------------------------------
// CGBPalettes
// ------------------------------------------------------------------------------------------------


struct CGBColor {
    
    CGBColor(uint8_t* basePtr = nullptr)
        : ptr(basePtr)
    {}


    static constexpr uint16_t maskR = 0x001F;
    static constexpr uint16_t maskG = 0x03E0;
    static constexpr uint16_t maskB = 0x7C00;
    
    uint8_t R5() const { return uint8_t(raw() & maskR); }
    uint8_t G5() const { return uint8_t((raw() & maskG) >> 5); }
    uint8_t B5() const { return uint8_t((raw() & maskB) >> 10); }

    uint8_t R() const { return conv5to8(R5()); }
    uint8_t G() const { return conv5to8(G5()); }
    uint8_t B() const { return conv5to8(B5()); }

    void setR(uint8_t r);
    void setG(uint8_t g);
    void setB(uint8_t b);

    void set(uint8_t r, uint8_t g, uint8_t  b);


    operator RgbaPixel() const {
        return RgbaPixel(R(), G(), B());
    }

    static uint8_t conv5to8(uint8_t v) {
        // to convert a 5 bit color value to an 8 bit color value 
        // we shift the 5 bits up and fill the lower 3 bits by propagating
        // the top 3 bits
        return (v << 3) | ((v >> 2) & 0x07);
    }

    // colors in the CGB are stored as RGB555 in an u16
    uint16_t raw() const {
        if (!ptr)
            return 0;

        uint8_t lo = ptr[0];
        uint8_t hi = ptr[1];

        return (hi << 8) | lo;
    }

    uint8_t *ptr;
};


struct CGBPalette {
    CGBPalette(uint8_t* basePtr = nullptr)
        : ptr(basePtr)
    {}

    CGBColor getColor(uint8_t idx) const {
        if (!ptr)
            return CGBColor();

        if (idx >= 4)
            idx = 3;

        // each color is 2-byte long
        return CGBColor(ptr + idx * 2);
    }

    uint8_t* ptr;
};


struct CGBPaletteData {

    void resetWhite() {
        std::fill(raw.begin(), raw.end(), 0xFF);
    }

    void resetBlack() {
        std::fill(raw.begin(), raw.end(), 0);
    }

    void resetRandom();

    CGBPalette getPalette(uint8_t idx) {
        if (idx >= 8)
            idx = 7;
        
        // each palette is stored as an 8-byte sequence (4 x u16 colors)
        return CGBPalette(raw.data() + idx * 8);
    }


    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(raw);
    }


    std::array<uint8_t, 64> raw;
};

CEREAL_CLASS_VERSION(CGBPaletteData, 1);





struct CGBPaletteIndexReg : public RegU8 {

    CGBPaletteIndexReg()
        : autoInc(false)
        , index(0)
    {}

    bool autoInc;
    uint8_t index;

    uint8_t asU8() const override {
        // the first 6 bits are used for the index, the top bit is used for auto increment
        uint8_t ret = index & ~0xC0;
        if (autoInc)
            ret |= 0x80;

        return ret;
    }

    void fromU8(uint8_t val) override {
        autoInc = val & 0x80;
        index = val & 0x3F;
    }

    void tryIncIndex() {
        if (autoInc)
            index = (index + 1) & 0x3F;
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

CEREAL_CLASS_VERSION(CGBPaletteIndexReg, 1);





class CGBPalettes : public ReadWriteIf {
public:
    CGBPalettes();

    void reset();

    void setIsCgb(bool val) { mIsCgb = val; }


    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

    CGBPalette getBgPalette(uint8_t idx) { return mBgData.getPalette(idx); }
    CGBPalette getObjPalette(uint8_t idx) { return mObjData.getPalette(idx); }

    // color palette data is only accessible during HBlank and VBlank
    void lockIndexRegs(bool val) { mDataRegsLocked = val; }


    // only for testing and debug
    CGBPaletteData& getBgPaletteData() { return mBgData; }
    CGBPaletteData& getObjPaletteData() { return mObjData; }


    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mBGPIReg, mDataRegsLocked, mOBPIReg, mBgData, mObjData);
    }


private:

    bool mIsCgb;

    bool mDataRegsLocked;
    CGBPaletteIndexReg mBGPIReg;
    CGBPaletteIndexReg mOBPIReg;

    CGBPaletteData mBgData;
    CGBPaletteData mObjData;
};

CEREAL_CLASS_VERSION(CGBPalettes, 1);




// ------------------------------------------------------------------------------------------------
// PPURegs
// ------------------------------------------------------------------------------------------------

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
    DMGPaletteReg BGP;

    // 0xFF48 and 0xFF49
    // OBJ palettes 0 and 1
    // they work the same as BGP but are used for objects except that color ID 0
    // is always transparent for objects
    DMGPaletteReg OBP0;
    DMGPaletteReg OBP1;

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
// BgHelper
// ------------------------------------------------------------------------------------------------

enum class BgHelperTileMap {
    Active,
    At9800,
    At9C00,
};

const char* bgHelperTileMapToStr(BgHelperTileMap bghtm);

enum class BgHelperTileAddressing {
    Active,
    At8000,
    At8800
};

const char* bgHelperTileAddressingToStr(BgHelperTileAddressing bghta);


struct BgHelperConfig {
    BgHelperConfig()
        : tileMapSelection(BgHelperTileMap::Active)
        , tileAddressing(BgHelperTileAddressing::Active)
        , lcdcTileMapBit(false)
        , lcdcTileAddressingBit(false)
    {}

    BgHelperTileMap tileMapSelection;
    BgHelperTileAddressing tileAddressing;

    bool lcdcTileMapBit;
    bool lcdcTileAddressingBit;
};

class BgHelper : public Matrix {
public:
    BgHelper(VRam& vram, BgHelperConfig config)
        : Matrix(w, h)
        , mVram(vram)
        , mConfig(config)
        , mTileMap(0, nullptr)
    {
        switch (mConfig.tileMapSelection) {
        default:
        case BgHelperTileMap::Active: mTileMap = mVram.getTileMap(mConfig.lcdcTileMapBit); break;
        case BgHelperTileMap::At9800: mTileMap = mVram.getTileMap(false); break;
        case BgHelperTileMap::At9C00: mTileMap = mVram.getTileMap(true); break;
        }
    }

    uint8_t getImpl(uint32_t x, uint32_t y) const override
    {
        return getTile(x / 8, y / 8).get(x % 8, y % 8);
    }
    
    void setImpl(uint32_t /*x*/, uint32_t /*y*/, uint8_t /*val*/) override
    {
        // don't set anything, this helper class is meant to be used 
        // for reading the background data
    }

    uint8_t getTileId(uint32_t r, uint32_t c) const
    {
        return mTileMap.get(c, r);
    }

    TileData getTile(uint32_t r, uint32_t c) const
    {
        // get the tile id
        auto tileId = getTileId(r, c);

        bool getTileParam = true;

        switch (mConfig.tileAddressing) {
        default:
        case BgHelperTileAddressing::Active: getTileParam = mConfig.lcdcTileAddressingBit; break;
        case BgHelperTileAddressing::At8000: getTileParam = true; break;
        case BgHelperTileAddressing::At8800: getTileParam = false; break;
        }

        return mVram.getBgTile(tileId, getTileParam);
    }

    TileMap tileMap() const { return mTileMap; }

    static constexpr uint32_t w = 256;
    static constexpr uint32_t h = 256;

    static constexpr uint32_t rows = 32;
    static constexpr uint32_t cols = 32;


private:
    VRam& mVram;
    BgHelperConfig mConfig;
    TileMap mTileMap;

};



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
    bool full() const { return oams.size() >= maxCount; }

    OAMData& operator[](size_t i) { return oams[i]; }
    const OAMData& operator[](size_t i) const { return oams[i]; }

    std::vector<OAMData>::iterator begin() { return oams.begin(); }
    std::vector<OAMData>::iterator end() { return oams.end(); }

    std::vector<OAMData>::const_iterator begin() const { return oams.begin(); }
    std::vector<OAMData>::const_iterator end() const { return oams.end(); }

private:
    std::vector<OAMData> oams;

};


class PPU : public ReadWriteIf {
public:
    PPU(Bus& bus);

    void reset();

    void setIsCgb(bool val);

    bool step(uint32_t mCycles);
    void stepLine(uint32_t n = 1);
    void stepFrame(uint32_t n = 1);

    uint32_t getDotCounter() const { return mDotCounter; }
    const OAMRegister& getOamScanRegister() const { return mOamScanRegister; }

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;



    template<class Archive>
    void save(Archive& ar, uint32_t const /*version*/) const {
        ar(regs, colors, vram, oamRam, display);
        ar(mDotCounter, mFirstStep);
    }

    template<class Archive>
    void load(Archive& ar, uint32_t const /*version*/) {
        ar(regs, colors, vram, oamRam, display);
        ar(mDotCounter, mFirstStep);

        // since the oam scan register contains pointer to the memory it can't
        // be serialized directly, it's easier to do an oam scan now and fill it again
        // it's not a problem if the scan doesn't happen at the start of a line
        oamScan();
    }


    PPURegs regs;
    CGBPalettes colors;
    Hdma hdma;
    VRam vram;
    OAMRam oamRam;
    Display display;


    BgHelper getBgHelper(BgHelperTileMap mapSelection, BgHelperTileAddressing tileAddressing);


private:

    struct PixelInfo {
        const OAMData* oam;
        uint8_t colorId;
        RgbaPixel colorVal;
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
    typedef List<PixelInfo, OAMRegister::maxCount>   OAMPixelInfoList;



    void lockRamAreas(bool lock);

    void writeLCDC(uint8_t val);

    void updateSTAT();
    void oamScan();
    OAMDataPtrList findCurrOams(uint32_t currX) const;


    void renderPixelDMG(uint32_t dispX);
    uint8_t renderPixelDMGGetBgVal(uint32_t dispX);
    bool renderPixelDMGGetWinVal(uint32_t dispX, uint8_t& colorId);

    void renderPixelCGB(uint32_t dispX);
    PixelInfo renderPixelCGBGetBgVal(uint32_t dispX);
    
    OAMPixelInfoList renderPixelGetObjsValues(uint32_t currX);
    std::optional<PixelInfo> renderPixelGetObjInfo(uint32_t currX);


    Bus* mBus;

    bool mIsCgb;

    uint32_t mDotCounter;
    OAMRegister mOamScanRegister;

    bool mFirstStep;
};

CEREAL_CLASS_VERSION(PPU, 1);




#endif // GBEMU_SRC_GB_PPU_H_

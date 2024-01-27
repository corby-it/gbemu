

#ifndef GBEMU_SRC_GB_PPU_H_
#define GBEMU_SRC_GB_PPU_H_


#include "Bus.h"
#include "Vram.h"
#include "GbCommons.h"
#include <array>






// ------------------------------------------------------------------------------------------------
// PPURegs
// ------------------------------------------------------------------------------------------------

struct LCDCReg : public RegU8 {
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
};



enum PPUMode : uint8_t {
    HBlank = 0,
    VBlank = 1,
    OAMScan = 2,
    Draw = 3
};

struct STATReg :public RegU8 {
    // 0xFF41
    // STAT - LCD Status register
    // this register contains the status of the lcd and ppu

    // bit  function
    // 0..1 PPU mode (read-only): contains the PPU current status (modes 0, 1, 2 or 3)
    PPUMode ppuMode : 2;
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
};


struct PaletteReg : public RegU8 {
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
};


struct PPURegs {

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


    void reset();
};



// ------------------------------------------------------------------------------------------------
// PPU
// ------------------------------------------------------------------------------------------------

struct OAMRegister {
    OAMRegister()
        : count(0)
    {}

    static constexpr size_t maxCount = 10;

    void reset() {
        count = 0;
    }

    void add(const OAMData& oam) {
        if(count < maxCount)
            oams[count++] = oam;
    }

    std::array<OAMData, maxCount> oams;
    size_t count;
};


class PPU {
public:
    PPU(Bus& bus);

    void reset();

    void step(uint16_t mCycles);

    PPURegs regs;

    uint32_t getDotCounter() const { return mDotCounter; }

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

private:


    void updateSTAT();
    void oamScan();
    bool findCurrOam(OAMData& oam, uint32_t currX) const;

    void renderPixel(uint32_t dispX);
    uint8_t renderPixelGetBgVal(uint32_t dispX);
    bool renderPixelGetWinVal(uint32_t dispX, uint8_t& colorId);
    bool renderPixelGetObjVal(uint32_t dispX, uint8_t& colorId, bool& palette, bool& priority);

    Bus& mBus;

    uint32_t mDotCounter;
    OAMRegister mOamScanRegister;

    VRam mVram;
    OAMRam mOamRam;
    Display mDisplay;

};





#endif // GBEMU_SRC_GB_PPU_H_

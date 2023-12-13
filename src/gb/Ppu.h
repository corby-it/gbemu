

#ifndef GBEMU_SRC_GB_PPU_H_
#define GBEMU_SRC_GB_PPU_H_


#include "Vram.h"


struct LCDCReg {
    // 0xFF40
    // LCDC - LCD Control register
    // This is the main LCD control register. Its bits toggle what elements are displayed on the screen, and how
    // bit  function
    // 0    BG and window enable/priority: if 0 both background and window are blank (white)
    // 1    OBJ enable: if 0 objects are not rendered (can be changed mid scanline)
    // 2    OBJ size: if 0 objs are 8x8, if 1 objs are 8x16
    // 3    BG tile map area: if 0 tile map at 0x9800 will be used, if 1 tile map at 0x9C00 will be used
    // 4    BG and window tile data area: addressing mode for BG and window, if 1 address will start at 0x8000,
    //          if 0 addresses will start at 0x9000 and the tile ID will be signed
    // 5    Window enable: if 0 the window will not be rendered
    // 6    Window tile map area: if 0 tile map at 0x9800 will be used, if 1 tile map at 0x9C00 will be used
    // 7    LCD enable: Enables or disable the LCD and the PPU, when disabled the screen is blank (white)

    bool bgWinEnable;
    bool objEnable;
    bool objDoubleH;
    bool bgTileMapArea;
    bool bgWinTileDataArea;
    bool winEnable;
    bool winTileMapArea;
    bool lcdEnable;

    uint8_t asU8() const;
    void fromU8(uint8_t b);
};

struct STATReg {
    // 0xFF41
    // STAT - LCD Status register
    // this register contains the status of the lcd and ppu
    // bit  function
    // 0..1 PPU mode (read-only): contains the PPU current status (modes 0, 1, 2 or 3)
    // 2    LYC == LY (read-only): true if the current value of LYC is equal to the value of LY
    // 3    mode 0 interrupt selected: if set, selects mode 0 for interrupt triggering
    // 4    mode 1 interrupt selected: if set, selects mode 1 for interrupt triggering
    // 5    mode 2 interrupt selected: if set, selects mode 2 for interrupt triggering
    // 6    LYC interrupt select: if set, selects LYC==LY for interrupt triggering
    // 7    unused

    uint8_t ppuMode : 2;
    bool lycEqual;
    bool mode0IrqEnable;
    bool mode1IrqEnable;
    bool mode2IrqEnable;
    bool lycIrqEnable;

    uint8_t asU8() const;
    void fromU8(uint8_t b);
};

struct PaletteReg {
    // this register assigns gray shades to the 3 possible colors for a pixel
    // bit  function
    // 0..1 Color ID 0
    // 2..3 Color ID 1
    // 4..5 Color ID 2
    // 6..7 Color ID 3
    uint8_t id0 : 2;
    uint8_t id1 : 2;
    uint8_t id2 : 2;
    uint8_t id3 : 2;

    uint8_t asU8() const;
    void fromU8(uint8_t b);
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

};




class PPU {
public:
    PPU();

    void step(uint16_t mCycles);


    PPURegs regs;

private:

    VRam mVram;
    OAMRam mOamRam;

};





#endif // GBEMU_SRC_GB_PPU_H_

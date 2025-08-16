

#include "Ppu.h"
#include "Irqs.h"
#include <tracy/Tracy.hpp>
#include <algorithm>
#include <array>


// ------------------------------------------------------------------------------------------------
// PPURegs
// ------------------------------------------------------------------------------------------------

LCDCReg::LCDCReg()
    : bgWinEnable(false)
    , objEnable(false)
    , objDoubleH(false)
    , bgTileMapArea(false)
    , bgWinTileDataArea(false)
    , winEnable(false)
    , winTileMapArea(false)
    , lcdEnable(false)
{}

uint8_t LCDCReg::asU8() const
{
    uint8_t val = (uint8_t)bgWinEnable
        | objEnable << 1
        | objDoubleH << 2
        | bgTileMapArea << 3
        | bgWinTileDataArea << 4
        | winEnable << 5
        | winTileMapArea << 6
        | lcdEnable << 7;

    return val;
}

void LCDCReg::fromU8(uint8_t b)
{
    bgWinEnable = b & 0x01;
    objEnable = (b & 0x02) >> 1;
    objDoubleH = (b & 0x04) >> 2;
    bgTileMapArea = (b & 0x08) >> 3;
    bgWinTileDataArea = (b & 0x10) >> 4;
    winEnable = (b & 0x20) >> 5;
    winTileMapArea = (b & 0x40) >> 6;
    lcdEnable = (b & 0x80) >> 7;
}


STATReg::STATReg()
    : ppuMode(PPUMode::OAMScan)
    , lycEqual(false)
    , mode0IrqEnable(false)
    , mode1IrqEnable(false)
    , mode2IrqEnable(false)
    , lycIrqEnable(false)
{}

uint8_t STATReg::asU8() const
{
    uint8_t val = (static_cast<uint8_t>(ppuMode) & 0x03)
        | lycEqual << 2
        | mode0IrqEnable << 3
        | mode1IrqEnable << 4
        | mode2IrqEnable << 5
        | lycIrqEnable << 6;

    return val;
}

void STATReg::fromU8(uint8_t b)
{
    // PPU mode and lyc==ly are read-only
    mode0IrqEnable = (b & 0x08u) << 3;
    mode1IrqEnable = (b & 0x10u) << 4;
    mode2IrqEnable = (b & 0x20u) << 5;
    lycIrqEnable = (b & 0x40u) << 6;
}


PaletteReg::PaletteReg()
    : valForId0(0)
    , valForId1(1)
    , valForId2(2)
    , valForId3(3)
{}

uint8_t PaletteReg::asU8() const
{
    uint8_t val = valForId0
        | valForId1 << 2
        | valForId2 << 4
        | valForId3 << 6;

    return val;
}

void PaletteReg::fromU8(uint8_t b)
{
    valForId0 = b & 0x03;
    valForId1 = (b & 0x0C) >> 2;
    valForId2 = (b & 0x30) >> 4;
    valForId3 = (b & 0xC0) >> 6;
}



PPURegs::PPURegs()
    : SCY(0)
    , SCX(0)
    , LY(0)
    , LYC(0)
    , WY(0)
    , WX(0)
{}

void PPURegs::reset()
{
    // initialize the PPU registers with their default values
    // from: https://gbdev.gg8.se/wiki/articles/Power_Up_Sequence

    LCDC.fromU8(0x91);
    STAT.fromU8(0x00);

    SCY = 0;
    SCX = 0;
    
    LY = 0;
    LYC = 0;

    BGP.fromU8(0xFC);

    OBP0.fromU8(0xFF);
    OBP1.fromU8(0xFF);

    WY = 0;
    WX = 0;
}



// ------------------------------------------------------------------------------------------------
// BgHelper
// ------------------------------------------------------------------------------------------------

const char* bgHelperTileMapToStr(BgHelperTileMap bghtm)
{
    switch (bghtm) {
    case BgHelperTileMap::Active: return "Active";
    case BgHelperTileMap::At9800: return "At 0x9800";
    case BgHelperTileMap::At9C00: return "At 0x9C00";
    default:
        return "unknown";
    }
}

const char* bgHelperTileAddressingToStr(BgHelperTileAddressing bghta)
{
    switch (bghta) {
    case BgHelperTileAddressing::Active: return "Active";
    case BgHelperTileAddressing::At8000: return "At 0x8000";
    case BgHelperTileAddressing::At8800: return "At 0x8800";
    default:
        return "unknown";
    }
}




// ------------------------------------------------------------------------------------------------
// PPU
// ------------------------------------------------------------------------------------------------

namespace lcdreg = mmap::regs::lcd;



PPU::PPU(Bus& bus)
    : mBus(&bus)
    , mIsCgb(false)
{
    reset();
}

void PPU::reset()
{
    mDotCounter = 0;
    mOamScanRegister.reset();
    mFirstStep = true;

    regs.reset();
    vram.reset();
    oamRam.reset();
    display.clear();
    
    // at reset update the STAT register to actually reflect the current
    // status of the PPU
    updateSTAT();

    // lock ram to correctly reflect the current ppu mode and the lcd enable status
    lockRamAreas(regs.LCDC.lcdEnable);
}

void PPU::setIsCgb(bool val)
{
    mIsCgb = val;
    vram.setIsCgb(val);
}


uint8_t PPU::read8(uint16_t addr) const
{
    switch (addr) {
    case lcdreg::lcdc: return regs.LCDC.asU8();
    case lcdreg::stat: return regs.STAT.asU8();
    case lcdreg::scy: return regs.SCY;
    case lcdreg::scx: return regs.SCX;
    case lcdreg::ly: return regs.LY;
    case lcdreg::lyc: return regs.LYC;
    case lcdreg::bgp: return regs.BGP.asU8();
    case lcdreg::obp0: return regs.OBP0.asU8();
    case lcdreg::obp1: return regs.OBP1.asU8();
    case lcdreg::wy: return regs.WY;
    case lcdreg::wx: return regs.WX;
    default:
        return 0xff;
    }
}

void PPU::write8(uint16_t addr, uint8_t val)
{
    switch (addr) {
    case lcdreg::lcdc: writeLCDC(val); break;
    case lcdreg::stat: regs.STAT.fromU8(val); break;
    case lcdreg::scx: regs.SCX = val; break;
    case lcdreg::scy: regs.SCY = val; break;
    case lcdreg::ly: break; // LY is read-only
    case lcdreg::lyc: regs.LYC = val; break;
    case lcdreg::bgp: regs.BGP.fromU8(val); break;
    case lcdreg::obp0: regs.OBP0.fromU8(val); break;
    case lcdreg::obp1: regs.OBP1.fromU8(val); break;
    case lcdreg::wy: regs.WY = val; break;
    case lcdreg::wx: regs.WX = val; break;
    default:
        break;
    }
}





void PPU::stepLine(uint32_t n)
{
    // step to the next line(s)
    while (n--) {
        step((456 - mDotCounter) / 4);
    }
}

void PPU::stepFrame(uint32_t n)
{
    // step to the next frame(s)
    while (n--) {
        stepLine(154 - regs.LY);
    }
}


bool PPU::step(uint32_t mCycles)
{
    ZoneScoped;

    // the PPU goes through a cycle of its own, separate from that of the CPU.
    // it draws 153 lines, top to bottom and left to right, from line 0 to 143 it 
    // draws the lines seen on the display, from lines 155 to 153 its in the vblank mode
    // 
    // the PPU it goes through 4 modes:
    // mode 2: OAM scan (checks the OAM memory to determine wich objects must be drawn)
    // mode 3: drawing pixels (access VRAM to get pixel values)
    // mode 0: hblank
    // mode 1: vblank

    // in each mode some memory areas might not be accessible to the CPU because they're 
    // locked by the PPU
    // mode 2: OAM memory not accessible
    // mode 3: VRAM and OAM memory not accessible
    // mode 0: everything accessible
    // mode 1: everything accessible

    // every part of this process takes a predefined amount of clock cycles (or machine cycles),
    // here 1 dot == 1 clock cycles (so 4 dots == 1 machine cycle):
    // mode 2: OAM scan -> 80 dots (20 m-cycles)
    // mode 3: draw -> 172 dots (43 m-cycles)
    // mode 0: hblank -> 204 dots (51 m-cycles)
    // mode 1: vblank -> 4560 dots (1140 m-cycles)
    // mode 3 might get longer and bleed into mode 0 but the sum of their dots will still be 
    // 172 + 204 = 376 dots (94 m-cycles)

    // in this function we advance the rendering cycle based on how many machine cycles the cpu just executed
    
    // mDotCounter counts the dots for the current line while the LY register counts the lines,
    // each line has 456 dots

    bool frameReady = false;
    
    // do nothing if the lcd and ppu are not enabled 
    if(regs.LCDC.lcdEnable) {

        // before doing anything we have to unlock the memory, the PPU can always access it
        lockRamAreas(false);

        // since the last call 'cCycles' have passed, act accordingly
        uint32_t cCycles = mCycles * 4;

        while (cCycles--) {
            mDotCounter = (mDotCounter + 1) % 456;

            if (mDotCounter == 0) {
                // if the new value of the dot counter is zero it means it wrapped around 
                // and a new line just started
                regs.LY = (regs.LY + 1) % 154;

                // at the beginning of a non-vblank new line the ppu enters mode 2 so we scan the OAM now
                if (regs.LY < 144) {
                    oamScan();

                    // check if we have to trigger mode 2 (OAM Scan) STAT irq
                    if (regs.STAT.mode2IrqEnable) {
                        auto currIF = mBus->read8(mmap::regs::IF);
                        mBus->write8(mmap::regs::IF, Irqs::mask(Irqs::Type::Lcd) | currIF);
                    }
                }
                
                // as soon as we enter v-blank mode the ppu triggers the v-blank interrupt in the cpu
                if (regs.LY == 144) {
                    auto currIF = mBus->read8(mmap::regs::IF);
                    uint8_t newIrqMask = Irqs::mask(Irqs::Type::VBlank);

                    // check if we also have to trigger mode 1 (V-Blank) STAT irq
                    if (regs.STAT.mode1IrqEnable)
                        newIrqMask |= Irqs::mask(Irqs::Type::Lcd);

                    mBus->write8(mmap::regs::IF, newIrqMask | currIF);

                    // when we enter the v-blank mode it means the PPU is done drawing the current frame:
                    // - swap the display buffers top bring the complete frame on the front
                    // - return frameReady = true to tell the rest of the app that the frame is ready
                    display.swapBufs();
                    frameReady = true;
                }

                // check if we have to trigger the LY==LYC irq
                if (regs.STAT.lycIrqEnable && regs.LY == regs.LYC) {
                    auto currIF = mBus->read8(mmap::regs::IF);
                    mBus->write8(mmap::regs::IF, Irqs::mask(Irqs::Type::Lcd) | currIF);
                }
            }
            // check if we have to trigger mode 0 (H-Blank) STAT irq
            if (regs.STAT.mode0IrqEnable && mDotCounter == 252) {
                auto currIF = mBus->read8(mmap::regs::IF);
                mBus->write8(mmap::regs::IF, Irqs::mask(Irqs::Type::Lcd) | currIF);
            }
            if (mFirstStep) {
                mFirstStep = false;
                oamScan();
            }

            // updated the STAT register to reflect the current status of the PPU
            updateSTAT();
        
            // if we are in mode 3 we have to draw the corresponding pixels
            // if we are in a different mode there is nothing to do as OAM scan
            // is handled all at once at the beginning of a new line
            // 
            // the draw mode is 172 dots long, rendering starts after the first 12 dots
            // during which the gpu fetches stuff, after that, the 160 screen dots are actually rendered
            if (regs.STAT.ppuMode == PPUMode::Draw && mDotCounter >= 80 + 12) {
                uint32_t currX = mDotCounter - (80 + 12);
                renderPixel(currX);
            }
        }
    }
    else {
        // update the STAT register even if the ppu is disabled
        updateSTAT();
    }

    // before returning control to the main loop we have to lock 
    // or unlock video related memory depending on the current PPU mode
    // and on the lcd enable flag
    lockRamAreas(regs.LCDC.lcdEnable);

    return frameReady;
}

void PPU::writeLCDC(uint8_t val)
{
    LCDCReg old = regs.LCDC;

    regs.LCDC.fromU8(val);


    if (old.lcdEnable != regs.LCDC.lcdEnable) {
        // when the lcd is turned on or off dot counter and LY are reset to 0 
        // and STAT is updated
        mDotCounter = 0;
        regs.LY = 0;

        if (!regs.LCDC.lcdEnable) {
            mOamScanRegister.reset();
            mFirstStep = true;
            display.clear();
        }

        updateSTAT();
    }

}


void PPU::lockRamAreas(bool lock)
{
    if (lock) {
        // ram areas are locked depending on the current ppu mode
        switch (regs.STAT.ppuMode) {
        default:
        case PPUMode::HBlank:
        case PPUMode::VBlank:
            oamRam.lock(false);
            vram.lock(false);
            break;
        case PPUMode::OAMScan:
            oamRam.lock(true);
            vram.lock(false);
            break;
        case PPUMode::Draw:
            oamRam.lock(true);
            vram.lock(true);
            break;
        }
    }
    else {
        oamRam.lock(false);
        vram.lock(false);
    }
}

void PPU::updateSTAT()
{
    // the first 3 bits of the STAT register are updated depending on the 
    // internal status of the PPU
    regs.STAT.lycEqual = (regs.LY == regs.LYC);

    if (regs.LY >= 144) {
        regs.STAT.ppuMode = PPUMode::VBlank;
    }
    else {
        // TODO handle mode 3 length differences
        if (mDotCounter < 80)
            regs.STAT.ppuMode = PPUMode::OAMScan;
        if (mDotCounter >= 80 && mDotCounter < 252)
            regs.STAT.ppuMode = PPUMode::Draw;
        if (mDotCounter >= 252)
            regs.STAT.ppuMode = PPUMode::HBlank;
    }
}

void PPU::oamScan()
{
    // during the OAM scan phase the ppu checks which of the 40 possible OAMs
    // should be drawn on the current line LY

    // the 40 OAMs are scanned sequentially and of those 40, up to 10 OAMs can be 
    // selected for each scanline, priority is given to the oams with the lowest id

    mOamScanRegister.reset();

    int32_t currY = regs.LY;

    for (uint8_t id = 0; id < OAMRam::oamCount && mOamScanRegister.size() < OAMRegister::maxCount; ++id) {
        auto oam = oamRam.getOAMData(id);

        int32_t objY = oam.y() - 16;

        // yTop is the first line of the object
        // yBottom is the first line AFTER the object
        auto yTop = objY;
        auto yBottom = objY + (regs.LCDC.objDoubleH ? 16 : 8);

        if (currY >= yTop && currY < yBottom)
            mOamScanRegister.add(oam);
    }
}


PPU::OAMDataPtrList PPU::findCurrOams(uint32_t currX) const
{
    // we scan the register to find if we have objects whose x-range corresponds the
    // the currX position of the screen

    OAMDataPtrList oams;

    for (auto& currOam : mOamScanRegister) {
        auto xLeft = currOam.x() - 8;
        auto xRight = xLeft + 8;

        if ((int32_t)currX >= xLeft && (int32_t)currX < xRight)
            oams.add(&currOam);
    }

    return oams;
}



void PPU::renderPixel(uint32_t dispX)
{
    uint8_t bgColorId = 0;
    uint8_t bgColorVal = 0;
  
    // get background info for this pixel
    bool hasWindow = renderPixelGetWinVal(dispX, bgColorId);
    if (!hasWindow) {
        // if the window must not be displayed or if the current pixel 
        // is not involved with the window we have to get the regular bg pixel
        bgColorId = renderPixelGetBgVal(dispX);
    }

    bgColorVal = regs.BGP.id2val(bgColorId);

    // get the back buffer for the display
    auto& dispBuf = display.getBackBuf();
    
    // get objects info for this pixel
    auto objsPixInfo = renderPixelGetObjsValues(dispX);

    if (objsPixInfo.empty()) {
        // no objects for this pixel, draw the background color
        dispBuf.set(dispX, regs.LY, bgColorVal);
    }
    else {
        // we have a list of colors and associated information:
        // - colors with priority == false must be drawn above the background, unless they are 0 (transparent)
        // - colors with priority == true must be drawn behind the background, only if the background color is 0 (transparent)
        // we start from top to bottom, from the first object, if the object is transparent then we go down and check the next object
        // then, when objects above the background are over we check the background color, if it's 0 we also check for objects that must
        // be drawn behind the background, etc.

        auto objIt = objsPixInfo.begin();
        
        // check object colors above the background (priority == false)
        while (objIt != objsPixInfo.end() && !objIt->priority) {

            if (objIt->colorId != 0) {
                dispBuf.set(dispX, regs.LY, objIt->colorVal);
                return;
            }
            ++objIt;
        }

        // done with objects above the background, check if the background is 0 and objects might be drawn behind it
        if (bgColorId != 0) {
            dispBuf.set(dispX, regs.LY, bgColorVal);
            return;
        }

        // draw the color of the first object behind the background (if any), otherwise draw the background color
        if (objIt != objsPixInfo.end())
            dispBuf.set(dispX, regs.LY, objIt->colorVal);
        else 
            dispBuf.set(dispX, regs.LY, bgColorVal);
    }
}

uint8_t PPU::renderPixelGetBgVal(uint32_t dispX)
{
    // if bit 0 of the LCDC reg is false the background and window will be blank (white)
    if (!regs.LCDC.bgWinEnable)
        return 0;

    // we are rendering the display pixel with coordinates (dispX, dispY)
    uint32_t dispY = regs.LY;

    // get the coordinates of the bg corresponding to the current display coordinates
    uint32_t bgX = (dispX + regs.SCX) % 256;
    uint32_t bgY = (dispY + regs.SCY) % 256;
        
    // get the current background tile map
    auto bgTileMap = vram.getTileMap(regs.LCDC.bgTileMapArea);
    // get the current tile id 
    auto bgTileId = bgTileMap.get(bgX / 8, bgY / 8);
    // get tile data
    auto bgTile = vram.getBgTile(bgTileId, regs.LCDC.bgWinTileDataArea);

    return bgTile.get(bgX % 8, bgY % 8);
}

bool PPU::renderPixelGetWinVal(uint32_t dispX, uint8_t& colorId)
{
    // if bit 0 of the LCDC reg is false the background and window will be blank (white)
    // if bit 5 of the LCDC reg is false the window is disabled
    if (!regs.LCDC.bgWinEnable || !regs.LCDC.winEnable) {
        colorId = 0;
        return false;
    }

    // check if the current display coordinate is inside the window
    uint32_t dispY = regs.LY;
    uint32_t winX = regs.WX - 7; // x coord of the window must always be shifted by 7
    uint32_t winY = regs.WY;

    if (dispY < regs.WY || dispX < winX) {
        colorId = 0;
        return false;
    }

    // find the coordinates in the background space
    uint32_t bgX = dispX - winX;
    uint32_t bgY = dispY - winY;

    // get the current background tile map
    auto bgTileMap = vram.getTileMap(regs.LCDC.winTileMapArea);
    // get the current tile id 
    auto bgTileId = bgTileMap.get(bgX / 8, bgY / 8);
    // get tile data
    auto bgTile = vram.getBgTile(bgTileId, regs.LCDC.bgWinTileDataArea);

    colorId = bgTile.get(bgX % 8, bgY % 8);

    return true;
}

PPU::OAMPixelInfoList PPU::renderPixelGetObjsValues(uint32_t currX)
{
    // to find the color ids of the objects on the current pixel we first have to find from which
    // objects we have to extract color data

    OAMPixelInfoList pixInfo;

    // get the objects involved in the current pixel
    for (auto oam : findCurrOams(currX)) {

        // TODO check signed/unsigned math
        auto oamAttr = oam->attr();

        // find coordinates inside the object
        uint32_t objX = currX - (oam->x() - 8);
        uint32_t objY = regs.LY - (oam->y() - 16);

        // flip x and y coordinates if needed
        if (oamAttr.hFlip())
            objX = 7 - objX;

        if (oamAttr.vFlip())
            objY = (regs.LCDC.objDoubleH ? 15 : 7) - objY;

        // get tile data from vram
        auto tile = vram.getObjTile(oam->tileId(), regs.LCDC.objDoubleH);

        OAMPixelInfo info;
        info.oam = oam;
        info.colorId = tile.get(objX, objY);
        info.palette = oamAttr.dmgPalette();
        info.priority = oamAttr.priority();

        auto& obp = info.palette ? regs.OBP1 : regs.OBP0;
        info.colorVal = obp.id2val(info.colorId);
        
        pixInfo.add(info);
    }

    // in the DMG different objects will be given priority as follows:
    // - if objects have different X coordinates: priority is given to the one with the lowest X
    // - if objects have the same X coordinates: priority is given to the one with the lowest ID
    // in this sorting we put the objects with the priority flag == false first

    std::sort(pixInfo.begin(), pixInfo.end(), [](const OAMPixelInfo& lhs, const OAMPixelInfo& rhs) {
        if (lhs.priority != rhs.priority)
            return lhs.priority ? false : true;
        else if (lhs.oam->x() == rhs.oam->x())
            return lhs.oam->tileId() < rhs.oam->tileId();
        else
            return lhs.oam->x() < rhs.oam->x();
    });

    return pixInfo;
}



BgHelper PPU::getBgHelper(BgHelperTileMap mapSelection, BgHelperTileAddressing tileAddressing)
{
    BgHelperConfig config;
    config.tileMapSelection = mapSelection;
    config.tileAddressing = tileAddressing;
    config.lcdcTileMapBit = regs.LCDC.bgTileMapArea;
    config.lcdcTileAddressingBit = regs.LCDC.bgWinTileDataArea;

    return BgHelper(vram, config);
}


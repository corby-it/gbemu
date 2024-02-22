

#ifndef GBEMU_SRC_GB_GAMEBOYCORE_H_
#define GBEMU_SRC_GB_GAMEBOYCORE_H_


#include "Bus.h"
#include "Cpu.h"
#include "Timer.h"
#include "Joypad.h"
#include "Dma.h"
#include "Ppu.h"
#include "Audio.h"
#include "Serial.h"
#include "Cartridge.h"



class GameBoyClassic {
public:
    GameBoyClassic();

    void run();

private:
    GBBus mBus;
    CPU mCpu;
    WorkRam mWram;
    PPU mPpu;
    DMA mDma;
    Cartridge mCartridge;
    Timer mTimer;
    Joypad mJoypad;
    Audio mAudio;
    Serial mSerial;
    HiRam mHiRam;


};


#endif // GBEMU_SRC_GB_GAMEBOYCORE_H_
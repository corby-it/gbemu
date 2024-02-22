

#ifndef GBEMU_SRC_GB_BUS_H_
#define GBEMU_SRC_GB_BUS_H_

#include "Ram.h"


// the GB has a 16-bit address bus that connects the CPU to everything else.
// Everything is memory mapped on the same bus


// ------------------------------------------------------------------------------------------------
// Bus
// ------------------------------------------------------------------------------------------------

class Bus {
public:
    virtual ~Bus() {}

    virtual uint8_t read8(uint16_t addr) const = 0;
    uint16_t read16(uint16_t addr) const;
    
    virtual void write8(uint16_t addr, uint8_t val) = 0;
    void write16(uint16_t addr, uint16_t val);

};



// ------------------------------------------------------------------------------------------------
// TestBus
// ------------------------------------------------------------------------------------------------

// the TestBus class is used for testing, it doesn't map the addresses to actual peripherals,
// it only have a 64kB ram where things can be read from and written to freely
class TestBus : public Bus {
public:

    uint8_t read8(uint16_t addr) const override {
        return mWram.read8(addr);
    }

    void write8(uint16_t addr, uint8_t val) override {
        mWram.write8(addr, val);
    }
    
private:
    Ram<64 * 1024> mWram;

};



// ------------------------------------------------------------------------------------------------
// GBBus
// ------------------------------------------------------------------------------------------------


// Actual bus for the GameBoy

// forward declarations for connected components
class CPU;
class PPU;
class DMA;
class Cartridge;
class Timer;
class Joypad;
class Audio;
class Serial;

typedef Ram<8 * 1024>   WorkRam;
typedef Ram<127>        HiRam;


class GBBus : public Bus {
public:
    GBBus();

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;
    
    void connect(CPU& cpu) { mCpu = &cpu; }
    void connect(WorkRam& wram) { mWram = &wram; }
    void connect(PPU& ppu) { mPpu = &ppu; }
    void connect(DMA& dma) { mDma = &dma; }
    void connect(Cartridge& cart) { mCartridge = &cart; }
    void connect(Timer& t) { mTimer = &t; }
    void connect(Joypad& jp) { mJoypad = &jp; }
    void connect(Audio& au) { mAudio = &au; }
    void connect(Serial& sr) { mSerial = &sr; }
    void connect(HiRam& hiram) { mHiRam = &hiram; }


private:


    CPU* mCpu;
    WorkRam* mWram;
    PPU* mPpu;
    DMA* mDma;
    Cartridge* mCartridge;
    Timer* mTimer;
    Joypad* mJoypad;
    Audio* mAudio;
    Serial* mSerial;
    HiRam* mHiRam;

};



#endif // GBEMU_SRC_GB_BUS_H_
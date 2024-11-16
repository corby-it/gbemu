

#ifndef GBEMU_SRC_GB_BUS_H_
#define GBEMU_SRC_GB_BUS_H_

#include "Ram.h"
#include "GbCommons.h"
#include <cereal/cereal.hpp>


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
    Ram<64_KB> mWram;

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
class APU;
class Serial;

typedef Ram<8_KB>   WorkRam;
typedef Ram<127>        HiRam;

CEREAL_CLASS_VERSION(WorkRam, 1);
CEREAL_CLASS_VERSION(HiRam, 1);



class GBBus : public Bus {
public:
    GBBus();

    uint8_t read8(uint16_t addr) const override { return (this->*mReadFnPtr)(addr); }
    void write8(uint16_t addr, uint8_t val) override { (this->*mWriteFnPtr)(addr, val); }
    
    void connect(CPU& cpu) { 
        mCpu = &cpu;
        switchRWFunctions();
    }
    void connect(WorkRam& wram) { 
        mWram = &wram;
        switchRWFunctions();
    }
    void connect(PPU& ppu) { 
        mPpu = &ppu;
        switchRWFunctions();
    }
    void connect(DMA& dma) { 
        mDma = &dma;
        switchRWFunctions();
    }
    void connect(Cartridge& cart) { 
        mCartridge = &cart;
        switchRWFunctions();
    }
    void connect(Timer& t) { 
        mTimer = &t;
        switchRWFunctions();
    }
    void connect(Joypad& jp) { 
        mJoypad = &jp;
        switchRWFunctions();
    }
    void connect(APU& apu) {
        mApu = &apu;
        switchRWFunctions();
    }
    void connect(Serial& sr) {
        mSerial = &sr; 
        switchRWFunctions();
    }
    void connect(HiRam& hiram) { 
        mHiRam = &hiram;
        switchRWFunctions();
    }


private:
    void switchRWFunctions();

    uint8_t realRead(uint16_t addr) const;
    void realWrite(uint16_t addr, uint8_t val);

    uint8_t dummyRead(uint16_t /*addr*/) const { return 0xFF; }
    void dummyWrite(uint16_t /*addr*/, uint8_t /*val*/) { }


    CPU* mCpu;
    WorkRam* mWram;
    PPU* mPpu;
    DMA* mDma;
    Cartridge* mCartridge;
    Timer* mTimer;
    Joypad* mJoypad;
    APU* mApu;
    Serial* mSerial;
    HiRam* mHiRam;


    using ReadFnType = uint8_t(GBBus::*)(uint16_t) const;
    using WriteFnType = void(GBBus::*)(uint16_t, uint8_t);

    ReadFnType mReadFnPtr;
    WriteFnType mWriteFnPtr;
};



#endif // GBEMU_SRC_GB_BUS_H_
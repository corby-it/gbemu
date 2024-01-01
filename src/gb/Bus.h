

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
class Timer;
class CPU;

class GBBus : public Bus {
public:
    GBBus();

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;
    
    void setCpu(CPU* cpu) { mCpu = cpu; }
    void setTimer(Timer* t) { mTimer = t; }


private:

    Ram<8 * 1024> mWram;

    CPU* mCpu;
    Timer* mTimer;

};



#endif // GBEMU_SRC_GB_BUS_H_
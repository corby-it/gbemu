

#ifndef GBEMU_SRC_GB_BUS_H_
#define GBEMU_SRC_GB_BUS_H_

#include "Ram.h"


// the GB has a 16 address bus that connects the CPU to everything else.
// Everything is memory mapped on the same bus so it's possible to model
// the bus as if it owned everything else in the system


class Bus {
public:
    virtual ~Bus() {}

    virtual uint8_t read8(uint16_t addr) const = 0;
    virtual uint16_t read16(uint16_t addr) const = 0;
    
    virtual void write8(uint16_t addr, uint8_t val) = 0;
    virtual void write16(uint16_t addr, uint16_t val) = 0;

};




// the TestBus class is used for testing, it doesn't map the addresses to actual peripherals,
// it only have a 64kB ram where things can be read from and written to freely
class TestBus : public Bus {
public:

    uint8_t read8(uint16_t addr) const override;
    uint16_t read16(uint16_t addr) const override;

    void write8(uint16_t addr, uint8_t val) override;
    void write16(uint16_t addr, uint16_t val) override;

private:
    WRam<64*1024> mWram;

};


#endif // GBEMU_SRC_GB_BUS_H_
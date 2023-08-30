

#ifndef GBEMU_SRC_GB_BUS_H_
#define GBEMU_SRC_GB_BUS_H_

#include "Ram.h"


// the GB has a 16 address bus that connects the CPU to everything else.
// Everything is memory mapped on the same bus so it's possible to model
// the bus as if it owned everything else in the system


class Bus {
public:

    uint8_t read8(uint16_t addr) const;
    uint16_t read16(uint16_t addr) const;

    void write8(uint16_t addr, uint8_t val);
    void write16(uint16_t addr, uint16_t val);

private:
    WRam mWram;

};



#endif // GBEMU_SRC_GB_BUS_H_
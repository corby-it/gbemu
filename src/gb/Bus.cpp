

#include "Bus.h"


uint8_t Bus::read8(uint16_t addr) const
{
    return mWram.read8(addr);
}

uint16_t Bus::read16(uint16_t addr) const
{
    return mWram.read16(addr);
}

void Bus::write8(uint16_t addr, uint8_t val)
{
    mWram.write8(addr, val);
}

void Bus::write16(uint16_t addr, uint16_t val)
{
    mWram.write16(addr, val);
}
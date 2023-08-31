

#include "Bus.h"


uint8_t TestBus::read8(uint16_t addr) const
{
    return mWram.read8(addr);
}

uint16_t TestBus::read16(uint16_t addr) const
{
    return mWram.read16(addr);
}

void TestBus::write8(uint16_t addr, uint8_t val)
{
    mWram.write8(addr, val);
}

void TestBus::write16(uint16_t addr, uint16_t val)
{
    mWram.write16(addr, val);
}
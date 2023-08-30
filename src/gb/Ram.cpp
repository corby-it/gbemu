

#include "Ram.h"
#include <cassert>


uint8_t WRam::read8(uint16_t addr) const
{
    return mData[addr];
}

uint16_t WRam::read16(uint16_t addr) const
{
    assert(addr < 64*1024 - 1);
    // the GB is little endian so:
    // - the byte at [addr] is the lsb
    // - the byte at [addr+1] is the msb
    return mData[addr] | mData[addr+1];
}

void WRam::write8(uint16_t addr, uint8_t val)
{
    mData[addr] = val;
}

void WRam::write16(uint16_t addr, uint16_t val)
{
    assert(addr < 64*1024 - 1);
    // the GB is little endian so:
    // - the lsb must be written at [addr]
    // - the msb must be written at [addr+1]
    mData[addr] = val & 0xff;
    mData[addr+1] = (val >> 8) & 0xff;
}

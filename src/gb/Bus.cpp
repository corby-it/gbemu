

#include "Bus.h"
#include "Cpu.h"
#include "Timer.h"
#include "Joypad.h"
#include "Dma.h"
#include "Ppu.h"
#include "Apu.h"
#include "Serial.h"
#include "Cartridge.h"
#include "GbCommons.h"
#include <cassert>


// ------------------------------------------------------------------------------------------------
// Bus
// ------------------------------------------------------------------------------------------------

uint16_t Bus::read16(uint16_t addr) const
{
    assert(addr <= UINT16_MAX - 1);
    // the GB is little endian so:
    // - the byte at [addr] is the lsb
    // - the byte at [addr+1] is the msb
    return read8(addr) | (read8(addr + 1) << 8);
}


void Bus::write16(uint16_t addr, uint16_t val)
{
    assert(addr <= UINT16_MAX - 1);
    // the GB is little endian so:
    // - the lsb must be written at [addr]
    // - the msb must be written at [addr+1]
    write8(addr, (uint8_t)val);
    write8(addr + 1, val >> 8);
}




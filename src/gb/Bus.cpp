

#include "Bus.h"
#include "Cpu.h"
#include "Timer.h"
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




// ------------------------------------------------------------------------------------------------
// GBBus
// ------------------------------------------------------------------------------------------------

GBBus::GBBus()
    : mCpu(nullptr)
    , mTimer(nullptr)
{}


uint8_t GBBus::read8(uint16_t addr) const
{
    if (addr >= mmap::wram::start && addr <= mmap::wram::end)
        return mWram.read8(addr - mmap::wram::start);

    if (addr >= mmap::echoram::start && addr <= mmap::echoram::end)
        return mWram.read8(addr - mmap::echoram::start);

    if (addr >= mmap::regs::timer::start && addr <= mmap::regs::timer::end) {
        if (!mTimer)
            return 0;

        if (addr == mmap::regs::timer::DIV) return mTimer->readDIV();
        if (addr == mmap::regs::timer::TIMA) return mTimer->readTIMA();
        if (addr == mmap::regs::timer::TMA) return mTimer->readTMA();
        if (addr == mmap::regs::timer::TAC) return mTimer->readTAC();
    }

    if (addr == mmap::regs::IF)
        return mCpu ? mCpu->irqs.IF : 0;

    if (addr == mmap::IE)
        return mCpu ? mCpu->irqs.IE : 0;

    return 0;
}

void GBBus::write8(uint16_t addr, uint8_t val)
{
    if (addr >= mmap::wram::start && addr <= mmap::wram::end)
        mWram.write8(addr - mmap::wram::start, val);

    if (addr >= mmap::echoram::start && addr <= mmap::echoram::end)
        mWram.write8(addr - mmap::echoram::start, val);

    if (addr >= mmap::regs::timer::start && addr <= mmap::regs::timer::end) {
        if (!mTimer)
            return;

        if (addr == mmap::regs::timer::DIV) mTimer->writeDIV(val);
        if (addr == mmap::regs::timer::TIMA) mTimer->writeTIMA(val);
        if (addr == mmap::regs::timer::TMA) mTimer->writeTMA(val);
        if (addr == mmap::regs::timer::TAC) mTimer->writeTAC(val);
    }

    if (mCpu && addr == mmap::regs::IF)
        mCpu->irqs.IF = val;

    if (mCpu && addr == mmap::IE)
        mCpu->irqs.IE = val;
}



#include "WorkRam.h"


// set RAM start address to 0 and then translate gb addresses appropriately,
// this is necessary otherwise the address value will overflow 16 bits

WorkRam::WorkRam()
    : Ram(0)
    , mIsCgb(false)
    , mCurrBank(1)
{
    reset();
}

void WorkRam::reset()
{
    Ram::reset();

    mCurrBank = 1;
}

void WorkRam::setIsCgb(bool val)
{
    mIsCgb = val;

    if (!mIsCgb) {
        // the DMG only has a single 8KB bank of ram
        mCurrBank = 1;
    }
}


uint8_t WorkRam::read8(uint16_t addr) const
{
    // echo ram access, translate the address to the actual work ram address space
    if (addr >= mmap::echoram::start && addr <= mmap::echoram::end) {
        addr = addr - (mmap::echoram::start - mmap::wram::start);
    }


    // regular ram access
    // the first half of the address space always points to bank 0
    // the second half points to bank 1 - 7 (if the selected bank is 0 then bank 1 is selected)
    if (addr >= mmap::wram::start && addr < mmap::wram::half_start) {
        addr -= mmap::wram::start;
        return Ram::read8(addr);
    }
    else if (addr >= mmap::wram::half_start && addr < mmap::wram::end) {
        addr -= mmap::wram::start;

        uint8_t bank = mCurrBank == 0 ? 1 : mCurrBank;
        addr += (bank - 1) * bankSize;

        return Ram::read8(addr);
    }
    // bank register access (CGB only)
    // only the lower 3 bits are used
    else if (mIsCgb && addr == mmap::regs::svbk) {
        return mCurrBank | 0xF8;
    }

    return 0xff;
}

void WorkRam::write8(uint16_t addr, uint8_t val)
{
    // echo ram access, translate the address to the actual work ram address space
    if (addr >= mmap::echoram::start && addr <= mmap::echoram::end) {
        addr = addr - (mmap::echoram::start - mmap::wram::start);
    }

    // regular ram access
    // the first half of the address space always points to bank 0
    // the second half points to bank 1 - 7 (if the selected bank is 0 then bank 1 is selected)
    if (addr >= mmap::wram::start && addr < mmap::wram::half_start) {
        addr -= mmap::wram::start;
        Ram::write8(addr, val);
    }
    else if (addr >= mmap::wram::half_start && addr < mmap::wram::end) {
        addr -= mmap::wram::start;

        uint8_t bank = mCurrBank == 0 ? 1 : mCurrBank;
        addr += (bank - 1) * bankSize;

        Ram::write8(addr, val);
    }
    // bank register access (CGB only)
    // only the lower 3 bits are used
    else if (mIsCgb && addr == mmap::regs::svbk) {
        mCurrBank = val & 0x07;
    }
}


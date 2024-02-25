

#include "Dma.h"
#include "GbCommons.h"


DMA::DMA(Bus& bus)
    : mBus(bus)
{
    reset();
}

void DMA::reset()
{
    mReg = 0x00;
    mStartAddr = 0x0000;
    mEndAddr = 0x0000;
    mCurrAddr = 0x0000;
    mIsTransferring = false;
}

void DMA::write(uint8_t val)
{
    // the value written in the DMA register is the high byte of a 16-bit address from where 
    // the copy will start, the low byte always starts at 0x00. The DMA chip will always copy 
    // 160 bytes, no matter what and the transfer cannot be stopped or monitored
    // 
    // During the transfer the bus will not be accessible from the CPU as the DMA chip is using it,
    // the only usable memory is that in the HIRAM area between FF80 and FFFE
    // (see https://gbdev.io/pandocs/OAM_DMA_Transfer.html)
    // 
    // for example:
    // val = 0xDE
    // start addr   = 0xDE00
    // end addr     = start addr + 160 (in this case 0xDEA0)
    // curr addr    = start addr
    // the end addr is not included in the copy

    // the destination of the copy is always the OAM area (FE00 - FE9F)

    // the gameboy dev manual states that the starting address can be specified in the range 8000 - DFFF
    // but it's not clear what happens if someone writes a "wrong" address to the DMA register

    // if the transfer is already running a new one cannot start 
    if (mIsTransferring)
        return;

    mReg = val;
    mStartAddr = (val << 8) | 0x00;
    mEndAddr = mStartAddr + 160;
    mCurrAddr = mStartAddr;
    mIsTransferring = true;
}

void DMA::step(uint32_t mCycles)
{
    if (!mIsTransferring)
        return;

    // the DMA chip copies 1 byte for each machine cycle
    while (mCycles--) {
        auto currVal = mBus.read8(mCurrAddr);
        uint16_t offset = mCurrAddr - mStartAddr;

        mBus.write8(mmap::oam::start + offset, currVal);

        ++mCurrAddr;

        if (mCurrAddr >= mEndAddr) {
            mIsTransferring = false;
            break;
        }
    }
}





#include "Dma.h"
#include "GbCommons.h"


DMA::DMA(Bus& bus)
    : mBus(&bus)
{
    reset();
}

void DMA::reset()
{
    mReg = 0x00;
    
    mWrittenAddr = 0x0000;
    mCurrAddr = 0x0000;
    mCounter = 0;
    
    mIsScheduled = false;
    mStartTransfer = false;
    mIsTransferring = false;
}


void DMA::write(uint8_t val)
{
    // the value written in the DMA register is the high byte of a 16-bit address from where 
    // the copy will start, the low byte always starts at 0x00. The DMA chip will always copy 
    // 160 bytes, no matter what and the transfer cannot be stopped or monitored
    // 
    // The transfer starts with a 1 machine cycle delay after the write to the register
    // If a new value is written to the register the old transfer will stop after a 1 m-cycle
    // delay and a new transfer will start
    // 
    // During the transfer the bus will not be accessible from the CPU as the DMA chip is using it,
    // the only usable memory is that in the HIRAM area between FF80 and FFFE
    // (see https://gbdev.io/pandocs/OAM_DMA_Transfer.html)
    // 
    // during the 1 m-cycle delay the bus will still be accessible, unless another transfer is 
    // already running in the background
    // 
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


    // when a write happens just store the value and schedule the transfer for the next cycle
    mReg = val;
    mWrittenAddr = (val << 8) & 0xFF00;
    mIsScheduled = true;
}

void DMA::step(uint32_t mCycles)
{
    while (mCycles--) {
        // if we're in the middle of a transfer or if we have to start a new one 
        // we have to copy a byte and increment the counter
        if (mStartTransfer || mIsTransferring) {
            mStartTransfer = false;
            mIsTransferring = true;

            auto currVal = mBus->read8(mCurrAddr + mCounter);
            
            mBus->write8(mmap::oam::start + mCounter, currVal);

            if (++mCounter >= 160)
                mIsTransferring = false;
        }

        // if a new transfer is scheduled reset the counter and change the address
        // so that on the NEXT CYCLE the new transfer will begin
        if (mIsScheduled) {
            mCurrAddr = mWrittenAddr;
            mCounter = 0;
            mStartTransfer = true;
            mIsScheduled = false;
        }
    }
}

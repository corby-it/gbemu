

#include "Hdma.h"

Hdma::Hdma(Bus& bus)
    : mBus(&bus)
    , mIsCgb(false)
{
    reset();
}


void Hdma::reset()
{
    mMode = HdmaMode::Stopped;


    // the length value represents the number of 16-byte blocks - 1 that must be transferred
    // it's a 7 bit value so the max number of bytes that can be transferred is 0x7F,
    // which results in (7F + 1) * 16 = 2048 bytes 

    mLen = 0x7F;
    mSrc = 0;
    mDst = 0;

    mPrevPpuHblank = false;
    mPauseHblankOnHalt = false;
    mSubcount = 0;
    mSrcInternal = 0;
    mDstInternal = 0;
}

uint8_t Hdma::read8(uint16_t addr) const
{
    // the only readable register is hdma5_len
    if (mIsCgb && addr == mmap::regs::hdma::len) {
        // reading from this register returns the following:
        // - bit 7:     1 transfer is not currently active
        //              0 transfer is currently active
        // - bits 0..6: contain the remaining mLen value
        //
        // when a transfer is over, mLen wraps around and its value becomes 0x7F
        // so a value of 0xFF read from this register indicates that the transfer has completed
        
        if (mMode == HdmaMode::Stopped) 
            return mLen | 0x80;
        else 
            return mLen & ~0x80;
    }
    else {
        return 0xff;
    }
}

void Hdma::write8(uint16_t addr, uint8_t val)
{
    if (!mIsCgb)
        return;

    switch (addr) {
    case mmap::regs::hdma::src_hi: 
        mSrc = (mSrc & ~0xFF00) | (val << 8);
        break;

    case mmap::regs::hdma::src_lo: 
        mSrc = (mSrc & ~0x00FF) | val;
        break;

    case mmap::regs::hdma::dst_hi: 
        mDst = (mDst & ~0xFF00) | (val << 8);
        break;

    case mmap::regs::hdma::dst_lo: 
        mDst = (mDst & ~0x00FF) | val;
        break;

    case mmap::regs::hdma::len:
        // writing to this register starts the transfer:
        // - bit 7 is the mode that will be used
        // - bits 0..6 specify the value of mLen (number of 16-byte blocks -1)

        // it's possible to stop an hblank transfer by writing 0 to bit 7 while it's running
        
        // it's not possible to stop a generic transfer because during the transfer the cpu is halted
        // and there's no way to interact with the transfer

        if (mMode == HdmaMode::HBlank && !(val & 0x80)) {
            mMode = HdmaMode::Stopped;
            mBus->sendEvent(BusEvent::HdmaStopped);
        }
        else {
            mMode = (val & 0x80) ? HdmaMode::HBlank : HdmaMode::Generic;
            mLen = val & 0x7F;
            
            // the source address may be located in rom (0000-7FFF) or work ram/external ram (A000-DFFF)
            // the lower 4 bits of source address are ignored
            // other address spaces will (probably) cause garbage to bo copied to the destination
            mSrcInternal = mSrc & ~0x000F;

            // the destination address is located in vram (8000-9FFF)
            // the lower 4 bits are ignored and the top 3 bits are ignored as well
            // so that the address is always in vram
            mDstInternal = (mDst & ~0xE00F) | mmap::vram::start;

            // reset internal stuff
            mPrevPpuHblank = false;
            mPauseHblankOnHalt = false;
            mSubcount = 0;

            // tell everybody that a generic transfer started
            if(mMode == HdmaMode::Generic)
                mBus->sendEvent(BusEvent::HdmaStarted);
        }

        break;

    default:
        break;
    }
}

void Hdma::step(bool isPPUInHblank)
{
    if (!mIsCgb)
        return;

    // this function is called for each machine cycles of the PPU
    // (this means that double clock speeds won't affect HDMA timing)
    
    // in both normal speed and double speed mode it takes about 8 us to transfer 
    // a block of 16 bytes. That is, 8 m-cycles in Normal Speed Mode, 
    // and 16 "fast" m-cycles in Double Speed Mode.
    // this means that for each m-cycle 2 bytes are transferred.

    bool run = false;
    bool sendEvt = false;
    BusEvent evt = BusEvent::HdmaStopped;

    if(mMode == HdmaMode::Stopped) {
        run = false;
        sendEvt = false;
    }
    else if (mMode == HdmaMode::Generic) {
        run = true;
    }
    else if (mMode == HdmaMode::HBlank) {

        // when running in hblank mode we have to tell the rest of the hardware that 
        // the hdma started transferring because the cpu has to switch to halt mode

        if (!mPrevPpuHblank && isPPUInHblank) {
            // entered hblank mode
            sendEvt = true;
            evt = BusEvent::HdmaStarted;
        }
        else if (mPrevPpuHblank && !isPPUInHblank) {
            // finished hblank mode
            sendEvt = true;
            evt = BusEvent::HdmaStopped;
        }

        // if the cpu entered the halt state on its own by executing the HALT instruction
        // the hblank transfer is paused
        run = isPPUInHblank && !mPauseHblankOnHalt;
    }

    mPrevPpuHblank = isPPUInHblank;


    if (run) {
        // copy 2 bytes
        for (uint32_t i = 0; i < 2; ++i) {

            mBus->write8(mDstInternal, mBus->read8(mSrcInternal));
            
            // increment src, dst and subcounter
            mSrcInternal++;
            mDstInternal++;
            mSubcount++;

            if (mSubcount == 16) {
                // a 16-byte block has been transferred
                mSubcount = 0;
                
                if (--mLen == 0xFF) {
                    // when mLen wraps around the transfer is over
                    mLen = 0x7F;
                    mMode = HdmaMode::Stopped;
                    sendEvt = true;
                    evt = BusEvent::HdmaStopped;
                    break;
                }
            }
        }
    }

    if (sendEvt)
        mBus->sendEvent(evt);
}

void Hdma::pauseOnCpuHalt()
{
    // as explained in https://gbdev.io/pandocs/CGB_Registers.html#bit-7--1--hblank-dma
    // when the cpu executes the halt instruction, an hblank transfer is paused
    // as well and will resume only when the cpu resumes from the halt state

    if (mIsCgb && mMode == HdmaMode::HBlank) {
        mPauseHblankOnHalt = true;
    }
}

void Hdma::resumeOnCpuHalt()
{
    if (mIsCgb && mMode == HdmaMode::HBlank) {
        mPauseHblankOnHalt = false;
    }
}



#include "Serial.h"
#include "Irqs.h"



Serial::Serial(Bus& bus)
    : mBus(&bus)
{
    reset();
}

void Serial::reset()
{
    mClockCounter = 0;
    mClockCounterTarget = 128;
    mShiftCounter = 0;

    mEnable = false;
    mClockSpeed = false;
    mClockIsMaster = false;

    mRegData = 0;

    mTransferredOut = 0;
}

void Serial::step(uint32_t mCycles)
{
  
    while (mCycles--) {

        if (++mClockCounter >= mClockCounterTarget) {
            mClockCounter = 0;

            // if the transfer is enabled and the internal clock is selected 
            // we transfer 1 bit out, the most significant bit is transferred first
            if (mEnable && mClockIsMaster) {
                bool bit = mRegData & 0x80;

                // the data reg is shifted left by 1
                mRegData <<= 1;

                mTransferredOut = (mTransferredOut << 1) | (bit ? 1 : 0);

                if (++mShiftCounter == 8) {
                    // when we transferred 8 bits we stop by setting the enable to false
                    // we also have to trigger the serial transfer interrupt
                    mEnable = false;

                    auto currIF = mBus->read8(mmap::regs::IF);
                    mBus->write8(mmap::regs::IF, Irqs::mask(Irqs::Type::Serial) | currIF);

                    if (mDataReadyCb)
                        mDataReadyCb(mTransferredOut);
                }
            }
        }
    }
}

void Serial::writeCtrl(uint8_t val)
{
    // bit 0 is clock select (master or slave)
    // bit 1 is clock speed
    // bits 2-6 are unused
    // bit 7 is transfer enable
    mClockIsMaster = val & 0x01;
    mClockSpeed = val & 0x02;
    mEnable = val & 0x80;

    // in the classic gameboy the internal clock used by the serial transfer is
    // fixed at 8192 HZ (~1KB/s), the external clock can be anything up to 500KHz
    // to divide the main ~1MHz clock down to 8192 we have to count up to 128
    // in the GBC it's possible to switch between multiple speeds:
    // https://gbdev.io/pandocs/Serial_Data_Transfer_(Link_Cable).html
    // when the clock speed bit is 1 the clock switches to 262144 HZ (count up to 4)
    mClockCounterTarget = mClockSpeed ? 4 : 128;

    // when the enable bit is set we reset the shift counter to 
    // be ready to transfer or receive a new byte
    if (mEnable && mClockIsMaster) {
        mShiftCounter = 0;
        mTransferredOut = 0;
    }
}

uint8_t Serial::readCtrl() const
{
    uint8_t val = 0xff;
    if (!mEnable)
        val &= ~0x80;
    if (!mClockSpeed)
        val &= ~0x02;
    if (!mClockIsMaster)
        val &= ~0x01;

    return val;
}


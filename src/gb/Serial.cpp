
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
    mShiftCounter = 0;

    mEnable = false;
    mClockSelect = false;

    mRegData = 0;

    mTransferredOut = 0;
}

void Serial::step(uint32_t mCycles)
{
    // in the classic gameboy the internal clock used by the serial transfer is
    // fixed at 8192 HZ (~1KB/s), the external clock can be anything up to 500KHz
    // to divide the main ~1MHz clock down to 8192 we have to count up to 128

    while (--mCycles) {

        if (++mClockCounter == 128) {
            mClockCounter = 0;

            // if the transfer is enabled and the internal clock is selected 
            // we transfer 1 bit out, the most significant bit is transferred first
            if (mEnable && mClockSelect) {
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
    // bit 0 is clock select
    // bits 1-6 are unused
    // bit 7 is transfer enable
    mClockSelect = val & 0x01;
    mEnable = val & 0x80;

    // when the enable bit is set we reset the shift counter to 
    // be ready to transfer or receive a new byte
    if (mEnable && mClockSelect) {
        mShiftCounter = 0;
        mTransferredOut = 0;
    }
}

uint8_t Serial::readCtrl() const
{
    uint8_t val = 0xff;
    if (!mEnable)
        val &= ~0x80;
    if(!mClockSelect)
        val &= ~0x01;

    return val;
}


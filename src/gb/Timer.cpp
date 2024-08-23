
#include "Timer.h"
#include "GbCommons.h"
#include "Irqs.h"


Timer::Timer(Bus& bus)
    : mBus(bus)
{
    reset();
}

// TODO: handle "Timer obscure behavior": https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html


void Timer::reset()
{
    // DIV starts from AC00 because before executing the actual cartridge code the 
    // gameboy executes its own boot rom that obviously takes a while to run
    // TAC unused bits are initially set to 1

    mDiv = initialDivVal << 8;
    mTima = 0;
    mTimaSubcounter = 0;
    mTma = 0;
    mTimaEnabled = false;
    mClockSelect = ClockSelect::N1024;
    mTacVal = 0xF8;
}

void Timer::step(uint32_t mCycles, bool isCpuStopped)
{
    // when the cpu is stopped the timer doesn't tick
    if (isCpuStopped)
        return;

    // the mCycles argument represents the number of machine cycles that 
    // have elapsed since the last step call
    // 1 machine cycle is equal to 4 clock cycles and DIV counts the number of clock cycles
    // so we have to increase it by 4
    uint16_t cCycles = (uint16_t)mCycles * 4;
    uint16_t subClock = clockDividers[mClockSelect];
    
    mDiv += cCycles;

    // if the timer is enabled we also have to increase the TIMA register and handle its overflows
    if (mTimaEnabled) {
        // increase the TIMA subcounter
        mTimaSubcounter += cCycles;

        // if necessary increase TIMA itself
        if (mTimaSubcounter >= subClock) {
            mTima += mTimaSubcounter / subClock;
            mTimaSubcounter = mTimaSubcounter % subClock;
        }

        // check if TIMA overflowed
        if (mTima > 0xFF) {
            // reset TIMA value to that of TMA
            mTima = mTma;

            auto currIF = mBus.read8(mmap::regs::IF);
            mBus.write8(mmap::regs::IF, Irqs::mask(Irqs::Type::Timer) | currIF);
        }
    }
}

void Timer::writeTAC(uint8_t val)
{
    mTacVal = val & 0xF8;
    auto newClockSelect = (ClockSelect)(val & 0x3);
    auto newTimaEnabled = val & 0x4;

    // the TIMA subcounter must be restarted when we enable
    // the timer (off to on transition)
    if (newTimaEnabled && !mTimaEnabled)
        mTimaSubcounter = 0;

    mClockSelect = newClockSelect;
    mTimaEnabled = newTimaEnabled;
}


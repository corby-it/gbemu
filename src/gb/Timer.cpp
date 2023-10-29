
#include "Timer.h"


Timer::Timer()
    : mDiv(0)
    , mTima(0)
    , mTimaSubcounter(0)
    , mTma(0)
    , mTimaEnabled(false)
    , mClockSelect(ClockSelect::N1024)
{}

// TODO: handle "Timer obscure behavior": https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html


void Timer::step(uint16_t mCycles)
{
    // the mCycles argument represents the number of machine cycles that 
    // have elapsed since the last step call
    // 1 machine cycle is equal to 4 clock cycles and DIV counts the number of clock cycles
    // so we have to increase it by 4
    uint16_t cCycles = mCycles * 4;
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

            // TODO: generate interrupt
        }
    }
}

uint8_t Timer::readTAC() const
{
    return mTimaEnabled << 2 | (uint8_t)mClockSelect;
}

void Timer::writeTAC(uint8_t val)
{
    auto newClockSelect = (ClockSelect)(val & 0x3);
    auto newTimaEnabled = val & 0x4;

    // the TIMA subcounter must be restarted when we enable
    // the timer (off to on transition)
    if (newTimaEnabled && !mTimaEnabled)
        mTimaSubcounter = 0;

    mClockSelect = newClockSelect;
    mTimaEnabled = newTimaEnabled;
}


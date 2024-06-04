
#include "Utils.h"

using namespace std::chrono;
using hr_clock = std::chrono::high_resolution_clock;


RTC::RTC()
{
    reset();
}


void RTC::reset()
{
    mSec = 0;
    mMin = 0;
    mHours = 0;
    mDaysL = 0;
    mDaysH = 0x40; // the 6th bit is the HALT flag, 1 if the counters are latched

    // reset the last latch timepoint to epoch
    mLastLatch = hr_clock::time_point();

    latch();
}

void RTC::latch()
{
    auto nowTp = hr_clock::now();

    auto elapsed = nowTp - mLastLatch;

    mSec = (mSec + duration_cast<seconds>(elapsed).count()) % 60;
    mMin = (mMin + duration_cast<minutes>(elapsed).count()) % 60;

    auto hoursVal = duration_cast<hours>(elapsed).count();
    mHours = (mHours + hoursVal) % 24;
    
    uint64_t oldDays = mDaysL | ((mDaysH & 1) << 8);
    uint64_t newDays = oldDays + (hoursVal / 24);

    mDaysL = newDays & 0xff;
    mDaysH &= 0xFE; // reset the least significant bit
    mDaysH |= (newDays >> 8) & 0x01; // set the least significant bit to the value of the 9th bit of the days

    // if there's something beyond the 9th bit set the days carry flag bit in the 7th bit of mDaysH
    if(newDays >> 9)
        mDaysH |= 0x80;
    else 
        mDaysH &= ~0x80;

    mLastLatch = nowTp;
}


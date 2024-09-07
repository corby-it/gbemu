

#ifndef GBEMU_SRC_GB_UTILS_H_
#define GBEMU_SRC_GB_UTILS_H_

#include <cstdint>
#include <chrono>
#include <cereal/types/chrono.hpp>


class RTC {
public:
    RTC();

    void reset();

    uint8_t readSec() const { return mSec; }
    uint8_t readMin() const { return mMin; }
    uint8_t readHours() const { return mHours; }
    uint8_t readDaysL() const { return mDaysL; }
    uint8_t readDaysH() const { return mDaysH; }

    void writeSec(uint8_t val) { mSec = val % 60; }
    void writeMin(uint8_t val) { mMin = val % 60; }
    void writeHours(uint8_t val) { mHours = val % 24; }
    void writeDaysL(uint8_t val) { mDaysL = val; }
    void writeDaysH(uint8_t val) { mDaysH = val & 0xC1; }

    void latch();


    template<class Archive>
    void serialize(Archive& ar) {
        ar(mLastLatch, mSec, mMin, mHours, mDaysL, mDaysH);
    }

private:

    std::chrono::high_resolution_clock::time_point mLastLatch;

    uint8_t mSec;
    uint8_t mMin;
    uint8_t mHours;
    uint8_t mDaysL;
    uint8_t mDaysH;

};


#endif // GBEMU_SRC_GB_UTILS_H_
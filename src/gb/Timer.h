
#ifndef GBEMU_SRC_GB_TIMER_H_
#define GBEMU_SRC_GB_TIMER_H_

#include "Bus.h"
#include <cstdint>
#include <cereal/cereal.hpp>


class Timer {
public:
    enum class ClockSelect : uint8_t {
        N1024 = 0,
        N16 = 1,
        N64 = 2,
        N256 = 3
    };

    static constexpr uint8_t TACTimerEnableMask = 0x04;


    Timer(Bus& bus);

    void reset();

    void step(uint32_t mCycles, bool isCpuStopped = false);

    uint8_t readDIV() const { return mDiv >> 8; }
    uint8_t readTIMA() const { return mTima & 0x00FF; }
    uint8_t readTMA() const { return mTma; }
    uint8_t readTAC() const { return mTacVal; }

    void writeDIV(uint8_t /*val*/) { mDiv = 0; }
    void writeTIMA(uint8_t val) { mTima = val; }
    void writeTMA(uint8_t val) { mTma = val; }
    void writeTAC(uint8_t val);


    // helpers
    void enableTimer(bool b) { mTimaEnabled = b; }
    void setSubclock(ClockSelect c) { mClockSelect = c; }

    uint16_t getTimaSubcounter() const { return mTimaSubcounter; }

    static constexpr uint8_t initialDivVal = 0xAC;

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mDiv, mTima, mTimaSubcounter, mTma, mTimaEnabled, mClockSelect, mTacVal);
    }


private:
    Bus* mBus;

    // REGISTERS

    // DIV - Divider register
    // can be thought of a 16 bit register that counts the basic clock frequency (4 MHz)
    // from the outside, only the upper 8 bits are accessible.
    // writing to the register resets its content to 0 and the counting restarts.
    // this register is also reset when the STOP instruction is executed.
    // DIV is always counting, even when the timer is not enabled!
    uint16_t mDiv;

    // TIMA - Timer Counter
    // when the timer is enabled this is the main timer counter.
    // it counts at the frequency specified by the TAC register (a divider of the main clock)
    // it generates an interrupt when it overflows (that is, becomes greater than 0xFF) and its
    // value is reset to the value of the TMA register
    uint16_t mTima;
    uint16_t mTimaSubcounter;

    // TMA - Timer Modulo
    // the value written to the TIMA register when it overflows.
    // the value written in TMA can be used to further divide the selected clock source, if TMA is FF
    // the TIMA register overflows every time its clock ticks, if TMA is FD the TIMA register overflows
    // after its clock ticks twice, and so on.
    uint8_t mTma;
    
    // TAC - Timer control register
    // The TAC register controls the operation of the timer:
    // bits 0-1: Clock select 
    // bit    2: Timer enable (if TIMA is actually incremented or not)
    // bits 3-7: unused
    bool mTimaEnabled;
    ClockSelect mClockSelect;
    uint8_t mTacVal;

    static constexpr uint16_t clockDividers[4] = { 1024, 16, 64, 256 };

};

CEREAL_CLASS_VERSION(Timer, 1);


#endif // GBEMU_SRC_GB_TIMER_H_
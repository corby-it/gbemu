
#ifndef GBEMU_SRC_GB_JOYPAD_H_
#define GBEMU_SRC_GB_JOYPAD_H_

#include "Bus.h"


// reference for how the joypad works in the gameboy: https://gbdev.io/pandocs/Joypad_Input.html


class Joypad {
public:
    enum class Btn {
        Start, Select, B, A,
        Down, Up, Left, Right
    };

    Joypad(Bus& bus);

    void reset();
    void step(uint32_t mCycles);

    // GB side
    void write(uint8_t val);
    uint8_t read() const;

    // UI side
    void press(Btn bt);
    void release(Btn bt);

private:
    enum class Selection : uint8_t {
        Both = 0x00,
        Buttons = 0x01,
        Dpad = 0x02,
        Disabled = 0x03
    };

    bool inCurrentSelection(Btn btn) const;

    Bus& mBus;

    Selection mSelection;

    uint8_t mDpadByte;
    uint8_t mBtnsByte;

    bool mCounterEnabled;
    uint32_t mCyclesCounter;

};



#endif // GBEMU_SRC_GB_JOYPAD_H_
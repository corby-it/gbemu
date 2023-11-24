
#ifndef GBEMU_SRC_GB_JOYPAD_H_
#define GBEMU_SRC_GB_JOYPAD_H_

#include <cstdint>


// reference: https://gbdev.io/pandocs/Joypad_Input.html


class Joypad {
public:
    enum class Btn {
        Start, Select, B, A,
        Down, Up, Left, Right
    };


    Joypad();

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

    Selection mSelection;

    uint8_t mDpadByte;
    uint8_t mBtnsByte;
};



#endif // GBEMU_SRC_GB_JOYPAD_H_

#ifndef GBEMU_SRC_GB_JOYPAD_H_
#define GBEMU_SRC_GB_JOYPAD_H_

#include "Bus.h"
#include <array>
#include <cereal/cereal.hpp>

// reference for how the joypad works in the gameboy: https://gbdev.io/pandocs/Joypad_Input.html


class Joypad : public ReadWriteIf {
public:
    enum class Btn {
        Start, Select, B, A,
        Down, Up, Left, Right
    };

    static constexpr size_t btnCount = 8;

    static constexpr Btn allBtns[] = {
        Btn::Start, Btn::Select, Btn::B, Btn::A,
        Btn::Down, Btn::Up, Btn::Left, Btn::Right
    };

    struct PressedButton {
        PressedButton()
            : pressed()
            , count(0)
        {}

        void add(Joypad::Btn btn) {
            if (count == pressed.max_size())
                return;

            pressed[count] = btn;
            ++count;
        }

        std::array<Joypad::Btn, Joypad::btnCount> pressed;
        size_t count;
    };


    Joypad(Bus& bus);

    void reset();
    void step(uint32_t mCycles);

    // GB side
    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

    // UI side
    void press(Btn bt);
    void release(Btn bt);

    void action(const PressedButton& pressedBtns);


    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mSelection, mDpadByte, mBtnsByte, mCounterEnabled, mCyclesCounter);
    }

private:
    enum class Selection : uint8_t {
        Both = 0x00,
        Buttons = 0x01,
        Dpad = 0x02,
        Disabled = 0x03
    };

    bool inCurrentSelection(Btn btn) const;

    Bus* mBus;

    Selection mSelection;

    uint8_t mDpadByte;
    uint8_t mBtnsByte;

    bool mCounterEnabled;
    uint32_t mCyclesCounter;

};

CEREAL_CLASS_VERSION(Joypad, 1);



#endif // GBEMU_SRC_GB_JOYPAD_H_
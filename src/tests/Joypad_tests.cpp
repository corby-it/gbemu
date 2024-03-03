

#include "gb/Joypad.h"
#include "gb/Bus.h"
#include "gb/Irqs.h"
#include "gb/GbCommons.h"
#include "doctest/doctest.h"



static const uint8_t dpadMask = 0x20;
static const uint8_t btnsMask = 0x10;
static const uint8_t disableMask = 0x30;



TEST_CASE("Joypad test writing disabled")
{
    TestBus bus;
    Joypad jp(bus);
    
    jp.write(disableMask);

    auto val = jp.read();
    CHECK(val == 0xCF);

    // press a few buttons
    jp.press(Joypad::Btn::A);
    jp.press(Joypad::Btn::Start);
    jp.press(Joypad::Btn::Down);

    val = jp.read();
    CHECK(val == 0xCF);
}

TEST_CASE("Joypad test writing enabled")
{
    TestBus bus;
    Joypad jp(bus);

    jp.write(disableMask);

    auto val = jp.read();
    CHECK(val == 0xCF);

    // press A and Start, we must read 0b11010110 (0xD6) when we
    // read the status of the buttons
    jp.press(Joypad::Btn::A);
    jp.press(Joypad::Btn::Start);

    // press Up and Left, we must read 0b11101001 (0xE9) when we
    // read the status of the dpad
    jp.press(Joypad::Btn::Up);
    jp.press(Joypad::Btn::Left);

    // enable buttons
    jp.write(btnsMask);

    val = jp.read();
    CHECK(val == 0xD6);

    // enable dpad
    jp.write(dpadMask);

    val = jp.read();
    CHECK(val == 0xE9);
}



TEST_CASE("Joypad test interrupt trigger")
{
    TestBus bus;
    Joypad jp(bus);

    constexpr auto jpIrqMask = Irqs::mask(Irqs::Type::Joypad);

    jp.write(dpadMask);

    auto val = jp.read();
    CHECK(val == 0xEF);

    // press a few buttons
    jp.press(Joypad::Btn::Down);

    val = jp.read();
    CHECK(val == 0xE7);

    // step for 2 cycles, the interrupt must still be disabled
    jp.step(2);

    CHECK(jpIrqMask != (bus.read8(mmap::regs::IF) & jpIrqMask));

    // step for 4 cycles, now the interrupts must be triggered
    jp.step(4);

    CHECK(jpIrqMask == (bus.read8(mmap::regs::IF) & jpIrqMask));
}


TEST_CASE("Joypad test interrupt counter reset")
{
    TestBus bus;
    Joypad jp(bus);

    constexpr auto jpIrqMask = Irqs::mask(Irqs::Type::Joypad);

    jp.write(dpadMask);

    auto val = jp.read();
    CHECK(val == 0xEF);

    // press a few buttons
    jp.press(Joypad::Btn::Down);

    val = jp.read();
    CHECK(val == 0xE7);

    // step for 2 cycles, the interrupt must still be disabled
    jp.step(2);

    CHECK(jpIrqMask != (bus.read8(mmap::regs::IF) & jpIrqMask));

    // release the button, step for 4 cycles, the interrutp must still be disabled
    jp.release(Joypad::Btn::Down);
    jp.step(4);

    CHECK(jpIrqMask != (bus.read8(mmap::regs::IF) & jpIrqMask));

    // press the button again and hold it down fro more than 4 cycles
    jp.press(Joypad::Btn::Down);
    jp.step(6);

    CHECK(jpIrqMask == (bus.read8(mmap::regs::IF) & jpIrqMask));
}
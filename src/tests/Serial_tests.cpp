
#include "gb/Serial.h"
#include "gb/GbCommons.h"
#include <doctest/doctest.h>


static const auto addrData = mmap::regs::serial_data;
static const auto addrCtrl = mmap::regs::serial_ctrl;


TEST_CASE("Serial tests")
{
    TestBus bus;
    Serial sr(bus);

    uint8_t transferred = 0;

    auto callback = [&transferred](uint8_t byte) {
        transferred = byte;
    };

    sr.setSerialDataReadyCb(callback);

    sr.write8(addrData, 0x42);

    uint32_t counterVal = 0;
    uint8_t ctrlLowNibbleMask = 0x00;

    SUBCASE("Slow speed (8192 Hz)") {
        // start transfer as master, slow clock speed (128 clock cycles to transfer 1 bit)
        counterVal = 128;
        sr.write8(addrCtrl, 0x81);
        ctrlLowNibbleMask = 0x0d;
    }
    SUBCASE("High speed (262144 Hz)") {
        // start transfer as master, high clock speed (4 clock cycles to transfer 1 bit)
        counterVal = 4;
        sr.write8(addrCtrl, 0x83);
        ctrlLowNibbleMask = 0x0f;
    }

    // transfer 1 bit
    sr.step(counterVal);

    // should still be enabled (unused bits are all set)
    CHECK(sr.read8(addrCtrl) == (0xf0 | ctrlLowNibbleMask));

    // transfer the remaining 7 bits
    sr.step(counterVal * 7);

    // it must be disabled now and the value should be in transferred
    CHECK(sr.read8(addrCtrl) == (0x70 | ctrlLowNibbleMask));
    CHECK(transferred == 0x42);
}

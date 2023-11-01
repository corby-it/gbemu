
#include "gb/Timer.h"
#include "gb/GbCommons.h"
#include "gb/Irqs.h"
#include "doctest/doctest.h"


TEST_CASE("Timer test DIV increase")
{
    TestBus bus;
    Timer t(bus);

    t.step(10);
    CHECK(t.readDIV() == 0); // should still be 0

    t.step(0x100);
    CHECK(t.readDIV() == ((0x100 + 10)*4) >> 8);

    t.writeDIV(100);
    CHECK(t.readDIV() == 0); // writing resets div
}

TEST_CASE("Timer test DIV overflow")
{
    TestBus bus;
    Timer t(bus);

    t.step(0x3FFF);
    CHECK(t.readDIV() == 0xFF);

    t.step(1);
    CHECK(t.readDIV() == 0);
}

TEST_CASE("Timer test TIMA increase")
{
    TestBus bus;
    Timer t(bus);

    SUBCASE("Increase with ClockSelect::N16") {
        t.writeTAC(Timer::TACTimerEnableMask | Timer::ClockSelect::N16);

        t.step(4);
        CHECK(t.readTIMA() == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(6);
        CHECK(t.readTIMA() == 2);
        CHECK(t.getTimaSubcounter() == 8);
    }

    SUBCASE("Increase with ClockSelect::N64") {
        t.writeTAC(Timer::TACTimerEnableMask | Timer::ClockSelect::N64);

        t.step(16);
        CHECK(t.readTIMA() == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(24);
        CHECK(t.readTIMA() == 2);
        CHECK(t.getTimaSubcounter() == 32);
    }

    SUBCASE("Increase with ClockSelect::N256") {
        t.writeTAC(Timer::TACTimerEnableMask | Timer::ClockSelect::N256);

        t.step(64);
        CHECK(t.readTIMA() == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(96);
        CHECK(t.readTIMA() == 2);
        CHECK(t.getTimaSubcounter() == 128);
    }

    SUBCASE("Increase with ClockSelect::N1024") {
        t.writeTAC(Timer::TACTimerEnableMask | Timer::ClockSelect::N1024);

        t.step(256);
        CHECK(t.readTIMA() == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(384);
        CHECK(t.readTIMA() == 2);
        CHECK(t.getTimaSubcounter() == 512);
    }
}

TEST_CASE("Timer test TIMA overflow and interrupt generation")
{
    TestBus bus;
    Timer t(bus);
    
    auto timerIrqMask = Irqs::mask(Irqs::Type::Timer);

    t.writeTAC(Timer::TACTimerEnableMask | Timer::ClockSelect::N16);
    
    t.writeTIMA(0xFE);
    // TIMA start from FE so we need 16 + 16 clock cycles (4 + 4 machine cycles) to overflow

    SUBCASE("TIMA overflow with TMA == 0") {
        // the value of TIMA after the overflow should be 0 in this case since TMA is still 0
        t.step(4);
        CHECK(t.readTIMA() == 0xFF);
        t.step(4);
        CHECK(t.readTIMA() == 0x00);
        CHECK(timerIrqMask == (bus.read8(mmap::regs::IF) & timerIrqMask));
    }

    SUBCASE("TIMA overflow with TMA == 0xFE") {
        // the value of TIMA after the overflow should be 0xDD in this case
        t.writeTMA(0xDD);

        t.step(4);
        CHECK(t.readTIMA() == 0xFF);
        t.step(4);
        CHECK(t.readTIMA() == 0xDD);
        CHECK(timerIrqMask == (bus.read8(mmap::regs::IF) & timerIrqMask));
    }
}


TEST_CASE("Timer test changing subclock while timer is running")
{
    TestBus bus;
    Timer t(bus);

    t.writeTAC(Timer::TACTimerEnableMask | Timer::ClockSelect::N16);

    // 4 machine cycles = 16 clock cycles --> TIMA must become 1 (and subcounter 0)
    t.step(4);
    CHECK(t.readTIMA() == 0x1);
    CHECK(t.getTimaSubcounter() == 0);
    // 2 machine cycles = 8 clock cycles --> TIMA must still be 1 (and subcounter 8)
    t.step(2);
    CHECK(t.readTIMA() == 1);
    CHECK(t.getTimaSubcounter() == 8);

    // change subclock to N64
    t.writeTAC(Timer::TACTimerEnableMask | Timer::ClockSelect::N64);

    // 14 machine cycles = 56 clock cycles, TIMA must become 2 (and subcounter 0)
    t.step(14);
    CHECK(t.readTIMA() == 2);
    CHECK(t.getTimaSubcounter() == 0);
}


TEST_CASE("Timer test starting, stopping and restarting the timer")
{
    TestBus bus;
    Timer t(bus);

    t.writeTAC(Timer::TACTimerEnableMask | Timer::ClockSelect::N16);

    // 2 machine cycles = 8 clock cycles --> TIMA must still be 0 (and subcounter 8)
    t.step(2);
    CHECK(t.readTIMA() == 0);
    CHECK(t.getTimaSubcounter() == 8);

    // disable timer
    t.writeTAC(Timer::ClockSelect::N16);
    
    // 4 machine cycles = 16 clock cycles but timer disabled --> TIMA must still be 0 (and subcounter 8)
    t.step(4);
    CHECK(t.readTIMA() == 0);
    CHECK(t.getTimaSubcounter() == 8);

    // re-enable timer 
    t.writeTAC(Timer::TACTimerEnableMask | Timer::ClockSelect::N16);

    // 4 machine cycles = 16 clock cycles --> TIMA must become 1 (and subcounter 0 because it resets on timer enable)
    t.step(4);
    CHECK(t.readTIMA() == 1);
    CHECK(t.getTimaSubcounter() == 0);
}

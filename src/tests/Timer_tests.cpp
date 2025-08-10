
#include "gb/Timer.h"
#include "gb/GbCommons.h"
#include "gb/Irqs.h"
#include "doctest/doctest.h"


static constexpr auto DIV = mmap::regs::timer::DIV;
static constexpr auto TIMA = mmap::regs::timer::TIMA;
static constexpr auto TMA = mmap::regs::timer::TMA;
static constexpr auto TAC = mmap::regs::timer::TAC;



TEST_CASE("Timer test DIV increase")
{
    TestBus bus;
    Timer t(bus);

    t.step(10);
    CHECK(t.read8(DIV) == Timer::initialDivVal); // should still be the initial value

    t.step(0x100);
    CHECK(t.read8(DIV) == Timer::initialDivVal + (((0x100 + 10)*4) >> 8));

    t.write8(DIV, 100);
    CHECK(t.read8(DIV) == 0); // writing resets div to 0
}

TEST_CASE("Timer test DIV overflow")
{
    TestBus bus;
    Timer t(bus);

    // DIV starts from AC00 so, to get to read FF and overflow we have to
    // start from (FFFF-AC00) / 4 = 14FF

    uint32_t steps = (0xFFFF - (Timer::initialDivVal << 8)) / 4;

    t.step(steps);
    CHECK(t.read8(DIV) == 0xFF);

    t.step(1);
    CHECK(t.read8(DIV) == 0);
}

TEST_CASE("Timer test TIMA increase")
{
    TestBus bus;
    Timer t(bus);

    SUBCASE("Increase with ClockSelect::N16") {
        t.write8(TAC, Timer::TACTimerEnableMask | (uint8_t)Timer::ClockSelect::N16);

        t.step(4);
        CHECK(t.read8(TIMA) == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(6);
        CHECK(t.read8(TIMA) == 2);
        CHECK(t.getTimaSubcounter() == 8);
    }

    SUBCASE("Increase with ClockSelect::N64") {
        t.write8(TAC, Timer::TACTimerEnableMask | (uint8_t)Timer::ClockSelect::N64);

        t.step(16);
        CHECK(t.read8(TIMA) == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(24);
        CHECK(t.read8(TIMA) == 2);
        CHECK(t.getTimaSubcounter() == 32);
    }

    SUBCASE("Increase with ClockSelect::N256") {
        t.write8(TAC, Timer::TACTimerEnableMask | (uint8_t)Timer::ClockSelect::N256);

        t.step(64);
        CHECK(t.read8(TIMA) == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(96);
        CHECK(t.read8(TIMA) == 2);
        CHECK(t.getTimaSubcounter() == 128);
    }

    SUBCASE("Increase with ClockSelect::N1024") {
        t.write8(TAC, Timer::TACTimerEnableMask | (uint8_t)Timer::ClockSelect::N1024);

        t.step(256);
        CHECK(t.read8(TIMA) == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(384);
        CHECK(t.read8(TIMA) == 2);
        CHECK(t.getTimaSubcounter() == 512);
    }
}

TEST_CASE("Timer test TIMA overflow and interrupt generation")
{
    TestBus bus;
    Timer t(bus);
    
    auto timerIrqMask = Irqs::mask(Irqs::Type::Timer);

    t.write8(TAC, Timer::TACTimerEnableMask | (uint8_t)Timer::ClockSelect::N16);
    
    t.write8(TIMA, 0xFE);
    // TIMA start from FE so we need 16 + 16 clock cycles (4 + 4 machine cycles) to overflow

    SUBCASE("TIMA overflow with TMA == 0") {
        // the value of TIMA after the overflow should be 0 in this case since TMA is still 0
        t.step(4);
        CHECK(t.read8(TIMA) == 0xFF);
        t.step(4);
        CHECK(t.read8(TIMA) == 0x00);
        CHECK(timerIrqMask == (bus.read8(mmap::regs::IF) & timerIrqMask));
    }

    SUBCASE("TIMA overflow with TMA == 0xFE") {
        // the value of TIMA after the overflow should be 0xDD in this case
        t.write8(TMA, 0xDD);

        t.step(4);
        CHECK(t.read8(TIMA) == 0xFF);
        t.step(4);
        CHECK(t.read8(TIMA) == 0xDD);
        CHECK(timerIrqMask == (bus.read8(mmap::regs::IF) & timerIrqMask));
    }
}


TEST_CASE("Timer test changing subclock while timer is running")
{
    TestBus bus;
    Timer t(bus);

    t.write8(TAC, Timer::TACTimerEnableMask | (uint8_t)Timer::ClockSelect::N16);

    // 4 machine cycles = 16 clock cycles --> TIMA must become 1 (and subcounter 0)
    t.step(4);
    CHECK(t.read8(TIMA) == 0x1);
    CHECK(t.getTimaSubcounter() == 0);
    // 2 machine cycles = 8 clock cycles --> TIMA must still be 1 (and subcounter 8)
    t.step(2);
    CHECK(t.read8(TIMA) == 1);
    CHECK(t.getTimaSubcounter() == 8);

    // change subclock to N64
    t.write8(TAC, Timer::TACTimerEnableMask | (uint8_t)Timer::ClockSelect::N64);

    // 14 machine cycles = 56 clock cycles, TIMA must become 2 (and subcounter 0)
    t.step(14);
    CHECK(t.read8(TIMA) == 2);
    CHECK(t.getTimaSubcounter() == 0);
}


TEST_CASE("Timer test starting, stopping and restarting the timer")
{
    TestBus bus;
    Timer t(bus);

    t.write8(TAC, Timer::TACTimerEnableMask | (uint8_t)Timer::ClockSelect::N16);

    // 2 machine cycles = 8 clock cycles --> TIMA must still be 0 (and subcounter 8)
    t.step(2);
    CHECK(t.read8(TIMA) == 0);
    CHECK(t.getTimaSubcounter() == 8);

    // disable timer
    t.write8(TAC, (uint8_t)Timer::ClockSelect::N16);
    
    // 4 machine cycles = 16 clock cycles but timer disabled --> TIMA must still be 0 (and subcounter 8)
    t.step(4);
    CHECK(t.read8(TIMA) == 0);
    CHECK(t.getTimaSubcounter() == 8);

    // re-enable timer 
    t.write8(TAC, Timer::TACTimerEnableMask | (uint8_t)Timer::ClockSelect::N16);

    // 4 machine cycles = 16 clock cycles --> TIMA must become 1 (and subcounter 0 because it resets on timer enable)
    t.step(4);
    CHECK(t.read8(TIMA) == 1);
    CHECK(t.getTimaSubcounter() == 0);
}

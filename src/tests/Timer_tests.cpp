
#include "gb/Timer.h"
#include "doctest/doctest.h"


TEST_CASE("Timer test DIV increase")
{
    Timer t;

    t.step(10);
    CHECK(t.readDIV() == 0); // should still be 0

    t.step(0x100);
    CHECK(t.readDIV() == ((0x100 + 10)*4) >> 8);

    t.writeDIV(100);
    CHECK(t.readDIV() == 0); // writing resets div
}

TEST_CASE("Timer test DIV overflow")
{
    Timer t;

    t.step(0x3FFF);
    CHECK(t.readDIV() == 0xFF);

    t.step(1);
    CHECK(t.readDIV() == 0);
}

TEST_CASE("Timer test TIMA increase")
{
    Timer t;

    SUBCASE("Increase with ClockSelect::N16") {
        t.setSubclock(Timer::ClockSelect::N16);
        t.enableTimer(true);

        t.step(4);
        CHECK(t.readTIMA() == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(6);
        CHECK(t.readTIMA() == 2);
        CHECK(t.getTimaSubcounter() == 8);
    }

    SUBCASE("Increase with ClockSelect::N64") {
        t.setSubclock(Timer::ClockSelect::N64);
        t.enableTimer(true);

        t.step(16);
        CHECK(t.readTIMA() == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(24);
        CHECK(t.readTIMA() == 2);
        CHECK(t.getTimaSubcounter() == 32);
    }

    SUBCASE("Increase with ClockSelect::N256") {
        t.setSubclock(Timer::ClockSelect::N256);
        t.enableTimer(true);

        t.step(64);
        CHECK(t.readTIMA() == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(96);
        CHECK(t.readTIMA() == 2);
        CHECK(t.getTimaSubcounter() == 128);
    }

    SUBCASE("Increase with ClockSelect::N1024") {
        t.setSubclock(Timer::ClockSelect::N1024);
        t.enableTimer(true);

        t.step(256);
        CHECK(t.readTIMA() == 1);
        CHECK(t.getTimaSubcounter() == 0);

        t.step(384);
        CHECK(t.readTIMA() == 2);
        CHECK(t.getTimaSubcounter() == 512);
    }
}

TEST_CASE("Timer test TIMA overflow")
{
    Timer t;

    t.enableTimer(true);
    t.setSubclock(Timer::ClockSelect::N16);
    
    t.writeTIMA(0xFE);
    // TIMA start from FE so we need 16 + 16 clock cycles (4 + 4 machine cycles) to overflow

    SUBCASE("TIMA overflow with TMA == 0") {
        // the value of TIMA after the overflow should be 0 in this case since TMA is still 0
        t.step(4);
        CHECK(t.readTIMA() == 0xFF);
        t.step(4);
        CHECK(t.readTIMA() == 0x00);
    }

    SUBCASE("TIMA overflow with TMA == 0xFE") {
        // the value of TIMA after the overflow should be 0xDD in this case
        t.writeTMA(0xDD);

        t.step(4);
        CHECK(t.readTIMA() == 0xFF);
        t.step(4);
        CHECK(t.readTIMA() == 0xDD);
    }
}

// TODO: test what happens when the selected clock is changed 
// TODO: test what happens when the timer is started -> stopped -> restarted
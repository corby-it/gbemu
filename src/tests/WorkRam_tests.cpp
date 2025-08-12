
#include "gb/WorkRam.h"
#include "doctest/doctest.h"

static constexpr uint16_t offset = 0x100;

static constexpr uint16_t memAddrBank0 = mmap::wram::start + (WorkRam::bankSize * 0) + offset;
static constexpr uint16_t memAddrBank1 = mmap::wram::start + (WorkRam::bankSize * 1) + offset;

static constexpr uint16_t echomemAddrBank0 = mmap::echoram::start + (WorkRam::bankSize * 0) + offset;
static constexpr uint16_t echomemAddrBank1 = mmap::echoram::start + (WorkRam::bankSize * 1) + offset;

static constexpr uint16_t ramAddrBank0 = (WorkRam::bankSize * 0) + offset;
static constexpr uint16_t ramAddrBank1 = (WorkRam::bankSize * 1) + offset;
static constexpr uint16_t ramAddrBank2 = (WorkRam::bankSize * 2) + offset;
static constexpr uint16_t ramAddrBank3 = (WorkRam::bankSize * 3) + offset;
static constexpr uint16_t ramAddrBank4 = (WorkRam::bankSize * 4) + offset;
static constexpr uint16_t ramAddrBank5 = (WorkRam::bankSize * 5) + offset;
static constexpr uint16_t ramAddrBank6 = (WorkRam::bankSize * 6) + offset;
static constexpr uint16_t ramAddrBank7 = (WorkRam::bankSize * 7) + offset;

static constexpr uint16_t SVBK = mmap::regs::svbk;


TEST_CASE("WorkRam - Check DMG functionality")
{
    WorkRam wram;
    
    wram.setIsCgb(false);

    // try writing to all banks
    wram.write8(memAddrBank0, 123);

    for (uint8_t i = 1; i < 8; ++i) {
        wram.write8(SVBK, i);
        wram.write8(memAddrBank1, i);
    }

    // read raw values through RAM ptr
    // banks 2 - 7 values must still be zeros
    CHECK(wram.Ram::read8(ramAddrBank0) == 123);
    CHECK(wram.Ram::read8(ramAddrBank1) == 7);
    CHECK(wram.Ram::read8(ramAddrBank2) == 0);
    CHECK(wram.Ram::read8(ramAddrBank3) == 0);
    CHECK(wram.Ram::read8(ramAddrBank4) == 0);
    CHECK(wram.Ram::read8(ramAddrBank5) == 0);
    CHECK(wram.Ram::read8(ramAddrBank6) == 0);
    CHECK(wram.Ram::read8(ramAddrBank7) == 0);

    // check echo ram access
    CHECK(wram.read8(echomemAddrBank0) == 123);
    CHECK(wram.read8(echomemAddrBank1) == 7);
}

TEST_CASE("WorkRam - Check CGB functionality")
{
    WorkRam wram;
    
    wram.setIsCgb(true);

    // try writing to all banks
    wram.write8(memAddrBank0, 123);

    for (uint8_t i = 1; i < 8; ++i) {
        wram.write8(SVBK, i);
        wram.write8(memAddrBank1, i);
    }

    // read raw values through RAM ptr
    // banks 2 - 7 values must still be zeros
    CHECK(wram.Ram::read8(ramAddrBank0) == 123);
    CHECK(wram.Ram::read8(ramAddrBank1) == 1);
    CHECK(wram.Ram::read8(ramAddrBank2) == 2);
    CHECK(wram.Ram::read8(ramAddrBank3) == 3);
    CHECK(wram.Ram::read8(ramAddrBank4) == 4);
    CHECK(wram.Ram::read8(ramAddrBank5) == 5);
    CHECK(wram.Ram::read8(ramAddrBank6) == 6);
    CHECK(wram.Ram::read8(ramAddrBank7) == 7);

    // check echo ram access
    CHECK(wram.read8(echomemAddrBank0) == 123);
    CHECK(wram.read8(echomemAddrBank1) == 7);
}
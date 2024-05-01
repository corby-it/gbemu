

#include "gb/Mbc.h"
#include "gb/GbCommons.h"
#include <doctest/doctest.h>

static constexpr uint32_t romBankAddr(uint32_t num)
{
    return num * MbcInterface::romBankSize;
}

static constexpr uint32_t ramBankAddr(uint32_t num)
{
    return num * MbcInterface::ramBankSize;
}


TEST_CASE("MbcNone test")
{
    std::vector<uint8_t> rom(32 * 1024, 0);
    std::vector<uint8_t> ram;

    // write some values in specific locations
    rom[0x0000] = 1;
    rom[0x3FA4] = 2;
    rom[0x4DC1] = 3;
    rom[0x544F] = 4;
    rom[0x7FFF] = 5;

    MbcNone mbc(rom, ram);

    // try reading the values back
    CHECK(mbc.read8(0x0000) == 1);
    CHECK(mbc.read8(0x3FA4) == 2);
    CHECK(mbc.read8(0x4DC1) == 3);
    CHECK(mbc.read8(0x544F) == 4);
    CHECK(mbc.read8(0x7FFF) == 5);

    // try writing something and verify that there is no effect
    mbc.write8(0x0000, 50);
    mbc.write8(0x3FA4, 50);

    CHECK(mbc.read8(0x0000) == 1);
    CHECK(mbc.read8(0x3FA4) == 2);

    // rom and ram banks must be 0
    CHECK(mbc.getRomBankId() == 0);
    CHECK(mbc.getRamBankId() == 0);

    // try reading ram values (always FF)
    CHECK(mbc.read8(mmap::external_ram::start + 0x20) == 0xFF);
    CHECK(mbc.read8(mmap::external_ram::start + 0x120) == 0xFF);
    CHECK(mbc.read8(mmap::external_ram::start + 0x220) == 0xFF);
    CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 0xFF);
}




TEST_CASE("Mbc1 test - 64K rom (4 banks) no ram")
{
    std::vector<uint8_t> rom(64 * 1024, 0);
    std::vector<uint8_t> ram;

    // write some values in specific locations
    rom[romBankAddr(0) + 0x0000] = 1;   // 0x0000
    rom[romBankAddr(0) + 0x3FA4] = 2;   // 0x3FA4
    rom[romBankAddr(1) + 0x0DC1] = 3;   // 0x4DC1
    rom[romBankAddr(1) + 0x144F] = 4;   // 0x544F
    rom[romBankAddr(1) + 0x3FFF] = 5;   // 0x7FFF
    rom[romBankAddr(2) + 0x0DC1] = 6;   // 0x4DC1
    rom[romBankAddr(2) + 0x144F] = 7;   // 0x544F
    rom[romBankAddr(3) + 0x0DC1] = 8;   // 0x4DC1
    rom[romBankAddr(3) + 0x144F] = 9;   // 0x544F
    rom[romBankAddr(3) + 0x3FFF] = 10;  // 0x7FFF

    Mbc1 mbc(rom, ram, false, false);

    // reading from ram should return FF
    CHECK(mbc.read8(mmap::external_ram::start + 0x20) == 0xFF);
    CHECK(mbc.read8(mmap::external_ram::start + 0x120) == 0xFF);
    CHECK(mbc.read8(mmap::external_ram::start + 0x220) == 0xFF);
    CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 0xFF);

    // read from banks 0 and 1
    CHECK(mbc.read8(0x0000) == 1);
    CHECK(mbc.read8(0x3FA4) == 2);
    CHECK(mbc.read8(0x4DC1) == 3);
    CHECK(mbc.read8(0x544F) == 4);
    CHECK(mbc.read8(0x7FFF) == 5);

    // write 0 to the rom bank low register and check that nothing has changed
    mbc.write8(0x2001, 0);
    CHECK(mbc.read8(0x0000) == 1);
    CHECK(mbc.read8(0x3FA4) == 2);
    CHECK(mbc.read8(0x4DC1) == 3);
    CHECK(mbc.read8(0x544F) == 4);
    CHECK(mbc.read8(0x7FFF) == 5);

    // switch to bank 2 
    mbc.write8(0x2001, 2);
    CHECK(mbc.read8(0x0000) == 1);
    CHECK(mbc.read8(0x3FA4) == 2);
    CHECK(mbc.read8(0x4DC1) == 6);
    CHECK(mbc.read8(0x544F) == 7);

    // switch to bank 3
    mbc.write8(0x2001, 3);
    CHECK(mbc.read8(0x0000) == 1);
    CHECK(mbc.read8(0x3FA4) == 2);
    CHECK(mbc.read8(0x4DC1) == 8);
    CHECK(mbc.read8(0x544F) == 9);
    CHECK(mbc.read8(0x7FFF) == 10);

    // switch to addressing mode 1, nothing should change in this case 
    // since we have so little rom
    mbc.write8(0x6001, 1);
    CHECK(mbc.read8(0x0000) == 1);
    CHECK(mbc.read8(0x3FA4) == 2);
    CHECK(mbc.read8(0x4DC1) == 8);
    CHECK(mbc.read8(0x544F) == 9);
    CHECK(mbc.read8(0x7FFF) == 10);

}
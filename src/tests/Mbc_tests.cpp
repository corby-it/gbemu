

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

    Mbc1 mbc(rom, ram, false);

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
    // since we don't have enough rom
    mbc.write8(0x6001, 1);
    CHECK(mbc.read8(0x0000) == 1);
    CHECK(mbc.read8(0x3FA4) == 2);
    CHECK(mbc.read8(0x4DC1) == 8);
    CHECK(mbc.read8(0x544F) == 9);
    CHECK(mbc.read8(0x7FFF) == 10);

}


TEST_CASE("Mbc1 test - 512K rom (32 banks), 8K ram (1 bank)")
{
    std::vector<uint8_t> rom(512 * 1024, 0);
    std::vector<uint8_t> ram(8 * 1024, 0);

    // fill each bank with the bank number
    for(uint32_t i = 0; i < rom.size() / MbcInterface::romBankSize; ++i) {
        for (uint32_t n = 0; n < MbcInterface::romBankSize; ++n) {
            rom[romBankAddr(i) + n] = (uint8_t)i;
        }
    }

    Mbc1 mbc(rom, ram, true);

    SUBCASE("Check RAM") {
        // reading from ram should return FF if not enabled
        CHECK(mbc.read8(mmap::external_ram::start + 0x0020) == 0xFF);
        CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 0xFF);

        // enable ram
        mbc.write8(0x0001, 0x1A);

        // reading must return 0 now
        CHECK(mbc.read8(mmap::external_ram::start + 0x0020) == 0);
        CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 0);

        // write something to ram
        mbc.write8(mmap::external_ram::start + 0x0020, 13);
        mbc.write8(mmap::external_ram::start + 0x1300, 44);

        // read it back
        CHECK(mbc.read8(mmap::external_ram::start + 0x0020) == 13);
        CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 44);

        // disable ram and try reading again, should return 0xff now
        mbc.write8(0x0001, 0x10);
        CHECK(mbc.read8(mmap::external_ram::start + 0x0020) == 0xFF);
        CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 0xFF);

        // re-enable ram, we should read the previous values now
        mbc.write8(0x0001, 0x1A);
        CHECK(mbc.read8(mmap::external_ram::start + 0x0020) == 13);
        CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 44);
    }

    SUBCASE("Check ROM") {
        // read from banks 0 and 1
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 1);
        CHECK(mbc.read8(0x544F) == 1);

        // write 0 to the rom bank low register and check that nothing has changed
        mbc.write8(0x2001, 0);
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 1);
        CHECK(mbc.read8(0x544F) == 1);

        // switch to bank 2 
        mbc.write8(0x2001, 2);
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 2);
        CHECK(mbc.read8(0x544F) == 2);

        // switch to bank 3
        mbc.write8(0x2001, 3);
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 3);
        CHECK(mbc.read8(0x544F) == 3);

        // switch to bank 12
        mbc.write8(0x2001, 12);
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 12);
        CHECK(mbc.read8(0x544F) == 12);

        // switch to bank 31
        mbc.write8(0x2001, 31);
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 31);
        CHECK(mbc.read8(0x544F) == 31);

        // switch to addressing mode 1, nothing should change in this case 
        // since we don't have enough rom
        mbc.write8(0x6001, 1);
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 31);
        CHECK(mbc.read8(0x544F) == 31);


        // switch back to addressing mode 0 and try to access rom bank 38 (0x26)
        // considering that we only have 32 banks (5 bits), the bank number will be masked
        // with 0x1F, so 0x26 & 0x1F = 0x06
        mbc.write8(0x6001, 0);
        mbc.write8(0x2001, 38);
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 6);
        CHECK(mbc.read8(0x544F) == 6);
    }
}


TEST_CASE("Mbc1 test - 2M rom (128 banks), 32K ram (4 banks)")
{
    std::vector<uint8_t> rom(2 * 1024 * 1024, 0);
    std::vector<uint8_t> ram(32 * 1024, 0);

    // fill each bank with the bank number
    for(uint32_t i = 0; i < rom.size() / MbcInterface::romBankSize; ++i) {
        for (uint32_t n = 0; n < MbcInterface::romBankSize; ++n) {
            rom[romBankAddr(i) + n] = (uint8_t)i;
        }
    }

    Mbc1 mbc(rom, ram, true);

    SUBCASE("Check RAM") {
        // reading from ram should return FF if not enabled
        CHECK(mbc.read8(mmap::external_ram::start + 0x0020) == 0xFF);
        CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 0xFF);

        // enable ram and switch to addressing mode 1, otherwise banks >0 
        // are not addressable
        mbc.write8(0x0001, 0x1A);
        mbc.write8(0x6001, 1);

        // write something to bank 0
        mbc.write8(mmap::external_ram::start + 0x0020, 11);
        mbc.write8(mmap::external_ram::start + 0x1300, 22);

        // switch to bank 1 and write something
        mbc.write8(0x4001, 1);
        mbc.write8(mmap::external_ram::start + 0x0020, 33);
        mbc.write8(mmap::external_ram::start + 0x1300, 44);

        // switch to bank 2 and write something
        mbc.write8(0x4001, 2);
        mbc.write8(mmap::external_ram::start + 0x0020, 55);
        mbc.write8(mmap::external_ram::start + 0x1300, 66);

        // switch to bank 3 and write something
        mbc.write8(0x4001, 3);
        mbc.write8(mmap::external_ram::start + 0x0020, 77);
        mbc.write8(mmap::external_ram::start + 0x1300, 88);


        // check if all values are read back
        mbc.write8(0x4001, 0);
        CHECK(mbc.read8(mmap::external_ram::start + 0x0020) == 11);
        CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 22);
        mbc.write8(0x4001, 1);
        CHECK(mbc.read8(mmap::external_ram::start + 0x0020) == 33);
        CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 44);
        mbc.write8(0x4001, 2);
        CHECK(mbc.read8(mmap::external_ram::start + 0x0020) == 55);
        CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 66);
        mbc.write8(0x4001, 3);
        CHECK(mbc.read8(mmap::external_ram::start + 0x0020) == 77);
        CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 88);

        // switch back to addressing mode 0 and verifies that only bank 0 is accessible
        mbc.write8(0x6001, 0);
        CHECK(mbc.read8(mmap::external_ram::start + 0x0020) == 11);
        CHECK(mbc.read8(mmap::external_ram::start + 0x1300) == 22);
    }

    SUBCASE("Check ROM") {
        // read from banks 0 and 1
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 1);
        CHECK(mbc.read8(0x544F) == 1);

        // write 0 to the rom bank low register and check that nothing has changed
        mbc.write8(0x2001, 0);
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 1);
        CHECK(mbc.read8(0x544F) == 1);

        // switch to bank 2 
        mbc.write8(0x2001, 2);
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 2);
        CHECK(mbc.read8(0x544F) == 2);

        // switch to bank 12
        mbc.write8(0x2001, 12);
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 12);
        CHECK(mbc.read8(0x544F) == 12);

        // switch to bank 31
        mbc.write8(0x2001, 31);
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 31);
        CHECK(mbc.read8(0x544F) == 31);

        // try to switch to bank 32 (0x20) and check that we actually read from 33
        mbc.write8(0x4001, 1); // bits 5..6
        mbc.write8(0x2001, 0); // bits 0..4
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 33);
        CHECK(mbc.read8(0x544F) == 33);

        // switch to bank 0x43
        mbc.write8(0x4001, 0x02); // bits 5..6
        mbc.write8(0x2001, 0x03); // bits 0..4
        CHECK(mbc.read8(0x0000) == 0);
        CHECK(mbc.read8(0x3FA4) == 0);
        CHECK(mbc.read8(0x4DC1) == 0x43);
        CHECK(mbc.read8(0x544F) == 0x43);


        // switch to addressing mode 1 and check that bank 0x20 and 0x40 are 
        // accessible in the first rom bank addresses
        // the bank mapped to the second section is determined as in addressing mode 0
        mbc.write8(0x6001, 1);
        mbc.write8(0x4001, 0x01); // bits 5..6
        mbc.write8(0x2001, 0x00); // bits 0..4
        CHECK(mbc.read8(0x0000) == 0x20);
        CHECK(mbc.read8(0x3FA4) == 0x20);
        CHECK(mbc.read8(0x4DC1) == 0x21);
        CHECK(mbc.read8(0x544F) == 0x21);

        mbc.write8(0x4001, 0x02); // bits 5..6
        mbc.write8(0x2001, 0x00); // bits 0..4
        CHECK(mbc.read8(0x0000) == 0x40);
        CHECK(mbc.read8(0x3FA4) == 0x40);
        CHECK(mbc.read8(0x4DC1) == 0x41);
        CHECK(mbc.read8(0x544F) == 0x41);
    }
}
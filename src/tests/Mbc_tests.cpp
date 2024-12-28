

#include "gb/Mbc.h"
#include "gb/GbCommons.h"
#include "gb/Rtc.h"
#include <thread>
#include <chrono>
#include <doctest/doctest.h>
#include <iostream>

using namespace std::chrono;
using namespace std::chrono_literals;



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
    MbcNone mbc(32_KB, 0);

    // write some values in specific locations
    mbc.rom[0x0000] = 1;
    mbc.rom[0x3FA4] = 2;
    mbc.rom[0x4DC1] = 3;
    mbc.rom[0x544F] = 4;
    mbc.rom[0x7FFF] = 5;


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
    Mbc1 mbc(64_KB, 0);

    // write some values in specific locations
    mbc.rom[romBankAddr(0) + 0x0000] = 1;   // 0x0000
    mbc.rom[romBankAddr(0) + 0x3FA4] = 2;   // 0x3FA4
    mbc.rom[romBankAddr(1) + 0x0DC1] = 3;   // 0x4DC1
    mbc.rom[romBankAddr(1) + 0x144F] = 4;   // 0x544F
    mbc.rom[romBankAddr(1) + 0x3FFF] = 5;   // 0x7FFF
    mbc.rom[romBankAddr(2) + 0x0DC1] = 6;   // 0x4DC1
    mbc.rom[romBankAddr(2) + 0x144F] = 7;   // 0x544F
    mbc.rom[romBankAddr(3) + 0x0DC1] = 8;   // 0x4DC1
    mbc.rom[romBankAddr(3) + 0x144F] = 9;   // 0x544F
    mbc.rom[romBankAddr(3) + 0x3FFF] = 10;  // 0x7FFF


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
    Mbc1 mbc(512_KB, 8_KB);
    
    // fill each bank with the bank number
    for(uint32_t i = 0; i < mbc.rom.size() / MbcInterface::romBankSize; ++i) {
        for (uint32_t n = 0; n < MbcInterface::romBankSize; ++n) {
            mbc.rom[romBankAddr(i) + n] = (uint8_t)i;
        }
    }


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
    Mbc1 mbc(2_MB, 32_KB);
    
    // fill each bank with the bank number
    for(uint32_t i = 0; i < mbc.rom.size() / MbcInterface::romBankSize; ++i) {
        for (uint32_t n = 0; n < MbcInterface::romBankSize; ++n) {
            mbc.rom[romBankAddr(i) + n] = (uint8_t)i;
        }
    }


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



TEST_CASE("MBC2") {

    SUBCASE("Test different rom sizes") {

        Mbc2 mbc;

        SUBCASE("64KB rom") { mbc = Mbc2(64_KB, 0); }
        SUBCASE("128KB rom") { mbc = Mbc2(128_KB, 0); }
        SUBCASE("256KB rom") { mbc = Mbc2(256_KB, 0); }
        
        size_t romSize = mbc.rom.size();
        size_t bankCount = mbc.rom.size() / MbcInterface::romBankSize;

        // write the bank number at the beginning of each bank
        for (uint32_t bank = 0; bank < bankCount; ++bank) {
            mbc.rom[romBankAddr(bank)] = uint8_t(bank);
        }

        CAPTURE(romSize);
        CAPTURE(bankCount);

        // check reading from bank 0
        auto val = mbc.read8(mmap::rom::bank0::start);

        CHECK(val == 0);

        // access all rom banks and check the value
        for (uint32_t bank = 0; bank < bankCount; ++bank) {

            // the bank number must be written using an address in the range 0x0000 - 0x3FFF 
            // that has bit 8 set
            mbc.write8(0x0100, uint8_t(bank));
            
            // read a value from rom and check it against the expected bank number
            val = mbc.read8(mmap::rom::bankN::start);

            // bank 0 is not accessible in this address range
            if(bank == 0)
                CHECK(val == 1);
            else 
                CHECK(val == bank);
        }
    }

    SUBCASE("Test ram") {

        Mbc2 mbc;

        // fill the ram with know values (only the lowest bits will be used)
        for (uint32_t i = 0; i < mbc.ram.size(); ++i) {
            mbc.ram[i] = uint8_t(i);
        }

        // check reading with ram disabled
        auto val = mbc.read8(mmap::external_ram::start);
        CHECK(val == 0xff);

        // enable ram
        mbc.write8(0x0000, 0x0A);

        // there are only 512 half-bytes that are accessible
        for (uint16_t i = 0; i < 512; ++i) {
            val = mbc.read8(mmap::external_ram::start + i);
            // the top 4 bits are always 1111
            CHECK(val == uint8_t(i | 0xF0));
            // write back a different value
            mbc.write8(mmap::external_ram::start + i, val + 1);
            // read the value back
            auto newVal = mbc.read8(mmap::external_ram::start + i);
            CHECK(newVal == uint8_t((val + 1) | 0xF0));
        }
    }
}



TEST_CASE("Test RTC")
{
    RTC rtc;

    rtc.latch();

    rtc.writeSec(30);
    std::this_thread::sleep_for(1s);
    rtc.latch();
    auto sec = rtc.readSec();

    CHECK(sec == 31);

    rtc.writeSec(59);
    std::this_thread::sleep_for(1s);
    rtc.latch();
    sec = rtc.readSec();

    CHECK(sec == 0);
}



TEST_CASE("Test MBC3")
{
    SUBCASE("Test different rom sizes") {

        Mbc3 mbc;

        SUBCASE("64KB rom") { mbc = Mbc3(64_KB, 0); }
        SUBCASE("128KB rom") { mbc = Mbc3(128_KB, 0); }
        SUBCASE("256KB rom") { mbc = Mbc3(256_KB, 0); }
        SUBCASE("512KB rom") { mbc = Mbc3(512_KB, 0); }
        SUBCASE("1MB rom") { mbc = Mbc3(1_MB, 0); }
        SUBCASE("2MB rom") { mbc = Mbc3(2_MB, 0); }

        size_t romSize = mbc.rom.size();
        size_t bankCount = mbc.rom.size() / MbcInterface::romBankSize;

        // write the bank number at the beginning of each bank
        for (uint32_t bank = 0; bank < bankCount; ++bank) {
            mbc.rom[romBankAddr(bank)] = uint8_t(bank);
        }

        CAPTURE(romSize);
        CAPTURE(bankCount);

        // check reading from bank 0
        auto val = mbc.read8(mmap::rom::bank0::start);

        CHECK(val == 0);

        // access all rom banks and check the value
        for (uint32_t bank = 0; bank < bankCount; ++bank) {

            // the bank number must be written using an address in the range 0x2000 - 0x3FFF 
            // that has bit 8 set
            mbc.write8(0x2100, uint8_t(bank));

            // read a value from rom and check it against the expected bank number
            val = mbc.read8(mmap::rom::bankN::start);

            // bank 0 is not accessible in this address range
            if (bank == 0)
                CHECK(val == 1);
            else
                CHECK(val == bank);
        }
    }

    SUBCASE("Test ram (only 32KB are supported)") {
        Mbc3 mbc(64_KB, 32_KB);

        size_t bankCount = mbc.ram.size() / MbcInterface::ramBankSize;

        // fill the first byte of each bank with the corresponding number
        for (uint32_t bank = 0; bank < bankCount; ++bank) {
            mbc.ram[ramBankAddr(bank)] = uint8_t(bank);
        }

        // check read with ram/RTC disabled
        auto val = mbc.read8(mmap::external_ram::start);

        CHECK(val == 0xff);

        // enable ram/RTC
        mbc.write8(0x0000, 0x0A);

        // for each bank read the first byt, check its value then try to modify it
        // and read it again to check that writes are working
        for (uint32_t bank = 0; bank < bankCount; ++bank) {
            // select the bank
            mbc.write8(0x4000, uint8_t(bank));
            // read the value
            val = mbc.read8(mmap::external_ram::start);
            CHECK(val == bank);

            // write a new value
            mbc.write8(mmap::external_ram::start, val | 0x80);

            // read it back and check it
            auto newVal = mbc.read8(mmap::external_ram::start);
            CHECK(newVal == (val | 0x80));
        }
    }

    SUBCASE("Test RTC") {
        Mbc3 mbc(64_KB, 32_KB);

        // write some random values into the 
        mbc.rtc.writeSec(30);
        mbc.rtc.writeMin(20);
        mbc.rtc.writeHours(12);
        mbc.rtc.writeDaysL(112);
        mbc.rtc.writeDaysH(1);

        // check read with ram/RTC disabled

        mbc.write8(0x4000, 0x08); // select seconds
        CHECK(mbc.read8(mmap::external_ram::start) == 0xff);

        mbc.write8(0x4000, 0x09); // select minutes
        CHECK(mbc.read8(mmap::external_ram::start) == 0xff);

        mbc.write8(0x4000, 0x0A); // select hours
        CHECK(mbc.read8(mmap::external_ram::start) == 0xff);

        mbc.write8(0x4000, 0x0B); // select days L
        CHECK(mbc.read8(mmap::external_ram::start) == 0xff);

        mbc.write8(0x4000, 0x0C); // select days H
        CHECK(mbc.read8(mmap::external_ram::start) == 0xff);

        // enable ram/RTC
        mbc.write8(0x0000, 0x0A);

        mbc.write8(0x4000, 0x08); // select seconds
        CHECK(mbc.read8(mmap::external_ram::start) == 30);

        mbc.write8(0x4000, 0x09); // select minutes
        CHECK(mbc.read8(mmap::external_ram::start) == 20);

        mbc.write8(0x4000, 0x0A); // select hours
        CHECK(mbc.read8(mmap::external_ram::start) == 12);

        mbc.write8(0x4000, 0x0B); // select days L
        CHECK(mbc.read8(mmap::external_ram::start) == 112);

        mbc.write8(0x4000, 0x0C); // select days H
        CHECK(mbc.read8(mmap::external_ram::start) == 1);

        // select wrong bank/register
        mbc.write8(0x4000, 0x05); 
        CHECK(mbc.read8(mmap::external_ram::start) == 0xff);


        // check latching
        mbc.write8(0x6000, 0);
        mbc.write8(0x6000, 1);

        mbc.write8(0x4000, 0x08); // select seconds
        auto secVal = mbc.read8(mmap::external_ram::start);

        std::this_thread::sleep_for(1s);

        // latch again
        mbc.write8(0x6000, 0);
        mbc.write8(0x6000, 1);

        auto newSecVal = mbc.read8(mmap::external_ram::start);

        CHECK(newSecVal == ((secVal + 1) % 60));
    }

}




TEST_CASE("Test MBC5")
{
    SUBCASE("Test different rom sizes") {

        Mbc5 mbc;

        SUBCASE("64KB rom") { mbc = Mbc5(64_KB, 0); }
        SUBCASE("128KB rom") { mbc = Mbc5(128_KB, 0); }
        SUBCASE("256KB rom") { mbc = Mbc5(256_KB, 0); }
        SUBCASE("512KB rom") { mbc = Mbc5(512_KB, 0); }
        SUBCASE("1MB rom") { mbc = Mbc5(1_MB, 0); }
        SUBCASE("2MB rom") { mbc = Mbc5(2_MB, 0); }
        SUBCASE("4MB rom") { mbc = Mbc5(4_MB, 0); }
        SUBCASE("8MB rom") { mbc = Mbc5(8_MB, 0); }

        size_t romSize = mbc.rom.size();
        size_t bankCount = mbc.rom.size() / MbcInterface::romBankSize;

        // write the bank number at the beginning of each bank
        // in this case write the bank number as a 16-bit value, as there can bu up to 512 banks
        for (uint32_t bank = 0; bank < bankCount; ++bank) {
            mbc.rom[romBankAddr(bank) + 0] = uint8_t(bank);
            mbc.rom[romBankAddr(bank) + 1] = uint8_t(bank >> 8);
        }

        CAPTURE(romSize);
        CAPTURE(bankCount);

        // check reading from bank 0
        auto lsb = mbc.read8(mmap::rom::bank0::start);
        auto msb = mbc.read8(mmap::rom::bank0::start + 1);

        CHECK((lsb | (msb << 8)) == 0);

        // access all rom banks and check the value
        for (uint32_t bank = 0; bank < bankCount; ++bank) {

            // write the bank value in the registers
            mbc.write8(0x2000, uint8_t(bank));
            mbc.write8(0x3000, uint8_t(bank >> 8));

            // read a value from rom and check it against the expected bank number
            lsb = mbc.read8(mmap::rom::bankN::start);
            msb = mbc.read8(mmap::rom::bankN::start + 1);

            CHECK((lsb | (msb << 8)) == bank);
        }
    }

    SUBCASE("Test different ram sizes") {
        Mbc5 mbc;

        SUBCASE("16KB ram") { mbc = Mbc5(64_KB, 16_KB); }
        SUBCASE("32KB ram") { mbc = Mbc5(64_KB, 32_KB); }
        SUBCASE("64KB ram") { mbc = Mbc5(64_KB, 64_KB); }
        SUBCASE("128KB ram") { mbc = Mbc5(64_KB, 128_KB); }

        size_t ramSize = mbc.ram.size();
        size_t bankCount = mbc.ram.size() / MbcInterface::ramBankSize;

        // fill the first byte of each bank with the corresponding number
        for (uint32_t bank = 0; bank < bankCount; ++bank) {
            mbc.ram[ramBankAddr(bank)] = uint8_t(bank);
        }

        CAPTURE(ramSize);
        CAPTURE(bankCount);

        // check read with ram disabled
        auto val = mbc.read8(mmap::external_ram::start);

        CHECK(val == 0xff);

        // enable ram
        mbc.write8(0x0000, 0x0A);

        // for each bank read the first byt, check its value then try to modify it
        // and read it again to check that writes are working
        for (uint32_t bank = 0; bank < bankCount; ++bank) {
            // select the bank
            mbc.write8(0x4000, uint8_t(bank));
            // read the value
            val = mbc.read8(mmap::external_ram::start);
            CHECK(val == bank);

            // write a new value
            mbc.write8(mmap::external_ram::start, val | 0x80);

            // read it back and check it
            auto newVal = mbc.read8(mmap::external_ram::start);
            CHECK(newVal == (val | 0x80));
        }
    }
}




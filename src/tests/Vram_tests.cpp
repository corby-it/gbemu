
#include "gb/Vram.h"
#include "gb/GbCommons.h"
#include "doctest/doctest.h"


TEST_CASE("Test TileData.get() function")
{
    // test data from https://www.huderlem.com/demos/gameboy2bpp.html

    uint8_t data1[] = {
        0xFF, 0x00,
        0x7E, 0xFF,
        0x85, 0x81,
        0x89, 0x83,
        0x93, 0x85,
        0xA5, 0x8B,
        0xC9, 0x97, 
        0x7E, 0xFF
    };
    
    TileData td1(0x2222, data1);
    
    CHECK(td1.get(0, 0) == 1);
    CHECK(td1.get(0, 1) == 2);
    CHECK(td1.get(1, 1) == 3);
    CHECK(td1.get(2, 6) == 0);
    CHECK(td1.get(7, 7) == 2);


    uint8_t data2[] = { 
        0x7C, 0x7C, 
        0x00, 0xC6, 
        0xC6, 0x00, 
        0x00, 0xFE, 
        0xC6, 0xC6, 
        0x00, 0xC6, 
        0xC6, 0x00,
        0x00, 0x00
    };

    TileData td2(0x2222, data2);

    CHECK(td2.get(0, 0) == 0);
    CHECK(td2.get(3, 0) == 3);
    CHECK(td2.get(1, 3) == 2);
    CHECK(td2.get(1, 6) == 1);
    CHECK(td2.get(7, 7) == 0);
}

TEST_CASE("Test TileData.set() function")
{
    uint8_t data1[TileData::size] = { 0 };

    TileData td(0x2222, data1);

    td.set(0, 0, 3);
    td.set(2, 3, 3);
    td.set(6, 5, 3);
    td.set(7, 7, 3);

    CHECK(td.ptr[0] == 0x80);
    CHECK(td.ptr[1] == 0x80);

    CHECK(td.ptr[6] == 0x20);
    CHECK(td.ptr[7] == 0x20);

    CHECK(td.ptr[10] == 0x02);
    CHECK(td.ptr[11] == 0x02);

    CHECK(td.ptr[14] == 0x01);
    CHECK(td.ptr[15] == 0x01);
}



TEST_CASE("Test ObjTileData.get() function")
{
    // test data from https://www.huderlem.com/demos/gameboy2bpp.html

    uint8_t data[] = {
        0xFF, 0x00,
        0x7E, 0xFF,
        0x85, 0x81,
        0x89, 0x83,
        0x93, 0x85,
        0xA5, 0x8B,
        0xC9, 0x97,
        0x7E, 0xFF,

        0x7C, 0x7C,
        0x00, 0xC6,
        0xC6, 0x00,
        0x00, 0xFE,
        0xC6, 0xC6,
        0x00, 0xC6,
        0xC6, 0x00,
        0x00, 0x00
    };

    ObjTileData otd(0x2222, data);

    // check upper 8x8 tile
    CHECK(otd.get(0, 0) == 1);
    CHECK(otd.get(0, 1) == 2);
    CHECK(otd.get(1, 1) == 3);
    CHECK(otd.get(2, 6) == 0);
    CHECK(otd.get(7, 7) == 2);

    // check lower 8x8 tile
    CHECK(otd.get(0, 8 + 0) == 0);
    CHECK(otd.get(3, 8 + 0) == 3);
    CHECK(otd.get(1, 8 + 3) == 2);
    CHECK(otd.get(1, 8 + 6) == 1);
    CHECK(otd.get(7, 8 + 7) == 0);
}


TEST_CASE("Test ObjTileData.set() function")
{
    uint8_t data[ObjTileData::size] = { 0 };

    ObjTileData otd(0x2222, data);

    // check upper 8x8 tile
    otd.set(0, 0, 1);
    otd.set(0, 1, 2);
    otd.set(1, 1, 3);
    otd.set(2, 6, 0);
    otd.set(7, 7, 2);

    CHECK(otd.get(0, 0) == 1);
    CHECK(otd.get(0, 1) == 2);
    CHECK(otd.get(1, 1) == 3);
    CHECK(otd.get(2, 6) == 0);
    CHECK(otd.get(7, 7) == 2);

    // check lower 8x8 tile
    otd.set(0, 8 + 0, 0);
    otd.set(3, 8 + 0, 3);
    otd.set(1, 8 + 3, 2);
    otd.set(1, 8 + 6, 1);
    otd.set(7, 8 + 7, 0);

    CHECK(otd.get(0, 8 + 0) == 0);
    CHECK(otd.get(3, 8 + 0) == 3);
    CHECK(otd.get(1, 8 + 3) == 2);
    CHECK(otd.get(1, 8 + 6) == 1);
    CHECK(otd.get(7, 8 + 7) == 0);
}



TEST_CASE("Test TileMap get() function")
{
    uint8_t data[TileMap::size] = { 0 };

    data[0] = 2;        // 0, 0
    data[35] = 12;      // 3, 1
    data[233] = 22;     // 9, 7
    data[475] = 99;     // 27, 14
    data[667] = 203;    // 27, 20
    data[990] = 11;     // 30, 30
    data[1023] = 74;    // 31, 31

    TileMap tm(0x2222, data);

    CHECK(tm.get(0, 0) == 2);
    CHECK(tm.get(3, 1) == 12);
    CHECK(tm.get(9, 7) == 22);
    CHECK(tm.get(27, 14) == 99);
    CHECK(tm.get(27, 20) == 203);
    CHECK(tm.get(30, 30) == 11);
    CHECK(tm.get(31, 31) == 74);
}

TEST_CASE("Test TileMap set() function")
{
    uint8_t data[TileMap::size] = { 0 };

    TileMap tm(0x2222, data);

    tm.set(0, 0, 2);
    tm.set(3, 1, 12);
    tm.set(9, 7, 22);
    tm.set(27, 14, 99);
    tm.set(27, 20, 203);
    tm.set(30, 30, 11);
    tm.set(31, 31, 74);

    CHECK(tm.get(0, 0) == 2);
    CHECK(tm.get(3, 1) == 12);
    CHECK(tm.get(9, 7) == 22);
    CHECK(tm.get(27, 14) == 99);
    CHECK(tm.get(27, 20) == 203);
    CHECK(tm.get(30, 30) == 11);
    CHECK(tm.get(31, 31) == 74);
}


TEST_CASE("Test Dispaly get/set functions")
{
    Display d;

    d.set(0, 0, 0);
    d.set(10, 10, 1);
    d.set(50, 40, 2);
    d.set(130, 100, 0);
    d.set(159, 143, 3);

    CHECK(d.get(0, 0) == 0);
    CHECK(d.get(10, 10) == 1);
    CHECK(d.get(50, 40) == 2);
    CHECK(d.get(130, 100) == 0);
    CHECK(d.get(159, 143) == 3);
}
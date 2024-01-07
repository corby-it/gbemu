
#include "gb/Vram.h"
#include "gb/GbCommons.h"
#include "doctest/doctest.h"


TEST_CASE("Test TileData pix() function")
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
    
    CHECK(td1.pix(0, 0) == 1);
    CHECK(td1.pix(0, 1) == 2);
    CHECK(td1.pix(1, 1) == 3);
    CHECK(td1.pix(2, 6) == 0);
    CHECK(td1.pix(7, 7) == 2);


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

    CHECK(td2.pix(0, 0) == 0);
    CHECK(td2.pix(3, 0) == 3);
    CHECK(td2.pix(1, 3) == 2);
    CHECK(td2.pix(1, 6) == 1);
    CHECK(td2.pix(7, 7) == 0);
}


TEST_CASE("Test ObjTileData pix() function")
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
    CHECK(otd.pix(0, 0) == 1);
    CHECK(otd.pix(0, 1) == 2);
    CHECK(otd.pix(1, 1) == 3);
    CHECK(otd.pix(2, 6) == 0);
    CHECK(otd.pix(7, 7) == 2);

    // check lower 8x8 tile
    CHECK(otd.pix(0, 8 + 0) == 0);
    CHECK(otd.pix(3, 8 + 0) == 3);
    CHECK(otd.pix(1, 8 + 3) == 2);
    CHECK(otd.pix(1, 8 + 6) == 1);
    CHECK(otd.pix(7, 8 + 7) == 0);
}


TEST_CASE("Test TileMap getTileId() function")
{
    uint8_t data[1024] = { 0 };

    data[0] = 2;        // 0, 0
    data[35] = 12;      // 3, 1
    data[233] = 22;     // 9, 7
    data[475] = 99;     // 27, 14
    data[667] = 203;    // 27, 20
    data[990] = 11;     // 30, 30
    data[1023] = 74;    // 31, 31

    TileMap tm(0x2222, data);

    CHECK(tm.getTileId(0, 0) == 2);
    CHECK(tm.getTileId(3, 1) == 12);
    CHECK(tm.getTileId(9, 7) == 22);
    CHECK(tm.getTileId(27, 14) == 99);
    CHECK(tm.getTileId(27, 20) == 203);
    CHECK(tm.getTileId(30, 30) == 11);
    CHECK(tm.getTileId(31, 31) == 74);
}
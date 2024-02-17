
#include "gb/Cartridge.h"
#include "gb/GbCommons.h"
#include <filesystem>
#include <vector>
#include <fstream>
#include <vector>
#include "doctest/doctest.h"


namespace fs = std::filesystem;

static fs::path getTestRoot()
{
    auto curr = fs::current_path();
    fs::path ret;

    for (const auto& sub : curr) {
        ret /= sub;

        if (sub == "gbemu")
            break;
    }

    ret /= "test-files";

    return ret;
}

static const fs::path testFilesRoot = getTestRoot();



static std::vector<uint8_t> readFile(const fs::path& path)
{
    if (fs::exists(path) && fs::is_regular_file(path)) {
        auto size = fs::file_size(path);

        std::vector<uint8_t> data;
        data.resize(size);

        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        ifs.read((char*)data.data(), size);

        return data;
    }
    else {
        return {};
    }
}

static constexpr EntryPointData expectedEntryPoint = { 0x00, 0xC3, 0x50, 0x01 };


static constexpr LogoData expectedLogoData = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
};


TEST_CASE("Cartridge header parsing - Tetris")
{
    auto romData = readFile(testFilesRoot / "tetris-header.gb");

    REQUIRE(romData.size() > 0);

    CartridgeHeader header(romData.data());

    CHECK(header.entryPoint() == expectedEntryPoint);
    CHECK(header.logoData() == expectedLogoData);
    CHECK(header.title() == "TETRIS");
    CHECK(header.cgbFlag() == CGBFlag::CGBIncompatible);
    CHECK(header.newLicenseeCode() == "");
    CHECK(header.sgbFlag() == SGBFlag::GB);
    CHECK(header.cartType() == CartridgeType::NoMBC);
    CHECK(header.romSize() == 32 * 1024);
    CHECK(header.ramSize() == 0);
    CHECK(header.destCode() == DestCode::Japan);
    CHECK(header.oldLicenseeCode() == "Nintendo");
    CHECK(header.maskRomVersionNum() == 0x01);
    CHECK(header.headerChecksum() == 0x0A);
    CHECK(header.globalChecksum() == 0x16BF);
    CHECK(header.verifyHeaderChecksum());
}

TEST_CASE("Cartridge header parsing - Pokemon Red")
{
    auto romData = readFile(testFilesRoot / "pokemon-red-header.gb");

    REQUIRE(romData.size() > 0);

    CartridgeHeader header(romData.data());

    CHECK(header.entryPoint() == expectedEntryPoint);
    CHECK(header.logoData() == expectedLogoData);
    CHECK(header.title() == "POKEMON RED");
    CHECK(header.cgbFlag() == CGBFlag::CGBIncompatible);
    CHECK(header.newLicenseeCode() == "Nintendo R&D1");
    CHECK(header.sgbFlag() == SGBFlag::SGB);
    CHECK(header.cartType() == CartridgeType::MBC3RamBattery);
    CHECK(header.romSize() == 1 * 1024 * 1024);
    CHECK(header.ramSize() == 32 * 1024);
    CHECK(header.destCode() == DestCode::World);
    CHECK(header.oldLicenseeCode() == "Refer to the \"New licensee code\"");
    CHECK(header.maskRomVersionNum() == 0x00);
    CHECK(header.headerChecksum() == 0x20);
    CHECK(header.globalChecksum() == 0x91E6);
    CHECK(header.verifyHeaderChecksum());
}

TEST_CASE("Cartridge header parsing - Super Mario Land")
{
    auto romData = readFile(testFilesRoot / "super-mario-land-header.gb");

    REQUIRE(romData.size() > 0);

    CartridgeHeader header(romData.data());

    CHECK(header.entryPoint() == expectedEntryPoint);
    CHECK(header.logoData() == expectedLogoData);
    CHECK(header.title() == "SUPER MARIOLAND");
    CHECK(header.cgbFlag() == CGBFlag::CGBIncompatible);
    CHECK(header.newLicenseeCode() == "");
    CHECK(header.sgbFlag() == SGBFlag::GB);
    CHECK(header.cartType() == CartridgeType::MBC1);
    CHECK(header.romSize() == 64 * 1024);
    CHECK(header.ramSize() == 0);
    CHECK(header.destCode() == DestCode::Japan);
    CHECK(header.oldLicenseeCode() == "Nintendo");
    CHECK(header.maskRomVersionNum() == 0x01);
    CHECK(header.headerChecksum() == 0x9D);
    CHECK(header.globalChecksum() == 0x5ECF);
    CHECK(header.verifyHeaderChecksum());
}

TEST_CASE("Cartridge header parsing - Zelda Link's Awakening")
{
    auto romData = readFile(testFilesRoot / "zelda-links-awakening-header.gb");

    REQUIRE(romData.size() > 0);

    CartridgeHeader header(romData.data());

    CHECK(header.entryPoint() == expectedEntryPoint);
    CHECK(header.logoData() == expectedLogoData);
    CHECK(header.title() == "ZELDA");
    CHECK(header.cgbFlag() == CGBFlag::CGBIncompatible);
    CHECK(header.newLicenseeCode() == "");
    CHECK(header.sgbFlag() == SGBFlag::GB);
    CHECK(header.cartType() == CartridgeType::MBC1RamBattery);
    CHECK(header.romSize() == 512 * 1024);
    CHECK(header.ramSize() == 8 * 1024);
    CHECK(header.destCode() == DestCode::World);
    CHECK(header.oldLicenseeCode() == "Nintendo");
    CHECK(header.maskRomVersionNum() == 0x02);
    CHECK(header.headerChecksum() == 0x6A);
    CHECK(header.globalChecksum() == 0x3AEE);
    CHECK(header.verifyHeaderChecksum());
}


TEST_CASE("Cartridge header - check if can load")
{
    uint8_t headerData[512] = { 0 };

    // fill the header data with meaningful values
    headerData[0x147] = 0; // no MBC
    headerData[0x148] = 0; // 32K rom
    headerData[0x149] = 0; // 0K ram

    SUBCASE("Check nullptr") {
        CartridgeHeader hd;

        CHECK_FALSE(hd.canLoad());
    }

    SUBCASE("Check MBC type") {
        headerData[0x147] = 0x25; // random value

        CartridgeHeader hd(headerData);

        CHECK_FALSE(hd.canLoad());
    }

    SUBCASE("Check rom sizes") {
        headerData[0x148] = 45; // random value

        CartridgeHeader hd(headerData);
        
        CHECK_FALSE(hd.canLoad());
    }

    SUBCASE("Check ram sizes") {
        headerData[0x149] = 45; // random value

        CartridgeHeader hd(headerData);

        CHECK_FALSE(hd.canLoad());
    }
}


// ------------------------------------------------------------------------------------------------




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
}




// ------------------------------------------------------------------------------------------------



TEST_CASE("Test Cartridge")
{
    Cartridge c;

    SUBCASE("Tetris - bad file (too small)") {
        auto parsing = c.loadRomFile(testFilesRoot / "tetris-fake-rom-bad.gb");

        REQUIRE_FALSE(parsing);
    }

    SUBCASE("Tetris - no MBC") {
        auto parsing = c.loadRomFile(testFilesRoot / "tetris-fake-rom-good.gb");

        REQUIRE(parsing);
        
        CHECK(c.header.romSize() == 32 * 1024);
        CHECK(c.rom.size() == c.header.romSize());

        CHECK(c.header.ramSize() == 0);
        CHECK(c.ram.size() == c.header.ramSize());

        CHECK(c.mbc->type() == MbcType::None);

        // the first byte of the rom must be C3 and the last must be F5
        CHECK(c.read8(mmap::rom::start) == 0xC3);
        CHECK(c.read8(mmap::rom::end) == 0xF5);
    }


}

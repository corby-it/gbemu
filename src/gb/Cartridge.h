

#ifndef GBEMU_SRC_GB_CARTRIDGE_H_
#define GBEMU_SRC_GB_CARTRIDGE_H_

#include "Ram.h"
#include <cstdint>
#include <array>
#include <string_view>
#include <memory>
#include <filesystem>


namespace fs = std::filesystem;



// ------------------------------------------------------------------------------------------------
// CartridgeHeader
// ------------------------------------------------------------------------------------------------

typedef std::array<uint8_t, 4>  EntryPointData;
typedef std::array<uint8_t, 48> LogoData;


enum class CGBFlag {
    CGBIncompatible,
    CGBOnly,
    CGBCompatible,
    PGBMode,
    Unknown
};

enum class SGBFlag {
    GB,
    SGB,
    Unknown
};

enum class DestCode {
    Japan,
    World,
    Unknown
};

enum class CartridgeType {
    NoMBC,
    MBC1,
    MBC1Ram,
    MBC1RamBattery,
    MBC2,
    MBC2Battery,
    RomRam,
    RomRamBattery,
    MMM01,
    MMM01Ram,
    MMM01RamBattery,
    MBC3TimerBattery,
    MBC3TimerRamBattery,
    MBC3,
    MBC3Ram,
    MBC3RamBattery,
    MBC5,
    MBC5Ram,
    MBC5RamBattery,
    MBC5Rumble,
    MBC5RumbleRam,
    MBC5RumbleRamBattery,
    MBC6,
    MBC7SensorRumbleRamBattery,
    PocketCamera,
    BandaiTama5,
    HuC3,
    HuC1RamBattery,
    Unknown
};



class CartridgeHeader {
public:
    CartridgeHeader(const uint8_t* romBaseAddr)
        : mRomBaseAddr(romBaseAddr)
    {}


    EntryPointData entryPoint() const;
    LogoData logoData() const;
    std::string_view title() const;
    CGBFlag cgbFlag() const;
    std::string_view newLicenseeCode() const;
    SGBFlag sgbFlag() const;
    CartridgeType cartType() const;
    uint32_t romSize() const;
    uint32_t ramSize() const;
    DestCode destCode() const;
    std::string_view oldLicenseeCode() const;
    uint8_t maskRomVersionNum() const;
    uint8_t headerChecksum() const;
    uint16_t globalChecksum() const;

    bool verifyHeaderChecksum() const;
    bool verifyGlobalChecksum() const;


private:

    const uint8_t* mRomBaseAddr;

};



// ------------------------------------------------------------------------------------------------
// MBC
// ------------------------------------------------------------------------------------------------

class MbcInterface {
public:
    MbcInterface(uint8_t *romPtr, uint32_t romSize, uint8_t *ramPtr, uint32_t ramSize);
    virtual ~MbcInterface() {}

    virtual uint8_t read8(uint16_t addr) const = 0;
    virtual void write8(uint16_t addr, uint8_t val) = 0;


protected:
    uint8_t *mRomPtr;
    const uint32_t mRomSize;

    uint8_t *mRamPtr;
    const uint32_t mRamSize;

    uint8_t mRomCurrBank;
    uint8_t mRamCurrBank;
};


class MbcNone : public MbcInterface {
public:
    MbcNone(uint8_t *romPtr, uint32_t romSize);

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

};





// ------------------------------------------------------------------------------------------------
// Cartridge
// ------------------------------------------------------------------------------------------------

class Cartridge {
public:
    Cartridge();

    bool parseRomFile(const fs::path& romPath);


    uint8_t read8(uint16_t addr) const;
    void write8(uint16_t addr, uint8_t val);


    MbcInterface* getMbc() { return mMbc.get(); }


private:
    std::unique_ptr<uint8_t[]> mRom;
    Ram<128 * 1024> mRam;

    std::unique_ptr<MbcInterface> mMbc;

    CartridgeHeader mHeader;
};





#endif // GBEMU_SRC_GB_CARTRIDGE_H_
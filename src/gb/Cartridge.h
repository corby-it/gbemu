

#ifndef GBEMU_SRC_GB_CARTRIDGE_H_
#define GBEMU_SRC_GB_CARTRIDGE_H_

#include "Ram.h"
#include <cstdint>
#include <array>
#include <string_view>
#include <memory>
#include <vector>
#include <filesystem>




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
    CartridgeHeader(const uint8_t* romBaseAddr = nullptr)
        : mRomBaseAddr(romBaseAddr)
    {}

    static constexpr size_t headerSize = 0x150;


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

    bool canLoad() const;

private:

    const uint8_t* mRomBaseAddr;

};



// ------------------------------------------------------------------------------------------------
// MBC
// ------------------------------------------------------------------------------------------------

enum class  MbcType {
    None,
    Mbc1,
    Mbc2,
    Mbc3,
    Mbc5,
    Mbc6,
    Mbc7,
};


class MbcInterface {
public:
    MbcInterface(MbcType type, const std::vector<uint8_t>& rom, std::vector<uint8_t>& ram);
    virtual ~MbcInterface() {}

    virtual uint8_t read8(uint16_t addr) const = 0;
    virtual void write8(uint16_t addr, uint8_t val) = 0;

    MbcType type() const { return mType; }

    uint8_t getRomBankId() const { return mRomCurrBank; }
    uint8_t getRamBankId() const { return mRamCurrBank; }

protected:
    const MbcType mType;

    const std::vector<uint8_t>& mRom;
    std::vector<uint8_t>& mRam;
    
    uint8_t mRomCurrBank;
    uint8_t mRamCurrBank;
};


class MbcNone : public MbcInterface {
public:
    MbcNone(const std::vector<uint8_t>& rom, std::vector<uint8_t>& ram);

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

};





// ------------------------------------------------------------------------------------------------
// Cartridge
// ------------------------------------------------------------------------------------------------

class Cartridge {
public:
    Cartridge();

    bool loadRomFile(const std::filesystem::path& romPath);


    uint8_t read8(uint16_t addr) const {
        return mbc->read8(addr);
    }

    void write8(uint16_t addr, uint8_t val) {
        mbc->write8(addr, val);
    }



    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;

    std::unique_ptr<MbcInterface> mbc;

    CartridgeHeader header;
};





#endif // GBEMU_SRC_GB_CARTRIDGE_H_
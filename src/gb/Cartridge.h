

#ifndef GBEMU_SRC_GB_CARTRIDGE_H_
#define GBEMU_SRC_GB_CARTRIDGE_H_

#include "Mbc.h"
#include <cstdint>
#include <array>
#include <string_view>
#include <memory>
#include <vector>
#include <filesystem>
#include <cereal/types/memory.hpp>




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

const char* CGBFlagToStr(CGBFlag cgb);


enum class SGBFlag {
    GB,
    SGB,
    Unknown
};

const char* SGBFlagToStr(SGBFlag sgb);


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

const char* cartTypeToStr(CartridgeType ct);


enum class DestCode {
    Japan,
    World,
    Unknown
};

const char* destCodeToStr(DestCode dc);



class CartridgeHeader {
public:
    CartridgeHeader(const uint8_t* romBaseAddr = nullptr)
        : mRomBaseAddr(romBaseAddr)
    {}

    static constexpr size_t headerSize = 0x150;

    std::array<uint8_t, headerSize> asArray() const;

    bool operator==(const std::array<uint8_t, headerSize>& buffer) const;
    
    bool operator!=(const std::array<uint8_t, headerSize>& buffer) const {
        return !operator==(buffer);
    }


    EntryPointData entryPoint() const;
    LogoData logoData() const;
    std::string title() const;
    CGBFlag cgbFlag() const;
    const char* newLicenseeCode() const;
    SGBFlag sgbFlag() const;
    CartridgeType cartType() const;
    uint32_t romSize() const;
    uint32_t ramSize() const;
    DestCode destCode() const;
    const char* oldLicenseeCode() const;
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
// Cartridge
// ------------------------------------------------------------------------------------------------

enum class CartridgeLoadingRes {
    Ok,
    FileError,
    FileTooSmall,
    HeaderRomSizeFileSizeMismatch,
    HeaderVerificationFailed,
    MbcNotSupported
};

const char* cartridgeLoadingResToStr(CartridgeLoadingRes lr);



class Cartridge : public ReadWriteIf {
public:
    Cartridge();
    Cartridge(const Cartridge& other);
    Cartridge& operator=(const Cartridge& other);

    void reset();

    CartridgeLoadingRes loadRomFile(const std::filesystem::path& romPath);
    CartridgeLoadingRes loadRomData(const uint8_t* data, size_t size);


    uint8_t read8(uint16_t addr) const override {
        return mbc->read8(addr);
    }

    void write8(uint16_t addr, uint8_t val) override {
        mbc->write8(addr, val);
    }


    template<class Archive>
    void save(Archive& archive, uint32_t const /*version*/) const {
        archive(mbc);
    }

    template<class Archive>
    void load(Archive& archive, uint32_t const /*version*/) {
        archive(mbc);
        header = CartridgeHeader(mbc->rom.data());
    }

    std::unique_ptr<MbcInterface> mbc;

    CartridgeHeader header;

};

CEREAL_CLASS_VERSION(Cartridge, 1);




#endif // GBEMU_SRC_GB_CARTRIDGE_H_
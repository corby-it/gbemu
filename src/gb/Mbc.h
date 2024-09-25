

#ifndef GBEMU_SRC_GB_MBC_H_
#define GBEMU_SRC_GB_MBC_H_

#include "Utils.h"
#include <cstdint>
#include <vector>
#include <cereal/types/vector.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>



// ------------------------------------------------------------------------------------------------
// MbcInterface
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
    MbcInterface(MbcType type, size_t romSize, size_t ramSize);
    virtual ~MbcInterface() {}

    void reset();

    virtual uint8_t read8(uint16_t addr) const = 0;
    virtual void write8(uint16_t addr, uint8_t val) = 0;

    MbcType type() const { return mType; }

    uint8_t getRomBankId() const { return mRomCurrBank; }
    uint8_t getRamBankId() const { return mRamCurrBank; }



    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;

    static constexpr uint16_t ramBankSize = 8 * 1024;
    static constexpr uint16_t romBankSize = 16 * 1024;



    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(rom, ram, mType, mRamBanksCount, mRamBanksCount, mRomCurrBank, mRamCurrBank);
    }


protected:

    virtual void onReset() {}

    MbcType mType;

    uint8_t mRomBanksCount;
    uint8_t mRamBanksCount;

    uint8_t mRomCurrBank;
    uint8_t mRamCurrBank;
};

CEREAL_CLASS_VERSION(MbcInterface, 1);



// ------------------------------------------------------------------------------------------------
// MbcNone
// ------------------------------------------------------------------------------------------------

class MbcNone : public MbcInterface {
public:
    MbcNone(size_t romSize = 32 * 1024, size_t ramSize = 0);

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(cereal::base_class<MbcInterface>(this));
    }

private:

    void onReset() override;

};

CEREAL_CLASS_VERSION(MbcNone, 1);


// ------------------------------------------------------------------------------------------------
// Mbc1
// ------------------------------------------------------------------------------------------------

class Mbc1 : public MbcInterface {
public:
    Mbc1(size_t romSize = 32 * 1024, size_t ramSize = 0);

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(cereal::base_class<MbcInterface>(this));
        ar(mRamEnabled, mAddrMode1, mRomBankLow, mRomBankHigh, mRomCurrBankLow);
    }

private:

    void onReset() override;

    void updateBankConfiguration();

    bool mRamEnabled;
    bool mAddrMode1;

    uint8_t mRomMask;
    uint8_t mRamMask;

    uint8_t mRomBankLow;
    uint8_t mRomBankHigh;

    uint8_t mRomCurrBankLow;
};

CEREAL_CLASS_VERSION(Mbc1, 1);



// ------------------------------------------------------------------------------------------------
// Mbc3
// ------------------------------------------------------------------------------------------------

class Mbc3 : public MbcInterface {
public:
    Mbc3(size_t romSize = 32 * 1024, size_t ramSize = 0);

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(cereal::base_class<MbcInterface>(this));
        ar(mRomBankMask, mRamBankMask, mRtc, mRtcLatchReg, mRamRtcEnabled);
    }

private:

    void onReset() override;

    uint8_t mRomBankMask;
    uint8_t mRamBankMask;

    RTC mRtc;
    uint8_t mRtcLatchReg;

    bool mRamRtcEnabled;

};

CEREAL_CLASS_VERSION(Mbc3, 1);


CEREAL_REGISTER_TYPE(MbcNone);
CEREAL_REGISTER_TYPE(Mbc1);
CEREAL_REGISTER_TYPE(Mbc3);


#endif // GBEMU_SRC_GB_MBC_H_

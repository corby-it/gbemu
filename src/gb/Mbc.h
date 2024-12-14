

#ifndef GBEMU_SRC_GB_MBC_H_
#define GBEMU_SRC_GB_MBC_H_

#include "Utils.h"
#include "GbCommons.h"
#include <cstdint>
#include <vector>
#include <memory>
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

    virtual std::unique_ptr<MbcInterface> clone() const = 0;

    void reset();

    virtual uint8_t read8(uint16_t addr) const = 0;
    virtual void write8(uint16_t addr, uint8_t val) = 0;

    MbcType type() const { return mType; }

    uint16_t getRomBankId() const { return mRomCurrBank; }
    uint16_t getRamBankId() const { return mRamCurrBank; }



    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;

    static constexpr uint16_t ramBankSize = uint16_t(8_KB);
    static constexpr uint16_t romBankSize = uint16_t(16_KB);



    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(rom, ram, mType, mRamBanksCount, mRamBanksCount, mRomCurrBank, mRamCurrBank);
    }


protected:

    virtual void onReset() {}

    MbcType mType;

    uint16_t mRomBanksCount;
    uint16_t mRamBanksCount;

    // MBC5 can have up to 512 banks of rom (8MB) so uint8_t is not enough
    uint16_t mRomCurrBank;
    uint16_t mRamCurrBank;
};

CEREAL_CLASS_VERSION(MbcInterface, 1);



// ------------------------------------------------------------------------------------------------
// MbcNone
// ------------------------------------------------------------------------------------------------

class MbcNone : public MbcInterface {
public:
    MbcNone(size_t romSize = 32_KB, size_t ramSize = 0);

    std::unique_ptr<MbcInterface> clone() const override {
        return std::make_unique<MbcNone>(*this);
    }


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
    Mbc1(size_t romSize = 32_KB, size_t ramSize = 0);

    std::unique_ptr<MbcInterface> clone() const override {
        return std::make_unique<Mbc1>(*this);
    }

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(cereal::base_class<MbcInterface>(this));
        ar(mRamEnabled, mAddrMode1, mRomMask, mRamMask, mRomBankLow, mRomBankHigh, mRomCurrBankLow);
    }

private:

    void onReset() override;

    void updateBankConfiguration();

    bool mRamEnabled;
    bool mAddrMode1;

    uint16_t mRomMask;
    uint16_t mRamMask;

    uint8_t mRomBankLow;
    uint8_t mRomBankHigh;

    uint8_t mRomCurrBankLow;
};

CEREAL_CLASS_VERSION(Mbc1, 1);


// ------------------------------------------------------------------------------------------------
// Mbc2
// ------------------------------------------------------------------------------------------------

class Mbc2 : public MbcInterface {
public:
    Mbc2(size_t romSize = 32_KB, size_t ramSize = 0);

    std::unique_ptr<MbcInterface> clone() const override {
        return std::make_unique<Mbc2>(*this);
    }


    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(cereal::base_class<MbcInterface>(this));
        ar(mRamEnabled, mRomMask);
    }

private:

    void onReset() override;

    bool mRamEnabled;

    uint16_t mRomMask;

};

CEREAL_CLASS_VERSION(Mbc2, 1);




// ------------------------------------------------------------------------------------------------
// Mbc3
// ------------------------------------------------------------------------------------------------

class Mbc3 : public MbcInterface {
public:
    Mbc3(size_t romSize = 32_KB, size_t ramSize = 0);

    std::unique_ptr<MbcInterface> clone() const override {
        return std::make_unique<Mbc3>(*this);
    }


    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(cereal::base_class<MbcInterface>(this));
        ar(mRomMask, mRamMask, rtc, mRtcLatchReg, mRamRtcEnabled);
    }


    RTC rtc;


private:

    void onReset() override;

    uint16_t mRomMask;
    uint16_t mRamMask;

    uint8_t mRtcLatchReg;

    bool mRamRtcEnabled;

};

CEREAL_CLASS_VERSION(Mbc3, 1);



// ------------------------------------------------------------------------------------------------
// Mbc5
// ------------------------------------------------------------------------------------------------

class Mbc5 : public MbcInterface {
public:
    Mbc5(size_t romSize = 32_KB, size_t ramSize = 0);

    std::unique_ptr<MbcInterface> clone() const override {
        return std::make_unique<Mbc5>(*this);
    }


    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(cereal::base_class<MbcInterface>(this));
        ar(mRamEnabled, mRomMask, mRamMask, mRomB0, mRomB1);
    }

private:

    void onReset() override;

    bool mRamEnabled;

    uint16_t mRomMask;
    uint16_t mRamMask;

    uint8_t mRomB0;
    uint8_t mRomB1;
};





CEREAL_REGISTER_TYPE(MbcNone);
CEREAL_REGISTER_TYPE(Mbc1);
CEREAL_REGISTER_TYPE(Mbc2);
CEREAL_REGISTER_TYPE(Mbc3);
CEREAL_REGISTER_TYPE(Mbc5);


#endif // GBEMU_SRC_GB_MBC_H_

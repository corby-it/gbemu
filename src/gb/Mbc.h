

#ifndef GBEMU_SRC_GB_MBC_H_
#define GBEMU_SRC_GB_MBC_H_

#include <cstdint>
#include <vector>



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
    MbcInterface(MbcType type, const std::vector<uint8_t>& rom, std::vector<uint8_t>& ram);
    virtual ~MbcInterface() {}

    void reset();

    virtual uint8_t read8(uint16_t addr) const = 0;
    virtual void write8(uint16_t addr, uint8_t val) = 0;

    MbcType type() const { return mType; }

    uint8_t getRomBankId() const { return mRomCurrBank; }
    uint8_t getRamBankId() const { return mRamCurrBank; }



    static constexpr uint16_t ramBankSize = 8 * 1024;
    static constexpr uint16_t romBankSize = 16 * 1024;


protected:

    virtual void onReset() {}

    const MbcType mType;

    const std::vector<uint8_t>& mRom;
    std::vector<uint8_t>& mRam;

    const uint8_t mRomBanksCount;
    const uint8_t mRamBanksCount;

    uint8_t mRomCurrBank;
    uint8_t mRamCurrBank;
};




// ------------------------------------------------------------------------------------------------
// MbcNone
// ------------------------------------------------------------------------------------------------

class MbcNone : public MbcInterface {
public:
    MbcNone(const std::vector<uint8_t>& rom, std::vector<uint8_t>& ram);

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

private:

    void onReset() override;

};



// ------------------------------------------------------------------------------------------------
// Mbc1
// ------------------------------------------------------------------------------------------------

class Mbc1 : public MbcInterface {
public:
    Mbc1(const std::vector<uint8_t>& rom, std::vector<uint8_t>& ram, bool hasRam);

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

private:

    void onReset() override;

    void updateBankConfiguration();

    const bool mHasRam;

    bool mRamEnabled;
    bool mAddrMode1;

    static uint8_t computeRomBankLowMask(uint8_t romBanksCount);
    const uint8_t mRomBankLowMask;

    uint8_t mRomBankLow;
    uint8_t mRomBankHigh;

    uint8_t mRomCurrBankLow;
};




// ------------------------------------------------------------------------------------------------
// Mbc3
// ------------------------------------------------------------------------------------------------

class Mbc3 : public MbcInterface {
public:
    Mbc3(const std::vector<uint8_t>& rom, std::vector<uint8_t>& ram, bool hasRam);

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

private:

    void onReset() override;

    const bool mHasRam;
};

#endif // GBEMU_SRC_GB_MBC_H_
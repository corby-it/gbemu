
#ifndef GBEMU_SRC_GB_HDMA_H_
#define GBEMU_SRC_GB_HDMA_H_

#include "Bus.h"
#include <cereal/cereal.hpp>


enum class HdmaMode {
    Stopped,
    Generic,
    HBlank,
};



class Hdma : public ReadWriteIf {
public:
    Hdma(Bus& bus);

    void reset();

    void setIsCgb(bool val) { mIsCgb = val; }


    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;

    // step() MUST be called for each machine cycle of the PPU
    // (CGB doulbe speed doesn't affect HDMA timing)
    void step(bool isPPUInHblank);

    HdmaMode currMode() const { return mMode; }



    template<class Archive>
    void save(Archive& archive, uint32_t const /*version*/) const {
        uint8_t modeU8 = static_cast<uint8_t>(mMode);
        archive(modeU8, mLen, mSrc, mDst, mSubcount, mSrcInternal, mDstInternal);
    }

    template<class Archive>
    void load(Archive& archive, uint32_t const /*version*/) {
        uint8_t modeU8 = 0;
        archive(modeU8, mLen, mSrc, mDst, mSubcount, mSrcInternal, mDstInternal);
        mMode = static_cast<HdmaMode>(modeU8);
    }


private:

    Bus* mBus;

    bool mIsCgb;

    HdmaMode mMode;

    uint8_t mLen;
    uint16_t mSrc;
    uint16_t mDst;

    bool mPrevPpuHblank;
    uint8_t mSubcount;
    uint16_t mSrcInternal;
    uint16_t mDstInternal;

};

CEREAL_CLASS_VERSION(Hdma, 1);




#endif // GBEMU_SRC_GB_HDMA_H_
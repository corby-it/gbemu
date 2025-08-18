
#ifndef GBEMU_SRC_GB_HDMA_H_
#define GBEMU_SRC_GB_HDMA_H_

#include "Bus.h"


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

private:

    Bus* mBus;

    bool mIsCgb;

    HdmaMode mMode;

    uint8_t mLen;
    uint16_t mSrc;
    uint16_t mDst;

    uint8_t mSubcount;
    uint16_t mSrcInternal;
    uint16_t mDstInternal;

};




#endif // GBEMU_SRC_GB_HDMA_H_
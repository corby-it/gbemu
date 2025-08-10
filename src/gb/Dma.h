
#ifndef GBEMU_SRC_GB_DMA_H_
#define GBEMU_SRC_GB_DMA_H_

#include "Bus.h"
#include <cstdint>
#include <cereal/cereal.hpp>


class DMA : public ReadWriteIf {
public:
    DMA(Bus& bus);

    void reset();
    void step(uint32_t mCycles);

    
    uint8_t read8(uint16_t addr) const override { 
        assert(addr == mmap::regs::lcd::dma);
        return mReg;
    }

    void write8(uint16_t addr, uint8_t val) override;

    bool isTransferring() const { return mIsTransferring; }
    bool isScheduled() const { return mIsScheduled; }


    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mReg, mWrittenAddr, mCurrAddr, mCounter, mIsScheduled, mStartTransfer, mIsTransferring);
    }


private:
    Bus* mBus;

    uint8_t mReg;

    uint16_t mWrittenAddr;
    uint16_t mCurrAddr;
    uint16_t mCounter;

    bool mIsScheduled;
    bool mStartTransfer;
    bool mIsTransferring;

};

CEREAL_CLASS_VERSION(DMA, 1);


#endif // GBEMU_SRC_GB_DMA_H_

#ifndef GBEMU_SRC_GB_DMA_H_
#define GBEMU_SRC_GB_DMA_H_

#include "Bus.h"
#include <cstdint>
#include <cereal/cereal.hpp>


class DMA {
public:
    DMA(Bus& bus);

    void reset();
    void step(uint32_t mCycles);

    uint8_t read() const { return mReg; }
    void write(uint8_t val);

    bool isTransferring() const { return mIsTransferring; }
    bool isScheduled() const { return mIsScheduled; }


    template<class Archive>
    void serialize(Archive& ar) {
        ar(mReg, mWrittenAddr, mCurrAddr, mCounter, mIsScheduled, mStartTransfer, mIsTransferring);
    }


private:
    Bus& mBus;

    uint8_t mReg;

    uint16_t mWrittenAddr;
    uint16_t mCurrAddr;
    uint16_t mCounter;

    bool mIsScheduled;
    bool mStartTransfer;
    bool mIsTransferring;

};



#endif // GBEMU_SRC_GB_DMA_H_
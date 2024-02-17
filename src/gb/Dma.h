
#ifndef GBEMU_SRC_GB_DMA_H_
#define GBEMU_SRC_GB_DMA_H_

#include "Bus.h"
#include <cstdint>


class DMA {
public:
    DMA(Bus& bus);

    bool isTransferring() const { return mIsTransferring; }

    uint8_t read() const { return mReg; }
    void write(uint8_t val);


    void step(uint8_t mCycles);


private:
    Bus& mBus;

    uint8_t mReg;

    uint16_t mStartAddr;
    uint16_t mEndAddr;
    uint16_t mCurrAddr;

    bool mIsTransferring;

};



#endif // GBEMU_SRC_GB_DMA_H_
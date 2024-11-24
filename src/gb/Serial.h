

#ifndef GBEMU_SRC_GB_SERIAL_H_
#define GBEMU_SRC_GB_SERIAL_H_

#include "Bus.h"
#include <cstdint>
#include <functional>
#include <cereal/cereal.hpp>


typedef std::function<void(uint8_t)>    SerialDataReadyCb;


class Serial {
public:
    Serial(Bus& bus);

    void reset();
    void step(uint32_t mCycles);

    void setSerialDataReadyCb(SerialDataReadyCb cb) { mDataReadyCb = cb; }


    uint8_t readData() const { return mRegData; }
    uint8_t readCtrl() const;

    void writeData(uint8_t val) { mRegData = val; }
    void writeCtrl(uint8_t val);

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mClockCounter, mClockCounterTarget, mShiftCounter);
        ar(mEnable, mClockSpeed, mClockIsMaster);
        ar(mRegData, mTransferredOut);
    }

private:

    Bus* mBus;

    uint32_t mClockCounter;
    uint32_t mClockCounterTarget;
    uint32_t mShiftCounter;

    bool mEnable;
    bool mClockSpeed;
    bool mClockIsMaster;

    uint8_t mRegData;
    
    uint8_t mTransferredOut;

    SerialDataReadyCb mDataReadyCb;
};

CEREAL_CLASS_VERSION(Serial, 1);



#endif // GBEMU_SRC_GB_SERIAL_H_
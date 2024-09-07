

#ifndef GBEMU_SRC_GB_SERIAL_H_
#define GBEMU_SRC_GB_SERIAL_H_

#include <cstdint>
#include <cereal/cereal.hpp>

// Dummy implementation

class Serial {
public:
    Serial() {
        reset();
    }

    void reset() {
        mData = 0x00;
        mCtrl = 0x7E;
    }

    uint8_t readData() const { return mData; }
    uint8_t readCtrl() const { return mCtrl; }

    void writeData(uint8_t val) { mData = val; }
    void writeCtrl(uint8_t val) { mCtrl = val; }

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mData, mCtrl);
    }

private:
    uint8_t mData;
    uint8_t mCtrl;

};

CEREAL_CLASS_VERSION(Serial, 1);



#endif // GBEMU_SRC_GB_SERIAL_H_
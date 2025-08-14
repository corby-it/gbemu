
#ifndef GBEMU_SRC_GB_INFRARED_H_
#define GBEMU_SRC_GB_INFRARED_H_

#include "GbCommons.h"
#include <cereal/cereal.hpp>


// Infrared communication is not actually implemented
// the purpose of this class is to handle reads and writes 
// to the infrared RP register

class Infrared : public ReadWriteIf {
public:

    Infrared()
        : mIsCgb(false)
    {
        reset();
    }

    void reset() {
        mRpReg = 0;
    }

    void setIsCgb(bool val) { mIsCgb = val; }

    // for more info on IR communication
    // see https://gbdev.io/pandocs/IR.html

    uint8_t read8(uint16_t addr) const override {

        if (mIsCgb && addr == mmap::regs::infrared) {
            // bits 2-5 are not used 
            return mRpReg | 0x3C;
        }

        return 0xFF;
    }

    void write8(uint16_t addr, uint8_t val) override {
        if (mIsCgb && addr == mmap::regs::infrared) {
            // bit 1 is read only and bits 2-5 are not used 
            mRpReg = val & ~0x3E;
        }
    }

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mRpReg);
    }


private:

    bool mIsCgb;
    uint8_t mRpReg;

};

CEREAL_CLASS_VERSION(Infrared, 1);



#endif // GBEMU_SRC_GB_INFRARED_H_
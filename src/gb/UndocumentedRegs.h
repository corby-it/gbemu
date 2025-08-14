
#ifndef GBEMU_SRC_GB_UNDOCUMENTEDREGS_H_
#define GBEMU_SRC_GB_UNDOCUMENTEDREGS_H_


#include "GbCommons.h"
#include <cereal/cereal.hpp>



class UndocumentedRegs : public ReadWriteIf {
public:
    UndocumentedRegs();

    void reset();

    void setIsCgb(bool val) { mIsCgb = val; }

    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;


    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mFF72, mFF73, mFF74, mFF75);
    }


private:

    bool mIsCgb;

    uint8_t mFF72;
    uint8_t mFF73;
    uint8_t mFF74;
    uint8_t mFF75;

};


CEREAL_CLASS_VERSION(UndocumentedRegs, 1);




#endif // GBEMU_SRC_GB_UNDOCUMENTEDREGS_H_
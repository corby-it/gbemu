
#ifndef GBEMU_SRC_GB_WORKRAM_H_
#define GBEMU_SRC_GB_WORKRAM_H_

#include "Ram.h"
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>



class WorkRam : public Ram<32_KB> {
public:
    WorkRam();

    void reset();

    virtual uint8_t read8(uint16_t addr) const override;
    virtual void write8(uint16_t addr, uint8_t val) override;


    void setIsCgb(bool val);


    static constexpr uint16_t bankSize = uint16_t(4_KB);


    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(cereal::base_class<Ram<32_KB>>(this));
        ar(mIsCgb, mCurrBank);
    }


private:
    bool mIsCgb;
    uint8_t mCurrBank;

};

CEREAL_CLASS_VERSION(WorkRam, 1);




#endif // GBEMU_SRC_GB_WORKRAM_H_
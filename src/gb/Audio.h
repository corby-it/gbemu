

#ifndef GBEMU_SRC_GB_AUDIO_H_
#define GBEMU_SRC_GB_AUDIO_H_

#include "GbCommons.h"
#include <cereal/cereal.hpp>
#include <chrono>
#include <functional>
#include <array>






// ------------------------------------------------------------------------------------------------
// Audio
// ------------------------------------------------------------------------------------------------



// Just a dummy implementation to have some registers for read/write operations

typedef std::function<void(float, float)>   OnSampleReadyCallback;


class Audio {
public:
    Audio();
    ~Audio();

    void reset();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);

    void step(uint32_t mCycles);

    void setSampleCallback(OnSampleReadyCallback cb) { mSampleCallback = cb; }

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        size_t size = mmap::regs::audio::end - mmap::regs::audio::start + 1;
        ar(cereal::binary_data(mData, size));
    }


private:

    OnSampleReadyCallback mSampleCallback;
    uint32_t mRdh;

    std::chrono::nanoseconds mTimeCounter;

    uint8_t mData[mmap::regs::audio::end - mmap::regs::audio::start + 1];


};

CEREAL_CLASS_VERSION(Audio, 1);


#endif // GBEMU_SRC_GB_AUDIO_H_
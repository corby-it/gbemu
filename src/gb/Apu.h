

#ifndef GBEMU_SRC_GB_AUDIO_H_
#define GBEMU_SRC_GB_AUDIO_H_

#include "GbCommons.h"
#include "AudioChannel.h"
#include <cereal/cereal.hpp>
#include <chrono>
#include <functional>
#include <array>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>



// ------------------------------------------------------------------------------------------------
// ApuHpfFilter
// ------------------------------------------------------------------------------------------------

class ApuHpfFilter {
public:

    ApuHpfFilter();

    void setCutoff(float fc, float fs);

    float process(float x0);

    template<class Archive>
    void serialize(Archive& ar)
    {
        ar(mB0, mB1, mB2, mA1, mA2);
        ar(mYz1, mYz2, mXz1, mXz2);
    }


private:

    float mB0, mB1, mB2;
    float mA1, mA2;

    float mYz1, mYz2;
    float mXz1, mXz2;

};

CEREAL_CLASS_VERSION(ApuHpfFilter, 1);



// ------------------------------------------------------------------------------------------------
// APU
// ------------------------------------------------------------------------------------------------

typedef std::function<void(float, float)>   OnSampleReadyCallback;


class APU {
public:
    APU(uint32_t downsamplingFreq = 44100);

    void reset();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);

    bool step(uint32_t mCycles);

    float getOutputR() const { return mOutR; }
    float getOutputL() const { return mOutL; }

    void enableHpf(bool b) { mEnableHpf = b; }

    void setSampleCallback(OnSampleReadyCallback cb) { mSampleCallback = cb; }


    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/)
    {
        ar(square1, square2, wave, noise);
        ar(mVinL, mVinR, mVolL, mVolR);
        ar(mChPanL, mChPanR);
        ar(mOutL, mOutR, mApuEnabled);
        ar(mEnableHpf, mFrameSeq, mHpfL, mHpfR);
        ar(mDownsamplingFreq, mTimeCounter);
    }


    SquareWaveChannel square1;
    SquareWaveChannel square2;
    UserWaveChannel wave;
    NoiseChannel noise;


private:

    void mix();

    void writeReg0(uint8_t val);
    uint8_t readReg0() const;

    void writeReg1(uint8_t val);
    uint8_t readReg1() const;

    void writeReg2(uint8_t val);
    uint8_t readReg2() const;

    static constexpr uint32_t chCount = 4;
    
    // NR50
    bool mVinL;
    bool mVinR;
    uint8_t mVolL;
    uint8_t mVolR;

    // NR51
    std::array<bool, chCount> mChPanL;
    std::array<bool, chCount> mChPanR;
    
    // NR52
    bool mApuEnabled;

    float mOutR;
    float mOutL;

    // internal stuff

    bool mEnableHpf;

    FrameSequencer mFrameSeq;


    std::array<AudioChannelIf*, chCount> mChannels;

    ApuHpfFilter mHpfR;
    ApuHpfFilter mHpfL;

    OnSampleReadyCallback mSampleCallback;

    uint32_t mDownsamplingFreq;
    std::chrono::nanoseconds mTimeCounter;

};

CEREAL_CLASS_VERSION(APU, 1);




#endif // GBEMU_SRC_GB_AUDIO_H_
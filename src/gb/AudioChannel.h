
#ifndef GBEMU_SRC_GB_AUDIOSOURCE_H_
#define GBEMU_SRC_GB_AUDIOSOURCE_H_

#include <cstdint>
#include <array>
#include <chrono>
#include <bitset>
#include <memory>
#include <algorithm>
#include <cstring>
#include <cereal/cereal.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>



template<typename DataT, size_t Size>
class RingBuffer {
public:

    static_assert((Size > 0) && ((Size & (Size - 1)) == 0), "Size must be a power of 2!");

    static constexpr size_t size = Size;

    RingBuffer()
        : mData(std::make_unique<DataT[]>(Size))
        , mWrHead(0)
    {
        std::fill(mData.get(), mData.get() + Size, DataT{});
    }

    RingBuffer(const RingBuffer& other)
        : mData(std::make_unique<DataT[]>(Size))
        , mWrHead(other.mWrHead)
    {
        memcpy(mData.get(), other.mData.get(), Size * sizeof(DataT));
    }

    RingBuffer& operator=(const RingBuffer& other)
    {
        memcpy(mData.get(), other.mData.get(), Size * sizeof(DataT));
        mWrHead = other.mWrHead;

        return *this;
    }

    void write(DataT sample)
    {
        mData[mWrHead] = sample;

        mWrHead = (mWrHead + 1) & (Size - 1);
    }

    void copyToBuf(DataT* buf, size_t bufSize) const
    {
        if (!buf)
            return;

        auto rdHead = mWrHead;

        while (--bufSize) {
            *buf = mData[rdHead];
            ++buf;

            rdHead = (rdHead + 1) & (Size - 1);
        }
    }


private:

    std::unique_ptr<DataT[]> mData;
    size_t mWrHead;

};






// ------------------------------------------------------------------------------------------------
// FrameSequencer
// ------------------------------------------------------------------------------------------------

class FrameSequencer {
public:
    enum class Event {
        None,
        LengthTimer,
        LengthTimerAndSweep,
        Envelope,
    };

    FrameSequencer();

    void reset();
    Event step();


    template<class Archive>
    void serialize(Archive& ar)
    {
        ar(mDivApuSubtickCounter);
        ar(mFrameSequencerCounter);
    }


private:

    uint16_t mDivApuSubtickCounter;
    uint8_t mFrameSequencerCounter;

};

CEREAL_CLASS_VERSION(FrameSequencer, 1);




// ------------------------------------------------------------------------------------------------
// AudioChannelIf
// ------------------------------------------------------------------------------------------------

class AudioChannelIf {
public:
    AudioChannelIf(uint16_t lengthTimerTarget);

    virtual void reset();

    // call this step function when the channel is emulated on its own
    // if it's emulated inside something else then, replicate its functionality
    // using its individual steps (that it, the onStep() and computeChannelOutput() functions)
    bool step();

    void setDownsamplingFreq(uint32_t freq) { mDownsamplingFreq = freq; }
    void enableInternalFrameSequencer(bool b) { mUseInternalFrameSequencer = b; }

    uint8_t getOutput() const { return mCurrOutput; }


    virtual void writeReg0(uint8_t /*val*/) {}
    virtual uint8_t readReg0() const { return 0xff; }

    virtual void writeReg1(uint8_t /*val*/) {}
    virtual uint8_t readReg1() const { return 0xff; }

    virtual void writeReg2(uint8_t /*val*/) {}
    virtual uint8_t readReg2() const { return 0xff; }

    virtual void writeReg3(uint8_t /*val*/) { }
    virtual uint8_t readReg3() const { return 0xff; }

    virtual void writeReg4(uint8_t /*val*/) {}
    virtual uint8_t readReg4() const { return 0xff; }

    
    virtual void envelopeTick() {}
    virtual void sweepTick() {}
    void lengthTimerTick();

    // returns true when the implementation decides that a new sample value must be computed
    virtual bool onStep() = 0;
    // update the current output value
    void updateChannelOutput();


    bool isChEnabled() const { return mChEnabled; }
    bool isDacEnabled() const { return mDacEnabled; }


    typedef RingBuffer<float, 1024> RingBufferType;

    const RingBufferType& getAudioBuffer() const { return mAudioRingBuf; }


    template<class Archive>
    void serialize(Archive& ar)
    {
        ar(mChEnabled, mDacEnabled, mCurrOutput);
        ar(mLengthTimerEnable, mLengthTimerCounter, mLengthTimerTargetVal);
        ar(mUseInternalFrameSequencer, mFrameSeq);
        ar(mTimeCounter, mDownsamplingFreq);
    }



protected:

    virtual uint8_t computeChannelOutput() = 0;

    void trigger();
    virtual void onTrigger() = 0;

    // length timer counter value can be changed at any time
    void setLengthTimerCounter(uint16_t val) { mLengthTimerCounter = val; }
    void enableLengthTimer(bool b) { mLengthTimerEnable = b; }
    bool isLengthTimerEnabled() const { return mLengthTimerEnable; }

    bool mChEnabled;
    bool mDacEnabled;


private:

    uint8_t mCurrOutput;

    // length timer stuff
    bool mLengthTimerEnable;
    uint16_t mLengthTimerCounter;
    uint16_t mLengthTimerTargetVal;

    // frame sequencer stuff
    bool mUseInternalFrameSequencer;
    FrameSequencer mFrameSeq;

    // downsampling counter
    std::chrono::nanoseconds mTimeCounter;
    uint32_t mDownsamplingFreq;


    RingBufferType mAudioRingBuf;
};

CEREAL_CLASS_VERSION(AudioChannelIf, 1);




// ------------------------------------------------------------------------------------------------
// SquareWaveChannel
// ------------------------------------------------------------------------------------------------

class SquareWaveChannel : public AudioChannelIf {
public:
    SquareWaveChannel();

    void enableSweepModulation(bool b) { mHasSweep = b; }
    
    void reset() override;


    void writeReg0(uint8_t val) override;
    uint8_t readReg0() const override;

    void writeReg1(uint8_t val) override;
    uint8_t readReg1() const override;

    void writeReg2(uint8_t val) override;
    uint8_t readReg2() const override;

    void writeReg3(uint8_t val) override { mPeriodL = val; }
    uint8_t readReg3() const override { return mPeriodL; }

    void writeReg4(uint8_t val) override;
    uint8_t readReg4() const override;


    void envelopeTick() override;
    void sweepTick() override;


    template<class Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::base_class<AudioChannelIf>(this));
        ar(mHasSweep, mSweepPace, mSweepDir, mSweepStep);
        ar(mDutyCycleIdx, mEnvInitialVol, mEnvDir, mEnvPace);
        ar(mPeriodL, mPeriodH, mSampleIdx, mVolume, mPeriodCounter);
        ar(mEnvPaceCounter, mSweepEnabled, mSweepShadowPeriod, mSweepCounter);
    }


private:
    bool onStep() override;
    uint8_t computeChannelOutput() override;

    uint16_t sweepCompute();

    void onTrigger() override;

    typedef std::array<uint8_t, 8>    WaveTable;

    static const std::array<WaveTable, 4> wavetables;

    bool mHasSweep;

    // Reg 0 - sweep control
    uint8_t mSweepPace;
    bool mSweepDir;
    uint8_t mSweepStep;

    // Reg 1 - length timer and duty cycle
    uint8_t mDutyCycleIdx;

    // Reg 2 - volume and envelope
    uint8_t mEnvInitialVol;
    bool mEnvDir;
    uint8_t mEnvPace;

    // Reg 3 - period low
    uint8_t mPeriodL;

    // Reg 4 - period high and control
    uint8_t mPeriodH;
    
    // counters and other internal stuff
    uint8_t mSampleIdx;
    uint8_t mVolume;

    // subticks
    uint16_t mPeriodCounter;
    
    // other functions counters
    uint8_t mEnvPaceCounter;
    bool mSweepEnabled;
    uint16_t mSweepShadowPeriod;
    uint8_t mSweepCounter;
};

CEREAL_CLASS_VERSION(SquareWaveChannel, 1);



// ------------------------------------------------------------------------------------------------
// NoiseChannel
// ------------------------------------------------------------------------------------------------

class NoiseChannel : public AudioChannelIf {
public:
    NoiseChannel();

    void reset() override;


    // there is no reg 0 in the noise channel
    
    void writeReg1(uint8_t val) override;
    uint8_t readReg1() const override;

    void writeReg2(uint8_t val) override;
    uint8_t readReg2() const override;

    void writeReg3(uint8_t val) override;
    uint8_t readReg3() const override;

    void writeReg4(uint8_t val) override;
    uint8_t readReg4() const override;

    
    void envelopeTick() override;

    template<class Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::base_class<AudioChannelIf>(this));
        ar(mEnvInitialVol, mEnvDir, mEnvPace);
        ar(mClockDivider, mLfsrWidthIs7, mClockShift);
        ar(mVolume, mLfsr);
        ar(mClockCounter, mClockCounterTarget, mEnvPaceCounter);
    }
    

private:

    bool onStep() override;
    uint8_t computeChannelOutput() override;

    void onTrigger() override;


    // Reg 1 - length timer initial val

    // Reg 2 - volume and envelope
    uint8_t mEnvInitialVol;
    bool mEnvDir;
    uint8_t mEnvPace;

    // Reg 3 - frequency and randomness
    uint8_t mClockDivider;
    bool mLfsrWidthIs7;
    uint8_t mClockShift;

    // Reg 4 - trigger and length timer enable

    // counters and other internal stuff
    uint8_t mVolume;
    std::bitset<16> mLfsr;

    // subticks
    uint16_t mClockCounter;
    uint16_t mClockCounterTarget;

    // other functions counters
    uint8_t mEnvPaceCounter;
    
};

CEREAL_CLASS_VERSION(NoiseChannel, 1);



// ------------------------------------------------------------------------------------------------
// UserWaveChannel
// ------------------------------------------------------------------------------------------------

class UserWaveChannel : public AudioChannelIf {
public:
    UserWaveChannel();

    void reset() override;
    void resetWaveRam();

    
    void writeReg0(uint8_t val) override;
    uint8_t readReg0() const override;

    void writeReg1(uint8_t val) override;
    uint8_t readReg1() const override;

    void writeReg2(uint8_t val) override;
    uint8_t readReg2() const override;

    void writeReg3(uint8_t val) override { mPeriodL = val; }
    uint8_t readReg3() const override { return mPeriodL; }

    void writeReg4(uint8_t val) override;
    uint8_t readReg4() const override;

    void writeWaveRam(uint16_t addr, uint8_t val);
    uint8_t readWaveRam(uint16_t addr) const;


    template<class Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::base_class<AudioChannelIf>(this));
        ar(mOutputVolume, mPeriodL, mPeriodH);
        ar(mWaveRam, mWaveRamIdx, mPeriodCounter);
    }


private:

    bool onStep() override;
    uint8_t computeChannelOutput() override;

    void onTrigger() override;

    // Reg 0 - DAC enable
    // Reg 1 - length timer initial val
    // Reg 2 - volume (this channel has no envelope)
    uint8_t mOutputVolume;

    // Reg 3 - period low
    uint8_t mPeriodL;

    // Reg 4 - period high and control
    uint8_t mPeriodH;


    // counters and other internal stuff
    std::array<uint8_t, 32> mWaveRam;
    uint8_t mWaveRamIdx;
    
    uint16_t mPeriodCounter;

};

CEREAL_CLASS_VERSION(UserWaveChannel, 1);




#endif // GBEMU_SRC_GB_AUDIOSOURCE_H_

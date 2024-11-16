
#ifndef GBEMU_SRC_GB_AUDIOSOURCE_H_
#define GBEMU_SRC_GB_AUDIOSOURCE_H_

#include <cstdint>
#include <array>
#include <chrono>
#include <bitset>




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


private:

    uint16_t mDivApuSubtickCounter;
    uint8_t mFrameSequencerCounter;

};




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

protected:

    virtual uint8_t computeChannelOutput() = 0;

    void enableLengthTimer(uint16_t startVal);
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

};




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


private:
    bool onStep() override;
    uint8_t computeChannelOutput() override;

    uint16_t sweepCompute();

    void trigger();

    typedef std::array<uint8_t, 8>    WaveTable;

    static const std::array<WaveTable, 4> wavetables;

    bool mHasSweep;

    // Reg 0 - sweep control
    uint8_t mSweepPace;
    bool mSweepDir;
    uint8_t mSweepStep;

    // Reg 1 - length timer and duty cycle
    uint8_t mLengthTimerInitialVal;
    uint8_t mDutyCycleIdx;

    // Reg 2 - volume and envelope
    uint8_t mEnvInitialVol;
    bool mEnvDir;
    uint8_t mEnvPace;

    // Reg 3 - period low
    uint8_t mPeriodL;

    // Reg 4 - period high and control
    uint8_t mPeriodH;
    bool mTrigger;

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
    

private:

    bool onStep() override;
    uint8_t computeChannelOutput() override;

    void trigger();


    // Reg 1 - length timer initial val
    uint8_t mLengthTimerInitialVal;

    // Reg 2 - volume and envelope
    uint8_t mEnvInitialVol;
    bool mEnvDir;
    uint8_t mEnvPace;

    // Reg 3 - frequency and randomness
    uint8_t mClockDivider;
    bool mLfsrWidthIs7;
    uint8_t mClockShift;

    // Reg 4 - trigger and length timer enable
    bool mTrigger;

    // counters and other internal stuff
    uint8_t mVolume;
    std::bitset<16> mLfsr;

    // subticks
    uint16_t mClockCounter;
    uint16_t mClockCounterTarget;

    // other functions counters
    uint8_t mEnvPaceCounter;
    
};



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
    uint8_t readWaveRam(uint16_t addr);



private:

    bool onStep() override;
    uint8_t computeChannelOutput() override;

    void trigger();

    // Reg 0 - DAC enable
    // Reg 1 - length timer initial val
    uint8_t mLengthTimerInitialVal;

    // Reg 2 - volume (this channel has no envelope)
    uint8_t mOutputVolume;

    // Reg 3 - period low
    uint8_t mPeriodL;

    // Reg 4 - period high and control
    uint8_t mPeriodH;
    bool mTrigger;


    // counters and other internal stuff
    std::array<uint8_t, 32> mWaveRam;
    uint8_t mWaveRamIdx;
    
    uint16_t mPeriodCounter;

};





#endif // GBEMU_SRC_GB_AUDIOSOURCE_H_

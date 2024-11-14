
#include "AudioSource.h"
#include "GameBoyCore.h"
#include "GbCommons.h"
#include <cassert>


using namespace std::chrono;



// ------------------------------------------------------------------------------------------------
// FrameSequencer
// ------------------------------------------------------------------------------------------------

FrameSequencer::FrameSequencer()
{
    reset();
}

void FrameSequencer::reset()
{
    mDivApuSubtickCounter = 0;
    mFrameSequencerCounter = 0;
}

FrameSequencer::Event FrameSequencer::step()
{
    // other functions such as envelope, sweep and length timer
    // are triggered by subticks of the main clock and are tied to a 
    // specific bit of the div register, specifically to bit 4 going from 1 to 0
    // this sub clock is called DIV-APU
    // usually, if div is not reset, the DIV-APU subclock ticks at 512hz (every 2048 main clock cycles)

    // this means that if the div register is reset at the right time, this 
    // falling edge may occur more frequently than expected

    // the envelope sweep ticks every 8 DIV-APU ticks (64 HZ)
    // the length timer ticks every 2 DIV-APU ticks (256 HZ)
    // the frequency sweep ticks every 4 DIV-APU ticks (128 HZ)

    // see: https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frame_Sequencer

    auto evt = Event::None;

    mDivApuSubtickCounter++;
    if (mDivApuSubtickCounter == 2048) {
        mDivApuSubtickCounter = 0;

        switch (mFrameSequencerCounter) {
        case 0: evt = Event::LengthTimer; break;
        case 2: evt = Event::LengthTimerAndSweep; break;
        case 4: evt = Event::LengthTimer; break;
        case 6: evt = Event::LengthTimerAndSweep; break;
        case 7: evt = Event::Envelope; break;
        default:
            evt = Event::None; break;
        }

        // wrap around every 8 steps;
        mFrameSequencerCounter = (mFrameSequencerCounter + 1) & 0x07;
    }

    return evt;
}




// ------------------------------------------------------------------------------------------------
// AudioChannelIf
// ------------------------------------------------------------------------------------------------

AudioChannelIf::AudioChannelIf(uint16_t lengthTimerTarget)
    : mUseInternalFrameSequencer(false)
    , mLengthTimerTargetVal(lengthTimerTarget)
    , mDownsamplingFreq(44100)
{
    reset();
}

void AudioChannelIf::reset()
{
    mFrameSeq.reset();

    mChEnabled = false;
    mDacEnabled = false;

    mCurrOutput = 0;

    mLengthTimerEnable = false;
    mLengthTimerCounter = 0;

    mTimeCounter = 0ns;
}

bool AudioChannelIf::step()
{
    // do the internal computations to determine the current output value
    bool updateOut = onStep();

    // run the internal frame sequencer if necessary
    if (mUseInternalFrameSequencer) {

        switch (mFrameSeq.step()) {
        case FrameSequencer::Event::LengthTimer:
            lengthTimerTick();
            break;
        case FrameSequencer::Event::LengthTimerAndSweep:
            lengthTimerTick();
            sweepTick();
            break;
        case FrameSequencer::Event::Envelope:
            envelopeTick();
            break;
        default:
            break;
        }
    }

    // if a new sample is available, update the current output value
    if (updateOut) {
        if (mDacEnabled) {
            // when the dac is enabled the current output value depends on the channel status
            // if the channel is disabled the output is 0, otherwise the implmementation
            // will compute the value
            // if the channel is disabled, the integer 0 will be turned into an analog 1 by the dac
            mCurrOutput = mChEnabled ? computeChannelOutput() : 0;
        }
        else {
            // when the dac is disabled the output value stays in the middle
            // of the available range 0x0 to 0xf, that is 0x7 (which should be 0 in the analog range)
            mCurrOutput = 0x7;
        }
    }


    // check downsampling, that is, check if a new sample is ready
    auto samplePeriod = duration_cast<nanoseconds>(1s) / mDownsamplingFreq;

    mTimeCounter += GameBoyClassic::machinePeriod;

    bool newSample = false;
    if (mTimeCounter >= samplePeriod) {
        mTimeCounter -= samplePeriod;
        newSample = true;
    }

    return newSample;
}


void AudioChannelIf::enableLengthTimer(uint16_t startVal)
{
    // when the length timer is enabled we have to reset the corresponding counter
    // starting from the value passed by the implementation
    mLengthTimerEnable = true;
    mLengthTimerCounter = startVal;
}

void AudioChannelIf::lengthTimerTick()
{
    // for every tick of the length timer we increase it by 1
    // when the counter reaches target value the channel is turned off
    // square wave and noise channels use 64 as target value
    // user wave channel use 256 as target value

    if (!mLengthTimerEnable)
        return;

    if (mLengthTimerCounter < mLengthTimerTargetVal) {
        mLengthTimerCounter++;

        if (mLengthTimerCounter == mLengthTimerTargetVal) {
            mChEnabled = false;
            mLengthTimerEnable = false;
        }
    }
}






// ------------------------------------------------------------------------------------------------
// SquareWaveChannel
// ------------------------------------------------------------------------------------------------

const std::array<SquareWaveChannel::WaveTable, 4> SquareWaveChannel::wavetables{ {
    { 0,0,0,0,0,0,0,1 },    // 12.5% duty cycle
    { 1,0,0,0,0,0,0,1 },    // 25% duty cycle
    { 1,0,0,0,0,1,1,1 },    // 50% duty cycle
    { 0,1,1,1,1,1,1,0 },    // 75% duty cycle
} };

SquareWaveChannel::SquareWaveChannel()
    // hasSweep is only relevent for the emulator so it doesn't need to be 
    // modified by the reset() function
    : AudioChannelIf(64)
    , mHasSweep(false)
{
    reset();
}

void SquareWaveChannel::reset()
{
    AudioChannelIf::reset();

    // Reg 0
    mSweepPace = 0;
    mSweepDir = false;
    mSweepStep = 0;
    // Reg 1
    mLengthTimerInitialVal = 0;
    mDutyCycleIdx = 0;
    // Reg 2
    mEnvInitialVol = 0;
    mEnvDir = false;
    mEnvPace = 0;
    // Reg 3
    mPeriodL = 0;
    // Reg 4
    mPeriodH = 0;
    mTrigger = false;


    mSampleIdx = 0;
    mVolume = 0;

    mPeriodCounter = 0;

    mEnvPaceCounter = 0;
    mSweepEnabled = false;
    mSweepShadowPeriod = 0;
    mSweepCounter = 0;
}


void SquareWaveChannel::writeReg0(uint8_t val)
{
    // if there is no sweep functionalty set val to 0,
    // this will disable the sweep
    if (!mHasSweep)
        val = 0;

    // bits 0-2 are the sweep step
    // bit 3 is the direction
    // bits 4-6 are the sweep pace
    // bit 7 is unused
    mSweepStep = val & 0x07;
    mSweepDir = val & 0x08;
    mSweepPace = (val & 0x70) >> 4;
}

uint8_t SquareWaveChannel::readReg0() const
{
    if (!mHasSweep)
        return 0xff;

    // the top bit is unused so we assume it reads as 1
    return 0x80 | (mSweepStep & 0x07) | (mSweepDir << 3) | ((mSweepPace & 0x07) << 4);
}

void SquareWaveChannel::writeReg1(uint8_t val)
{
    // bits 0-5 are the initial length timer
    // bits 6-7 select the duty cycle
    mLengthTimerInitialVal = val & 0x3F;
    mDutyCycleIdx = (val >> 6) & 0x03;
}

uint8_t SquareWaveChannel::readReg1() const
{
    // only the duty cycle is readable, we assume 
    // the other bits read as 1
    return 0xff & (mDutyCycleIdx << 6);
}

void SquareWaveChannel::writeReg2(uint8_t val)
{
    // bits 0-2 are the envelope pace
    // bit 3 is envelope direction
    // bits 4-7 are the initial volume
    mEnvPace = val & 0x07;
    mEnvDir = val & 0x08;
    mEnvInitialVol = (val >> 4) & 0x0f;

    // the channel dac is turned on when the top 5 bits are != 0
    // if the dac is turned off the channel is also turned off
    // if the dac is turned on, the channel stays off and it must be triggered to be turned on
    mDacEnabled = val & 0xF8;
    if (!mDacEnabled)
        mChEnabled = false;
}

uint8_t SquareWaveChannel::readReg2() const
{
    return (mEnvPace & 0x07) | (mEnvDir << 3) | (mEnvInitialVol << 4);
}

void SquareWaveChannel::writeReg4(uint8_t val)
{
    // bits 0-2 are the high bits of the period
    // bits 3-5 are unused
    // bit 6 enables the length timer
    // bit 7 triggers the channel
    mPeriodH = val & 0x07;
    bool lengthTimerEnable = val & 0x40;
    mTrigger = val & 0x80;

    if (mTrigger)
        trigger();
    if (lengthTimerEnable)
        enableLengthTimer(mLengthTimerInitialVal);
}

uint8_t SquareWaveChannel::readReg4() const
{
    // the only readable bit in this register is the length enable bit
    return 0xff & (isLengthTimerEnabled() << 6);
}

bool SquareWaveChannel::onStep()
{
    // audio is generated using the cpu main clock 
    // see https://gbdev.io/pandocs/Audio_Registers.html#ff13--nr13-channel-1-period-low-write-only
    // for the explanation of why the period counter works like this

    mPeriodCounter++;
    if (mPeriodCounter == 2048) {
        // reset the period counter using the values in the registers
        mPeriodCounter = mPeriodL | (mPeriodH << 8);

        // advance the sample index by 1 and wrap around when in reaches 8
        mSampleIdx = (mSampleIdx + 1) & 0x07;

        // new output value available
        return true;
    }

    return false;
}

uint8_t SquareWaveChannel::computeChannelOutput()
{
    // the current sample is picked from the currently selected wavetable
    // and multiplied by the current volume value
    
    return wavetables[mDutyCycleIdx][mSampleIdx] * mVolume;
}


void SquareWaveChannel::envelopeTick()
{
    // every time the envelope clock ticks we adjust the volume
    // using the parameters set in reg 2
    // if the envelope pace is 0 the volume stays constant

    if (mEnvPace > 0) {
        mEnvPaceCounter++;
        if (mEnvPaceCounter == mEnvPace) {
            mEnvPaceCounter = 0;

            if (mEnvDir) {
                // volume must increase, stop at 15
                if (mVolume < 15)
                    mVolume++;
            }
            else {
                // volume must decrease, stop at 0
                if (mVolume > 0)
                    mVolume--;
            }
        }
    }
}

void SquareWaveChannel::sweepTick()
{
    if (!mHasSweep || !mSweepEnabled)
        return;

    // every time the sweep clock ticks we have to update the periods
    // the sweep functionality writes the new period value directly to the period registers
    // where it will be picked up by the period counter on its next loop

    mSweepCounter++;
    if (mSweepCounter == mSweepPace) {
        mSweepCounter = 0;

        uint16_t newPeriod = sweepCompute();

        if (newPeriod > 0x7FF) {
            // new period overflow, disable the channel
            mChEnabled = false;
            mSweepEnabled = false;
        }
        else {
            // new period is ok, write the values back to the registers
            // and to the shadow period register
            mSweepShadowPeriod = newPeriod;
            mPeriodL = newPeriod & 0xff;
            mPeriodH = (newPeriod >> 8) & 0x07;

            // compute a new period again using the new value and perform the overflow check again
            // why check again? who knows
            // source: https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frequency_Sweep
            if (sweepCompute() > 0x7ff) {
                mChEnabled = false;
                mSweepEnabled = false;
            }
        }
    }
}

uint16_t SquareWaveChannel::sweepCompute()
{
    uint16_t tmp = mSweepShadowPeriod >> mSweepStep;
    
    uint16_t newPeriod = 0;
    if (mSweepDir)
        newPeriod = mSweepShadowPeriod - tmp;
    else 
        newPeriod = mSweepShadowPeriod + tmp;

    return newPeriod;
}

void SquareWaveChannel::trigger()
{
    // when a channel is triggered it becomes enabled and
    // it starts playing its waveform from the beginning
    mChEnabled = true;
    mSampleIdx = 0;

    // update the period counter with the current value
    mPeriodCounter = mPeriodL | (mPeriodH << 8);

    // reset envenlope stuff
    mVolume = mEnvInitialVol;
    mEnvPaceCounter = 0;

    // reset sweep stuff
    mSweepShadowPeriod = mPeriodL | (mPeriodH << 8);
    mSweepCounter = 0;
    mSweepEnabled = (mSweepPace > 0) || (mSweepStep > 0);


    // if the dac is off the channel is disabled again
    if (!mDacEnabled)
        mChEnabled = false;
}




// ------------------------------------------------------------------------------------------------
// NoiseChannel
// ------------------------------------------------------------------------------------------------

NoiseChannel::NoiseChannel()
    : AudioChannelIf(64)
{
    reset();
}

void NoiseChannel::reset()
{
    AudioChannelIf::reset();

    // Reg 1
    mLengthTimerInitialVal = 0;
    // Reg 2
    mEnvInitialVol = 0;
    mEnvDir = false;
    mEnvPace = 0;
    // Reg 3
    mClockDivider = 0;
    mLfsrWidthIs7 = false;
    mClockShift = 0;
    // Reg 4
    mTrigger = false;

    // counters and other internal stuff
    mVolume = 0;
    mLfsr.reset();

    // subticks
    mClockCounter = 0;

    // other functions counters
    mEnvPaceCounter = 0;
}

void NoiseChannel::writeReg1(uint8_t val)
{
    // bits 0-5 are the initial length timer
    // bits 6-7 are unused
    mLengthTimerInitialVal = val & 0x3F;
}

uint8_t NoiseChannel::readReg1() const
{
    // the register is write only
    return 0xff;
}

void NoiseChannel::writeReg2(uint8_t val)
{
    // same as the square wave channel 

    // bits 0-2 are the envelope pace
    // bit 3 is envelope direction
    // bits 4-7 are the initial volume
    mEnvPace = val & 0x07;
    mEnvDir = val & 0x08;
    mEnvInitialVol = (val >> 4) & 0x0f;

    // the channel dac is turned on when the top 5 bits are != 0
    // if the dac is turned off the channel is also turned off
    // if the dac is turned on, the channel stays off and it must be triggered to be turned on
    mDacEnabled = val & 0xF8;
    if (!mDacEnabled)
        mChEnabled = false;
}

uint8_t NoiseChannel::readReg2() const
{
    // same as the square wave channel 
    return (mEnvPace & 0x07) | (mEnvDir << 3) | (mEnvInitialVol << 4);
}

void NoiseChannel::writeReg3(uint8_t val)
{
    // bits 0-2 are the clock divider
    // bit 3 is the LFSR width
    // bits 4-7 are clock shift
    mClockDivider = val & 0x07;
    mLfsrWidthIs7 = val & 0x08;
    mClockShift = (val >> 4) & 0x0f;
}

uint8_t NoiseChannel::readReg3() const
{
    return (mClockDivider & 0x07) | (mLfsrWidthIs7 << 3) | (mClockShift << 4);
}

void NoiseChannel::writeReg4(uint8_t val)
{
    // bits 0-5 are unused
    // bit 6 enables the length timer
    // bit 7 triggers the channel
    bool lengthTimerEnable = val & 0x40;
    mTrigger = val & 0x80;

    if (mTrigger)
        trigger();
    if (lengthTimerEnable)
        enableLengthTimer(mLengthTimerInitialVal);
}

uint8_t NoiseChannel::readReg4() const
{
    // the only readable bit in this register is the length enable bit
    return 0xff & (isLengthTimerEnabled() << 6);
}


bool NoiseChannel::onStep()
{
    mClockCounter++;
    if (mClockCounter == mClockCounterTarget) {
        mClockCounter = 0;

        bool xored = mLfsr[0] ^ mLfsr[1] ^ 1;
        mLfsr >>= 1;
        mLfsr[15] = xored;
        
        if (mLfsrWidthIs7)
            mLfsr[6] = xored;

        // new value available
        return true;
    }

    return false;
}

uint8_t NoiseChannel::computeChannelOutput()
{
    // the output value depends on the first bit of the LFSR register:
    //  - if 0: the output is the current volume
    //  - if 1: the output is 0

    return mLfsr[0] ? 0 : mVolume;
}

void NoiseChannel::envelopeTick()
{
    // same as the square wave channel

        // every time the envelope clock ticks we adjust the volume
    // using the parameters set in reg 2
    // if the envelope pace is 0 the volume stays constant

    if (mEnvPace > 0) {
        mEnvPaceCounter++;
        if (mEnvPaceCounter == mEnvPace) {
            mEnvPaceCounter = 0;

            if (mEnvDir) {
                // volume must increase and 
                mVolume = std::min<uint8_t>(15, mVolume + 1);
            }
            else {
                // volume must decrease
                if (mVolume > 0)
                    mVolume--;
            }
        }
    }
}


void NoiseChannel::trigger()
{
    // when a channel is triggered it becomes enabled and
    // it starts playing noise
    mChEnabled = true;

    // reset the clock counter target using the clock divider and the shift values
    static constexpr uint8_t dividers[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };

    mClockCounterTarget = dividers[mClockDivider] << mClockShift;
    mClockCounter = 0;

    // reset envenlope stuff
    mVolume = mEnvInitialVol;
    mEnvPaceCounter = 0;


    // if the dac is off the channel is disabled again
    if (!mDacEnabled)
        mChEnabled = false;
}



// ------------------------------------------------------------------------------------------------
// UserWaveChannel
// ------------------------------------------------------------------------------------------------

UserWaveChannel::UserWaveChannel()
    : AudioChannelIf(256)
{
    reset();
}

void UserWaveChannel::reset()
{
    AudioChannelIf::reset();

    // Reg 1
    mLengthTimerInitialVal = 0;
    // Reg 2
    mOutputVolume = 0;
    // Reg 3
    mPeriodL = 0;
    // Reg 4
    mPeriodH = 0;
    mTrigger = false;


    std::fill(mWaveRam.begin(), mWaveRam.end(), 0);
    mWaveRamIdx = 0;

    mPeriodCounter = 0;
}

void UserWaveChannel::writeReg0(uint8_t val)
{
    // bits 0-6 are unused
    // bit 7 enables or disables the dac
    mDacEnabled = val & 0x80;

    // as soon as the dac is disabled the channel is disabled as well
    if (!mDacEnabled)
        mChEnabled = false;
}

uint8_t UserWaveChannel::readReg0() const
{
    return 0xff & (mDacEnabled << 1);
}

void UserWaveChannel::writeReg1(uint8_t val)
{
    // the initial length timer value is 8-bit wide for this channel
    mLengthTimerInitialVal = val;
}

uint8_t UserWaveChannel::readReg1() const
{
    // write-only
    return 0xff;
}

void UserWaveChannel::writeReg2(uint8_t val)
{
    // the only used bits are bits 5 and 6
    mOutputVolume = (val >> 5) & 0x03;
}

uint8_t UserWaveChannel::readReg2() const
{
    return 0xff & (mOutputVolume << 5);
}

void UserWaveChannel::writeReg4(uint8_t val)
{
    // bits 0-2 are the high bits of the period
    // bits 3-5 are unused
    // bit 6 enables the length timer
    // bit 7 triggers the channel
    mPeriodH = val & 0x07;
    bool lengthTimerEnable = val & 0x40;
    mTrigger = val & 0x80;

    if (mTrigger)
        trigger();
    if (lengthTimerEnable)
        enableLengthTimer(mLengthTimerInitialVal);
}

uint8_t UserWaveChannel::readReg4() const
{
    // the only readable bit in this register is the length enable bit
    return 0xff & (isLengthTimerEnabled() << 6);
}

void UserWaveChannel::writeWaveRam(uint16_t addr, uint8_t val)
{
    assert(addr >= mmap::regs::audio::wave_ram::start && addr <= mmap::regs::audio::wave_ram::end);

    addr -= mmap::regs::audio::wave_ram::start;

    // the ram is 16 bytes long = 32 4-bit samples
    // reading starts from the high nibble of byte 0, the its low nibble, then
    // the high nibble of byte 1, etc.

    uint8_t hi = val >> 4;
    uint8_t lo = val & 0x0f;

    mWaveRam[addr * 2] = hi;
    mWaveRam[addr * 2  + 1] = lo;
}

uint8_t UserWaveChannel::readWaveRam(uint16_t addr)
{
    assert(addr >= mmap::regs::audio::wave_ram::start && addr <= mmap::regs::audio::wave_ram::end);

    addr -= mmap::regs::audio::wave_ram::start;

    uint8_t hi = mWaveRam[addr * 2];
    uint8_t lo = mWaveRam[addr * 2 + 1];

    return lo | (hi << 4);
}


bool UserWaveChannel::onStep()
{
    // see https://gbdev.io/pandocs/Audio_Registers.html#ff1d--nr33-channel-3-period-low-write-only
    // for the explanation of why the period counter works like this
    // in comparison to the square wave channel, the wave is composed of 32 samples instead of 8
    // but this channel is clocked at double the frequency of the square wave channel
    // so, 4 times longer wavetable but twice faster clock
    // the result is that the frequency produced by this channel, given the same period value, is
    // half that of the square wave channel

    mPeriodCounter += 2; // double frequency
    if (mPeriodCounter >= 2048) {
        // reset the period counter using the values in the registers
        mPeriodCounter = mPeriodL | (mPeriodH << 8);

        // advance the sample index by 1 and wrap around when it reaches 32
        mWaveRamIdx = (mWaveRamIdx + 1) & 0x1F;

        // new output value available
        return true;
    }

    return false;
}

uint8_t UserWaveChannel::computeChannelOutput()
{
    // the channel output value is picked from wave ram
    // the ram is 16 bytes long = 32 4-bit samples
    // reading starts from the high nibble of byte 0, the its low nibble, then
    // the high nibble of byte 1, etc.

    auto sample = mWaveRam[mWaveRamIdx];

    // in this channel volume only has 4 levels:
    // 00   mute (shift sample value right by 4)
    // 01   100 % volume (use sample value as is)
    // 10   50 % volume (shift sample value right once)
    // 11   25 % volume (shift sample value right twice)

    uint8_t shift = 0;
    switch (mOutputVolume) {
    case 0: shift = 4; break;
    case 1: shift = 0; break;
    case 2: shift = 1; break;
    case 3: shift = 2; break;
    default:
        assert(false);
        break;
    }

    return sample >> shift;
}

void UserWaveChannel::trigger()
{
    // when a channel is triggered it becomes enabled and
    // it starts playing its waveform from the beginning, in this case 
    // the wave channel starts playing from sample 1 instead of 0 (idx will
    // be incremented one time before emitting the first sample)
    mChEnabled = true;
    mWaveRamIdx = 0;

    // update the period counter with the current value
    mPeriodCounter = mPeriodL | (mPeriodH << 8);

    // if the dac is off the channel is disabled again
    if (!mDacEnabled)
        mChEnabled = false;
}




#include "AudioChannel.h"
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

AudioChannelIf::AudioChannelIf(uint16_t lengthTimerMax, uint32_t downsamplingFreq)
    : mLengthTimerMax(lengthTimerMax)
    , mUseInternalFrameSequencer(false)
    , mDownsamplingFreq(downsamplingFreq)
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
    if (updateOut) 
        updateChannelOutput();


    // check downsampling, that is, check if a new sample is ready
    auto samplePeriod = duration_cast<nanoseconds>(1s) / mDownsamplingFreq;

    // the APU always runs at the base clock speed, even if the CGB is running at double clock speed
    mTimeCounter += GameBoy::machinePeriod;

    bool newSample = false;
    if (mTimeCounter >= samplePeriod) {
        mTimeCounter -= samplePeriod;
        newSample = true;
    }

    return newSample;
}

void AudioChannelIf::updateChannelOutput()
{
    if (mDacEnabled) {
        // when the dac is enabled the current output value depends on the channel status
        // if the channel is disabled the output is 0, otherwise the implementation
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


void AudioChannelIf::writeReg4(uint8_t val, uint8_t /*currentFrameSeqStep*/)
{
    // bits 0-2 are the high bits of the period on square and wave channels
    // bits 3-5 are unused
    // bit 6 enables the length timer
    // bit 7 triggers the channel

    bool triggered = val & 0x80;
    
    //bool wasLengthEnabled = mLengthTimerEnable;
    mLengthTimerEnable = val & 0x40;
    
    // allow the channels to read the period high bits
    onWriteReg4(val);

    // More obscure behavior, if:
    // - a write to reg 4 happens when the next frame of the apu frame sequencer is one that 
    //      DOESN'T trigger the length counter tick and (even steps)
    // - the length timer was previously disabled and now is enabled and
    // - length counter is not zero
    // then, one extra clock happens for the length timer (that is, it's decremented)
    // if the decrement makes the counter 0 and the channel is not triggered 
    // then the channel is disabled
    // see: https://gbdev.io/pandocs/Audio_details.html#obscure-behavior

    //bool nextFrameTicksLength = currentFrameSeqStep & 0x01;

    //if (!nextFrameTicksLength && !wasLengthEnabled && mLengthTimerEnable && mLengthTimerCounter > 0) {
    //    mLengthTimerCounter--;

    //    if (mLengthTimerCounter == 0 && !triggered) 
    //        mChEnabled = false;
    //}


    if (triggered) {
        // on a trigger event, if the length timer counter is elapsed it's automatically set to 
        // the maximum value
        if (mLengthTimerCounter == 0) {
            mLengthTimerCounter = mLengthTimerMax;
            
            /*if (mLengthTimerEnable && !nextFrameTicksLength)
                mLengthTimerCounter--;*/
        }

        // when a channel is triggered it becomes enabled and it starts playing
        // only if the DAC is enabled as well
        if(mDacEnabled)
            mChEnabled = true;

        // call the onTrigger function of the single channels
        onTrigger();
    }
}

uint8_t AudioChannelIf::readReg4() const
{
    // the only readable bit in this register is the length enable bit
    return 0xff & (mLengthTimerEnable << 6);
}

void AudioChannelIf::setLengthTimerCounter(uint16_t val) 
{
    // length timer counter value can be modified at any time
    // and is set at max - value
    // for the square and noise channels max is 64
    // for the wave channel max is 256

    mLengthTimerCounter = mLengthTimerMax - val; 
}

void AudioChannelIf::lengthTimerTick()
{
    // for every tick the length timer counter is decreased it by 1
    // when the counter reaches 0 channel is turned off
    
    // NOTES:
    //  - the length timer keeps ticking even if the channel is disabled
    //  - reaching 0 does not disable the counter (that is, the enable flag will still
    //      read as 1 when read from register 4 of the channel)
    //  - the current counter value can be changed at any time
    
    if (mLengthTimerEnable && mLengthTimerCounter > 0) {
        if (--mLengthTimerCounter == 0) {
            // the length timer elapsed, if length timer was enabled 
            // the channel must be turned off
            mChEnabled = false;
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

SquareWaveChannel::SquareWaveChannel(uint32_t downsamplingFreq)
    // hasSweep is only relevent for the emulator so it doesn't need to be 
    // modified by the reset() function
    : AudioChannelIf(64, downsamplingFreq)
    , mHasSweep(false)
{
    reset();
}

void SquareWaveChannel::reset()
{
    AudioChannelIf::reset();

    // Reg 0
    mSweepPace = 0;
    mSweepDir = SweepDir::Add;
    mSweepStep = 0;
    // Reg 1
    mDutyCycleIdx = 0;
    // Reg 2
    mEnvInitialVol = 0;
    mEnvDir = false;
    mEnvPace = 0;
    // Reg 3
    mPeriodL = 0;
    // Reg 4
    mPeriodH = 0;

    mSampleIdx = 0;
    mSampleBuf = 0;
    mVolume = 0;

    mPeriodCounter = 0;

    mEnvPaceCounter = 0;
    mSweepEnabled = false;
    mSweepShadowPeriod = 0;
    mSweepCounter = 0;
    mSweepSubtractionComputed = false;
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
    auto oldDir = mSweepDir;
    mSweepDir = val & 0x08 ? SweepDir::Sub : SweepDir::Add;
    mSweepPace = (val & 0x70) >> 4;

    // obscure behavior of the sweep functionality
    // when switching from subtraction to addition, if a sweep computation already
    // happened the channel turns off
    if (mSweepDir == SweepDir::Add && oldDir == SweepDir::Sub && mSweepSubtractionComputed) {
        mChEnabled = false;
    }
}

uint8_t SquareWaveChannel::readReg0() const
{
    if (!mHasSweep)
        return 0xff;

    // the top bit is unused so it reads as 1
    return 0x80 | (mSweepStep & 0x07) 
        | (static_cast<uint8_t>(mSweepDir) << 3) 
        | ((mSweepPace & 0x07) << 4);
}

void SquareWaveChannel::writeReg1(uint8_t val)
{
    // bits 0-5 are the initial length timer
    // bits 6-7 select the duty cycle
    setLengthTimerCounter(val & 0x3F);
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

void SquareWaveChannel::onWriteReg4(uint8_t val)
{
    // bits 0-2 are the high bits of the period
    mPeriodH = val & 0x07;
}


bool SquareWaveChannel::onStep()
{
    // audio is generated using the cpu main clock 
    // see https://gbdev.io/pandocs/Audio_Registers.html#ff13--nr13-channel-1-period-low-write-only
    // for the explanation of why the period counter works like this

    mPeriodCounter++;
    if (mPeriodCounter >= 2048) {
        // reset the period counter using the values in the registers
        mPeriodCounter = mPeriodL | (mPeriodH << 8);

        // advance the sample index by 1 and wrap around when in reaches 8
        mSampleIdx = (mSampleIdx + 1) & 0x07;
        // copy the sample from the currently selected waveform in the sample buffer 
        // which will be used to compute the output
        mSampleBuf = wavetables[mDutyCycleIdx][mSampleIdx];

        // new output value available
        return true;
    }

    return false;
}

uint8_t SquareWaveChannel::computeChannelOutput()
{
    // the output is computed using the current value of
    // the sample buffer multiplied by the current volume value
    
    return mSampleBuf * mVolume;
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
    if (!mHasSweep)
        return;

    // every time the sweep clock ticks we have to update the periods
    // the sweep functionality writes the new period value directly to the period registers
    // where it will be picked up by the period counter on its next loop

    mSweepCounter--;
    if (mSweepCounter == 0) {
        // for some reason if the sweep pace value is 0 the counter is reloaded with 8 instead
        // see: https://gbdev.io/pandocs/Audio_details.html#obscure-behavior
        mSweepCounter = mSweepPace == 0 ? 8 : mSweepPace;

        if (mSweepEnabled && mSweepPace > 0) {

            // the first overflow check is applied when sweeping is enabled, even if the channel is not 
            // actually sweeping because sweep pace is 0
            uint16_t newPeriod = sweepCompute();
            if (newPeriod > 0x7FF) {
                // new period overflow, disable the channel and return
                mChEnabled = false;
                mSweepEnabled = false;
                return;
            }

            // if the new frequency is ok then it is applied only if:
            // - sweep pace is != 0 AND 
            // - sweep step is != 0
            if (mSweepPace != 0 && mSweepStep != 0) {
                // write the values back to the registers and to the shadow period register
                mSweepShadowPeriod = newPeriod;
                mPeriodL = newPeriod & 0xff;
                mPeriodH = (newPeriod >> 8) & 0x07;

                // compute a new period again using the new value and perform the overflow check again
                // why check again? who knows
                // source: https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frequency_Sweep
                if (sweepCompute() > 0x7FF) {
                    mChEnabled = false;
                    mSweepEnabled = false;
                }
            }
        }
    }
}

uint16_t SquareWaveChannel::sweepCompute()
{
    uint16_t tmp = mSweepShadowPeriod >> mSweepStep;
    
    uint16_t newPeriod = 0;
    if (mSweepDir == SweepDir::Add) {
        newPeriod = mSweepShadowPeriod + tmp;
    }
    else {
        mSweepSubtractionComputed = true;
        newPeriod = mSweepShadowPeriod - tmp;
    }

    return newPeriod;
}

void SquareWaveChannel::onTrigger()
{
    // the sample idx is never reset, except when the apu is turned off and 
    // on again. on the other hand, the output sample is set to 0 when the channel
    // is triggered so it always outputs 0 when enabled
    // mSampleIdx = 0;
    mSampleBuf = 0;

    // update the period counter with the current value
    mPeriodCounter = mPeriodL | (mPeriodH << 8);

    // reset envenlope stuff
    mVolume = mEnvInitialVol;
    mEnvPaceCounter = 0;

    // reset sweep stuff
    mSweepShadowPeriod = mPeriodL | (mPeriodH << 8);
    // for some reason if the sweep pace value is 0 the counter is reloaded with 8 instead
    // see: https://gbdev.io/pandocs/Audio_details.html#obscure-behavior
    mSweepCounter = mSweepPace == 0 ? 8 : mSweepPace;
    mSweepEnabled = (mSweepPace > 0) || (mSweepStep > 0);
    mSweepSubtractionComputed = false;

    // if sweep step is != 0 the sweep computation and overflow check are performed immediately
    if (mSweepStep != 0) {
        if (sweepCompute() > 0x7FF) {
            // new period overflow, disable the channel
            mChEnabled = false;
            mSweepEnabled = false;
        }
    }
}




// ------------------------------------------------------------------------------------------------
// NoiseChannel
// ------------------------------------------------------------------------------------------------

NoiseChannel::NoiseChannel(uint32_t downsamplingFreq)
    : AudioChannelIf(64, downsamplingFreq)
{
    reset();
}

void NoiseChannel::reset()
{
    AudioChannelIf::reset();

    // Reg 2
    mEnvInitialVol = 0;
    mEnvDir = false;
    mEnvPace = 0;
    // Reg 3
    mClockDivider = 0;
    mLfsrWidthIs7 = false;
    mClockShift = 0;
    
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
    setLengthTimerCounter(val & 0x3F);
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


bool NoiseChannel::onStep()
{
    mClockCounter++;
    if (mClockCounter == mClockCounterTarget) {
        mClockCounter = 0;

        bool xored = mLfsr[0] ^ mLfsr[1] ^ 1;
        mLfsr >>= 1;
        mLfsr[14] = xored;
        
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


void NoiseChannel::onTrigger()
{
    // reset the clock counter target using the clock divider and the shift values
    /*static constexpr uint8_t dividers[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };*/
    static constexpr uint8_t dividers[8] = { 4, 8, 16, 24, 32, 40, 48, 56 };

    mClockCounterTarget = dividers[mClockDivider] << mClockShift;
    mClockCounter = 0;

    // reset envenlope stuff
    mVolume = mEnvInitialVol;
    mEnvPaceCounter = 0;
}



// ------------------------------------------------------------------------------------------------
// UserWaveChannel
// ------------------------------------------------------------------------------------------------

UserWaveChannel::UserWaveChannel(uint32_t downsamplingFreq)
    : AudioChannelIf(256, downsamplingFreq)
{
    reset();
    resetWaveRam();
}

void UserWaveChannel::reset()
{
    AudioChannelIf::reset();

    // Reg 2
    mOutputVolume = 0;
    // Reg 3
    mPeriodL = 0;
    // Reg 4
    mPeriodH = 0;
    
    // wave ram is not affected by a reset of the channel
    // but the index and the sample buf are
    mWaveRamIdx = 0;
    mWaveRamSampleBuf = 0;

    mPeriodCounter = 0;
}

void UserWaveChannel::resetWaveRam()
{
    std::fill(mWaveRam.begin(), mWaveRam.end(), 0);
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
    return 0xff & (mDacEnabled << 7);
}

void UserWaveChannel::writeReg1(uint8_t val)
{
    // the initial length timer value is 8-bit wide for this channel
    setLengthTimerCounter(val);
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

void UserWaveChannel::onWriteReg4(uint8_t val)
{
    // bits 0-2 are the high bits of the period
    mPeriodH = val & 0x07;
}


void UserWaveChannel::writeWaveRam(uint16_t addr, uint8_t val)
{
    assert(addr >= mmap::regs::audio::wave_ram::start && addr <= mmap::regs::audio::wave_ram::end);

    addr -= mmap::regs::audio::wave_ram::start;

    // the ram is 16 bytes long = 32 4-bit samples
    // reading starts from the high nibble of byte 0, then its low nibble, then
    // the high nibble of byte 1, etc.
    
    uint8_t hi = val >> 4;
    uint8_t lo = val & 0x0f;

    if (mChEnabled) {
        // when the channel is enabled, writes should not be possible

        // TODO: due to bugs it's actually possible to write wave ram but those are not
        // implemented in this case
    }
    else {
        // when the channel is disabled, wave ram can be accessed freely as 
        // any other ram location
        mWaveRam[addr * 2] = hi;
        mWaveRam[addr * 2 + 1] = lo;
    }
}

uint8_t UserWaveChannel::readWaveRam(uint16_t addr) const
{
    assert(addr >= mmap::regs::audio::wave_ram::start && addr <= mmap::regs::audio::wave_ram::end);

    addr -= mmap::regs::audio::wave_ram::start;

    if (mChEnabled) {
        // when the channel is enabled, reading wave ram is not possible and reads return 0xFF

        // TODO: due to bugs it's actually possible to read wave ram but those are not
        // implemented in this case
        return 0xff;
    }
    else {
        uint8_t hi = mWaveRam[addr * 2];
        uint8_t lo = mWaveRam[addr * 2 + 1];

        return lo | (hi << 4);
    }
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

        // update the internal sample buffer ONLY when the index is advanced by 1
        // the channel output value is picked from wave ram
        // the ram is 16 bytes long = 32 4-bit samples
        // reading starts from the high nibble of byte 0, then its low nibble, then
        // the high nibble of byte 1, etc.
        mWaveRamSampleBuf = mWaveRam[mWaveRamIdx];

        // new output value available
        return true;
    }

    return false;
}

uint8_t UserWaveChannel::computeChannelOutput()
{
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

    return mWaveRamSampleBuf >> shift;
}

void UserWaveChannel::onTrigger()
{
    // when a channel is triggered it starts playing its waveform from the beginning,
    // in this case the wave channel starts playing from sample 1 instead of 0 
    // (idx will be incremented one time before emitting the first sample)
    mWaveRamIdx = 0;

    // the sample value used for the output (stored in mWaveRamSampleBuf)
    // IS NOT RESET and maintains the value it had! it's only reset to 0 
    // when the entire channel is reset!

    // update the period counter with the current value
    mPeriodCounter = mPeriodL | (mPeriodH << 8);
}



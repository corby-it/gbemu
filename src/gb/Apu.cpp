

#include "Apu.h"
#include "GameBoyCore.h"
#include <cstring>
#include <algorithm>
#include <cassert>
#define _USE_MATH_DEFINES
#include <math.h>


using namespace std::chrono;
namespace areg = mmap::regs::audio;






// ------------------------------------------------------------------------------------------------
// ApuHpfFilter
// ------------------------------------------------------------------------------------------------

ApuHpfFilter::ApuHpfFilter()
    : mB0(1), mB1(0), mB2(0)
    , mA1(0), mA2(0)
    , mYz1(0), mYz2(0)
    , mXz1(0), mXz2(0)
{}

void ApuHpfFilter::setCutoff(float fc, float fs)
{
    // coeffs for a 2nd order biquad high pass filter
    // formulas found somewhere on the internet: https://www.earlevel.com/main/2021/09/02/biquad-calculator-v3/

    static const float Q = 1.f / sqrtf(2); // 0.7071 to avoid peaking 
    
    float K = tan((float)M_PI * fc / fs);
    float norm = 1 / (1 + K / Q + K * K);

    mB0 = 1 * norm;
    mB1 = -2 * mB0;
    mB2 = mB0;
    mA1 = 2 * (K * K - 1) * norm;
    mA2 = (1 - K / Q + K * K) * norm;
}


float ApuHpfFilter::process(float x0)
{
    float y0 = mB0 * x0 + mB1 * mXz1 + mB2 * mXz2 - mA1 * mYz1 - mA2 * mYz2;

    mYz2 = mYz1;
    mYz1 = y0;

    mXz2 = mXz1;
    mXz1 = x0;

    return y0;
}




// ------------------------------------------------------------------------------------------------
// APU
// ------------------------------------------------------------------------------------------------

APU::APU(uint32_t downsamplingFreq)
    : mDownsamplingFreq(downsamplingFreq)
    , mChannels{ &square1, &square2, &wave, &noise }
    , mEnableHpf(true)
{
    // set cutoff frequency for the HPFs to 30 HZ
    mHpfR.setCutoff(30.f, (float)downsamplingFreq);
    mHpfL.setCutoff(30.f, (float)downsamplingFreq);

    // enable sweep modulation for ch1 (ch2 doesn't have it)
    square1.enableSweepModulation(true);

    // a single frame sequencer in this class will trigger the channels
    for (auto ch : mChannels)
        ch->enableInternalFrameSequencer(false);

    reset();
}

void APU::reset()
{
    mFrameSeq.reset();

    for (auto ch : mChannels)
        ch->reset();

    wave.resetWaveRam();

    mVinL = false;
    mVinR = false;
    mVolL = 0;
    mVolR = 0;

    std::fill(mChPanL.begin(), mChPanL.end(), false);
    std::fill(mChPanR.begin(), mChPanR.end(), false);
    
    mApuEnabled = false;

    mOutL = 0.f;
    mOutR = 0.f;

    mTimeCounter = 0ns;
}

uint8_t APU::read(uint16_t addr) const
{
    assert(addr >= areg::start && addr <= areg::end);

    if (addr >= areg::wave_ram::start && addr <= areg::wave_ram::end) {
        return wave.readWaveRam(addr);
    }
    else {
        // some bits are always set to 1 when read back
        switch (addr) {
        case areg::NR10: return square1.readReg0() | 0x80; break;
        case areg::NR11: return square1.readReg1() | 0x3F; break;
        case areg::NR12: return square1.readReg2() | 0x00; break;
        case areg::NR13: return square1.readReg3() | 0xFF; break;
        case areg::NR14: return square1.readReg4() | 0xBF; break;

        case areg::NR21: return square2.readReg1() | 0x3F; break;
        case areg::NR22: return square2.readReg2() | 0x00; break;
        case areg::NR23: return square2.readReg3() | 0xFF; break;
        case areg::NR24: return square2.readReg4() | 0xBF; break;

        case areg::NR30: return wave.readReg0() | 0x7F; break;
        case areg::NR31: return wave.readReg1() | 0xFF; break;
        case areg::NR32: return wave.readReg2() | 0x9F; break;
        case areg::NR33: return wave.readReg3() | 0xFF; break;
        case areg::NR34: return wave.readReg4() | 0xBF; break;

        case areg::NR41: return noise.readReg1() | 0xFF; break;
        case areg::NR42: return noise.readReg2() | 0x00; break;
        case areg::NR43: return noise.readReg3() | 0x00; break;
        case areg::NR44: return noise.readReg4() | 0xBF; break;

        case areg::NR50: return readReg0() | 0x00; break;
        case areg::NR51: return readReg1() | 0x00; break;
        case areg::NR52: return readReg2() | 0x70; break;

        default:
            // not all addresses in the audio range are associated with a register
            return 0xff;
        }
    }
}

void APU::write(uint16_t addr, uint8_t val)
{
    assert(addr >= areg::start && addr <= areg::end);

    if (addr >= areg::wave_ram::start && addr <= areg::wave_ram::end) {
        wave.writeWaveRam(addr, val);
    }
    else {
        if (mApuEnabled) {
            switch (addr) {
            case areg::NR10: square1.writeReg0(val); break;
            case areg::NR11: square1.writeReg1(val); break;
            case areg::NR12: square1.writeReg2(val); break;
            case areg::NR13: square1.writeReg3(val); break;
            case areg::NR14: square1.writeReg4(val); break;

            case areg::NR21: square2.writeReg1(val); break;
            case areg::NR22: square2.writeReg2(val); break;
            case areg::NR23: square2.writeReg3(val); break;
            case areg::NR24: square2.writeReg4(val); break;

            case areg::NR30: wave.writeReg0(val); break;
            case areg::NR31: wave.writeReg1(val); break;
            case areg::NR32: wave.writeReg2(val); break;
            case areg::NR33: wave.writeReg3(val); break;
            case areg::NR34: wave.writeReg4(val); break;

            case areg::NR41: noise.writeReg1(val); break;
            case areg::NR42: noise.writeReg2(val); break;
            case areg::NR43: noise.writeReg3(val); break;
            case areg::NR44: noise.writeReg4(val); break;

            case areg::NR50: writeReg0(val); break;
            case areg::NR51: writeReg1(val); break;
            case areg::NR52: writeReg2(val); break;

            default:
                // not all addresses in the audio range are associated with a register
                break;
            }
        }
        else {
            // when the apu is not enabled only register NR52 is writable
            if (addr == areg::NR52)
                writeReg2(val);
        }
    }
}


void APU::writeReg0(uint8_t val)
{
    // bits 0-2 are the right volume
    // bit 3 is Vin right panning
    // bits 4-6 are the left volume
    // bit 7 is Vin left panning
    mVolR = val & 0x07;
    mVinR = val & 0x08;
    mVolL = (val & 0x70) >> 4;
    mVinL = val & 0x80;
}

uint8_t APU::readReg0() const
{
    // all bits are readable
    return (mVolR & 0x07)
        | (mVinR << 3)
        | ((mVolL & 0x07) << 4) 
        | (mVinL << 7);
}

void APU::writeReg1(uint8_t val)
{
    // bits 0-3 are for panning right
    mChPanR[0] = val & 0x01;
    mChPanR[1] = val & 0x02;
    mChPanR[2] = val & 0x04;
    mChPanR[3] = val & 0x08;
    
    // bits 4-7 are for panning right
    mChPanL[0] = val & 0x10;
    mChPanL[1] = val & 0x20;
    mChPanL[2] = val & 0x40;
    mChPanL[3] = val & 0x80;
}

uint8_t APU::readReg1() const
{
    return (mChPanR[0] << 0)
        | (mChPanR[1] << 1)
        | (mChPanR[2] << 2)
        | (mChPanR[3] << 3)
        | (mChPanL[0] << 4)
        | (mChPanL[1] << 5)
        | (mChPanL[2] << 6)
        | (mChPanL[3] << 7);
}

void APU::writeReg2(uint8_t val)
{
    // only bit 7 is writable (apu on/off)
    mApuEnabled = val & 0x80;

    if (!mApuEnabled) {
        // when the APU is turned off we reset all channels, reset all registers
        // and make all the registers (except for NR52) read-only
        // wave ram is unaffected
        for (auto ch : mChannels)
            ch->reset();

        writeReg0(0);
        writeReg1(0);
    }
}

uint8_t APU::readReg2() const
{
    // bits 0, 1, 2 and 3 contain the status of the channels
    return (mApuEnabled << 7)
        | (noise.isChEnabled() << 3)
        | (wave.isChEnabled() << 2)
        | (square2.isChEnabled() << 1)
        | (square1.isChEnabled() << 0);
}


bool APU::step(uint32_t mCycles)
{
    // for each cpu cycle we:
    // - run the onStep() function of each channel
    // - run the frame sequencer
    // - compute the output for each channel
    // - check the downsampling interval 
    // if the APU is disabled nothing happens

    bool newSample = false;

    while (mCycles--) {

        if (mApuEnabled) {
            std::array<bool, chCount> outputReady;

            // run the channels
            for (uint32_t i = 0; i < chCount; ++i)
                outputReady[i] = mChannels[i]->onStep();

            // run the frame sequencer
            auto frameSeqEvt = mFrameSeq.step();

            for (auto ch : mChannels) {
                switch (frameSeqEvt) {
                case FrameSequencer::Event::LengthTimer:
                    ch->lengthTimerTick();
                    break;
                case FrameSequencer::Event::LengthTimerAndSweep:
                    ch->lengthTimerTick();
                    ch->sweepTick();
                    break;
                case FrameSequencer::Event::Envelope:
                    ch->envelopeTick();
                    break;
                default:
                    break;
                }
            }

            // update outputs if necessary
            for (uint32_t i = 0; i < chCount; ++i) {
                if (outputReady[i])
                    mChannels[i]->updateChannelOutput();
            }
        }

        // check downsampling, that is, check if we must produce a new sample for the emulation
        auto samplePeriod = duration_cast<nanoseconds>(1s) / mDownsamplingFreq;

        mTimeCounter += GameBoyClassic::machinePeriod;

        if (mTimeCounter >= samplePeriod) {
            mTimeCounter -= samplePeriod;
            mix();
            newSample = true;
        }
    }

    return newSample;
}

void APU::mix()
{
    // mix the output of the channels into a single stereo sample

    // first turn the output of each channel into a float and scale it between 0 and 1
    std::array<float, chCount> chOutputs;
    for (uint32_t i = 0; i < chCount; ++i)
        chOutputs[i] = static_cast<float>(mChannels[i]->getOutput()) / 15.f;

    // apply panning, if a channel is selected for a side, sum the channel output and 
    // scale again between 0 and 1
    float sampleR = 0.f;
    float sampleL = 0.f;
    
    for (uint32_t i = 0; i < chCount; ++i) {
        sampleR += mChPanR[i] ? chOutputs[i] : 0.f;
        sampleL += mChPanL[i] ? chOutputs[i] : 0.f;
    }

    sampleR /= static_cast<float>(chCount);
    sampleL /= static_cast<float>(chCount);

    // scale volumes between 0 and 1 and apply to the samples 
    // it's not possible to set the volumes to 0, as if 1 is always added to 
    // the values written in the NR50 register
    mOutR = sampleR * static_cast<float>(mVolR + 1) / 8.f;
    mOutL = sampleL * static_cast<float>(mVolL + 1) / 8.f;

    // apply the HPF to the outputs
    if (mEnableHpf) {
        mOutR = mHpfR.process(mOutR);
        mOutL = mHpfL.process(mOutL);
    }

    if(mSampleCallback)
        mSampleCallback(mOutL, mOutR);
}



#include "Audio.h"
#include "GameBoyCore.h"
#include <cstring>
#define _USE_MATH_DEFINES
#include <math.h>


using namespace std::chrono;

static float wtblSquare50[512];
static float wtblSine[512];


Audio::Audio()
    : mRdh(0)
    , mTimeCounter(0ns)
{
    ma_pcm_rb_init(ma_format_f32, 2, 48000, nullptr, nullptr, &audioBuf);

    // populate the wave tables
    for (uint32_t i = 0; i < 256; ++i)
        wtblSquare50[i] = 1;
    for (uint32_t i = 256; i < 512; ++i)
        wtblSquare50[i] = 0;

    for (uint32_t i = 0; i < 512; ++i) {
        float t = i / 512.f;
        wtblSine[i] = sinf(2 * (float)M_PI * t);
    }

    reset();
}

Audio::~Audio()
{
    ma_pcm_rb_uninit(&audioBuf);
}

void Audio::reset()
{
    mRdh = 0;
    mTimeCounter = 0ns;

    // reset the audio buffer
    ma_pcm_rb_reset(&audioBuf);

    // initial values from https://gbdev.gg8.se/wiki/articles/Power_Up_Sequence
    write(0xFF10, 0x80); // NR10
    write(0xFF11, 0xBF); // NR11
    write(0xFF12, 0xF3); // NR12
    write(0xFF14, 0xBF); // NR14
    write(0xFF16, 0x3F); // NR21
    write(0xFF17, 0x00); // NR22
    write(0xFF19, 0xBF); // NR24
    write(0xFF1A, 0x7F); // NR30
    write(0xFF1B, 0xFF); // NR31
    write(0xFF1C, 0x9F); // NR32
    write(0xFF1E, 0xBF); // NR33
    write(0xFF20, 0xFF); // NR41
    write(0xFF21, 0x00); // NR42
    write(0xFF22, 0x00); // NR43
    write(0xFF23, 0xBF); // NR44
    write(0xFF24, 0x77); // NR50
    write(0xFF25, 0xF3); // NR51
    write(0xFF26, 0xF1); // NR52
}


void Audio::step(uint32_t mCycles)
{
    static constexpr auto samplePeriod = duration_cast<nanoseconds>(1s) / 48000;

    while (mCycles--) {

        mTimeCounter += GameBoyClassic::machinePeriod;

        if (mTimeCounter >= samplePeriod) {
            mTimeCounter -= samplePeriod;
            
            // write a sample in the buffer every 21 machine cycles
            
            void* pvBuf;
            uint32_t frames = 1;
            ma_pcm_rb_acquire_write(&audioBuf, &frames, &pvBuf);

            auto pfBuf = static_cast<float*>(pvBuf);

            for (uint32_t i = 0; i < frames; ++i) {
                *pfBuf = wtblSine[mRdh];
                ++pfBuf;
                *pfBuf = wtblSine[mRdh];
                ++pfBuf;

                // wrap after 512 bytes
                mRdh = (mRdh + 1) & 0x000001FF;
            }

            ma_pcm_rb_commit_write(&audioBuf, frames);
        }
    }
}

uint8_t Audio::read(uint16_t addr)
{
    if(addr < mmap::regs::audio::start || addr > mmap::regs::audio::end)
        return 0;

    return mData[addr - mmap::regs::audio::start];
}

void Audio::write(uint16_t addr, uint8_t val)
{
    if (addr < mmap::regs::audio::start || addr > mmap::regs::audio::end)
        return;
   
    mData[addr - mmap::regs::audio::start] = val;
}

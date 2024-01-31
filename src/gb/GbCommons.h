
#ifndef GBEMU_SRC_GB_GBCOMMONS_H_
#define GBEMU_SRC_GB_GBCOMMONS_H_

#include <cstdint>
#include <array>
#include <memory>


// the gb cpu actually runs at 4.194304 MHz but, since we are not counting actual clock
// cycles but machine cycles (clock cycles / 4) we have to use the clock frequency
// divided by 4
static constexpr uint32_t clockFreq = 4194304;
static constexpr uint32_t machineFreq = 1048576;



// Memory map of the gameboy address bus:
// - https://gbdev.io/pandocs/Memory_Map.html#io-ranges

#define MMAP_T  static constexpr uint16_t

namespace mmap {
    namespace rom {
        MMAP_T start = 0x0000;
        MMAP_T end = 0x7FFF;
    }
    namespace vram {
        MMAP_T start = 0x8000;
        MMAP_T end = 0x9FFF;
    }
    namespace xram {
        MMAP_T start = 0xA000;
        MMAP_T end = 0xBFFF;
    }
    namespace wram {
        MMAP_T start = 0xC000;
        MMAP_T end = 0xDFFF;
    }
    namespace echoram {
        MMAP_T start = 0xE000;
        MMAP_T end = 0xFDFF;
    }
    namespace oam {
        MMAP_T start = 0xFE00;
        MMAP_T end = 0xFE9F;
    }
    namespace regs {
        MMAP_T start = 0xFF00;

        MMAP_T joypad = 0xFF00;

        namespace timer {
            MMAP_T start = 0xFF04;
            MMAP_T DIV = 0xFF04;
            MMAP_T TIMA = 0xFF05;
            MMAP_T TMA = 0xFF06;
            MMAP_T TAC = 0xFF07;
            MMAP_T end = 0xFF07;
        }

        MMAP_T IF = 0xFF0F;

        MMAP_T end = 0xFF7F;
    }
    namespace hram {
        MMAP_T start = 0xFF80;
        MMAP_T end = 0xFFFE;
    }

    MMAP_T IE = 0xFFFF;
}




// Generic register class with helper methods
struct RegU8 {
public:
    virtual ~RegU8() {}

    virtual uint8_t asU8() const = 0;

    virtual void fromU8(uint8_t val) = 0;

    virtual operator uint8_t() const { return asU8(); }

    virtual bool operator==(const RegU8& other) const
    {
        return asU8() == other.asU8();
    }
};



// Generic classes for matrix-like objects



struct RgbPixel {
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

static constexpr RgbPixel black = { 0, 0, 0 };
static constexpr RgbPixel darkGrey = { 120, 120, 120 };
static constexpr RgbPixel lightGrey = { 200, 200, 200 };
static constexpr RgbPixel white = { 255, 255, 255 };


class RgbPixelRef {
public:
    RgbPixelRef(uint8_t* ptr)
        : mPtr(ptr)
    {}

    uint8_t& R() { return mPtr[0]; }
    uint8_t& G() { return mPtr[1]; }
    uint8_t& B() { return mPtr[2]; }

    RgbPixelRef& operator=(RgbPixel pix) {
        mPtr[0] = pix.R;
        mPtr[1] = pix.G;
        mPtr[2] = pix.B;
        return *this;
    }

private:
    uint8_t* mPtr;

};


class RgbBuffer {
public:
    RgbBuffer(uint32_t w, uint32_t h)
        : mWidth(w)
        , mHeight(h)
        , mSize(w * h * 3)
        , mData(std::make_unique<uint8_t[]>(mSize))
    {}

    uint8_t* ptr() { return mData.get(); }
    size_t size() const { return mSize; }

    RgbPixelRef operator()(uint32_t x, uint32_t y)
    {
        x %= mWidth;
        y %= mHeight;
        return RgbPixelRef(ptr() + (y * mWidth * 3) + (x * 3));
    }

private:
    const uint32_t mWidth;
    const uint32_t mHeight;
    const size_t mSize;
    std::unique_ptr<uint8_t[]> mData;
};


class Matrix {
public:
    Matrix(uint32_t w, uint32_t h)
        : mWidth(w)
        , mHeight(h)
    {}

    virtual ~Matrix() {}

    uint8_t get(uint32_t x, uint32_t y) const {
        assert(x < mWidth);
        assert(y < mHeight);

        return getImpl(x, y);
    }

    void set(uint32_t x, uint32_t y, uint8_t val) {
        assert(x < mWidth);
        assert(y < mHeight);

        setImpl(x, y, val);
    }

    uint32_t width() const { return mWidth; }
    uint32_t height() const { return mHeight; }


    virtual void fillRgbBuffer(RgbBuffer& buf) const
    {
        for (uint32_t y = 0; y < mHeight; ++y) {
            for (uint32_t x = 0; x < mWidth; ++x) {
                switch (get(x, y)) {
                default:
                case 0: buf(x, y) = white; break;
                case 1: buf(x, y) = lightGrey; break;
                case 2: buf(x, y) = darkGrey; break;
                case 3: buf(x, y) = black; break;
                }
            }
        }
    }
    

protected:
    virtual uint8_t getImpl(uint32_t x, uint32_t y) const = 0;
    virtual void setImpl(uint32_t x, uint32_t y, uint8_t val) = 0;


    const uint32_t mWidth;
    const uint32_t mHeight;

};

#endif // GBEMU_SRC_GB_GBCOMMONS_H_
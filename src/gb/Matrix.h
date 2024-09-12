

#ifndef GBEMU_SRC_GB_MATRIX_H_
#define GBEMU_SRC_GB_MATRIX_H_

#include <cstdint>
#include <memory>
#include <cassert>


// Generic classes for matrix-like objects



struct RgbaPixel {
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t A;
};



static constexpr RgbaPixel blackA = { 0, 0, 0, 255 };
static constexpr RgbaPixel darkGreyA = { 120, 120, 120, 255 };
static constexpr RgbaPixel lightGreyA = { 200, 200, 200, 255 };
static constexpr RgbaPixel whiteA = { 255, 255, 255, 255 };


class RgbaPixelRef {
public:
    RgbaPixelRef(uint8_t* ptr)
        : mPtr(ptr)
    {}

    uint8_t& R() { return mPtr[0]; }
    uint8_t& G() { return mPtr[1]; }
    uint8_t& B() { return mPtr[2]; }
    uint8_t& A() { return mPtr[3]; }

    RgbaPixelRef& operator=(RgbaPixel pix) {
        mPtr[0] = pix.R;
        mPtr[1] = pix.G;
        mPtr[2] = pix.B;
        mPtr[3] = pix.A;
        return *this;
    }

private:
    uint8_t* mPtr;

};



class RgbaBufferIf {
public:
    RgbaBufferIf(uint32_t w, uint32_t h)
        : mWidth(w)
        , mHeight(h)
        , mSize(w* h * 4)
    {}

    virtual ~RgbaBufferIf() {}

    virtual uint8_t* ptr() = 0;

    size_t size() const { return mSize; }

    RgbaPixelRef operator()(uint32_t x, uint32_t y)
    {
        x %= mWidth;
        y %= mHeight;
        return RgbaPixelRef(ptr() + (y * mWidth * 4) + (x * 4));
    }

protected:
    const uint32_t mWidth;
    const uint32_t mHeight;
    const size_t mSize;
};



class RgbaBuffer : public RgbaBufferIf {
public:
    RgbaBuffer(uint32_t w, uint32_t h)
        : RgbaBufferIf(w, h)
        , mData(std::make_unique<uint8_t[]>(mSize))
    {}

    uint8_t* ptr() override { return mData.get(); }

private:
    std::unique_ptr<uint8_t[]> mData;

};


template<size_t W, size_t H>
class RgbaBufferArray : public RgbaBufferIf {
public:
    RgbaBufferArray()
        : RgbaBufferIf(W, H)
    {}

    uint8_t* ptr() override { return mData.data(); }

private:
    std::array<uint8_t, W* H * 4> mData;

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


    virtual void fillRgbaBuffer(RgbaBufferIf& buf) const
    {
        for (uint32_t y = 0; y < mHeight; ++y) {
            for (uint32_t x = 0; x < mWidth; ++x) {
                switch (get(x, y)) {
                default:
                case 0: buf(x, y) = whiteA; break;
                case 1: buf(x, y) = lightGreyA; break;
                case 2: buf(x, y) = darkGreyA; break;
                case 3: buf(x, y) = blackA; break;
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


#endif // GBEMU_SRC_GB_MATRIX_H_

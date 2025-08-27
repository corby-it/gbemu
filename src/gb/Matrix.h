

#ifndef GBEMU_SRC_GB_MATRIX_H_
#define GBEMU_SRC_GB_MATRIX_H_

#include <cstdint>
#include <memory>
#include <functional>
#include <cassert>
#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>


// Generic classes for matrix-like objects



struct RgbaPixel {
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t A;

    RgbaPixel()
        : R(0), G(0), B(0), A(0)
    {}

    RgbaPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : R(r), G(g), B(b), A(a)
    {}

    RgbaPixel(const uint8_t* ptr) {
        R = ptr ? ptr[0] : 0;
        G = ptr ? ptr[1] : 0;
        B = ptr ? ptr[2] : 0;
        A = ptr ? ptr[3] : 0;
    }

    uint32_t asU32() const {
        return (R << 24) | (G << 16) | (B << 8) | A;
    }

    bool operator==(const RgbaPixel& other) const {
        return R == other.R && G == other.G
            && B == other.B && A == other.A;
    }

    bool operator!=(const RgbaPixel& other) const {
        return !(*this == other);
    }
};



static const auto blackA = RgbaPixel(0, 0, 0);
static const auto darkGreyA = RgbaPixel(120, 120, 120);
static const auto lightGreyA = RgbaPixel(200, 200, 200);
static const auto whiteA = RgbaPixel(255, 255, 255);


static inline RgbaPixel dmgVal2RGB(uint8_t val)
{
    switch (val) {
    default:
    case 0: return whiteA;
    case 1: return lightGreyA;
    case 2: return darkGreyA;
    case 3: return blackA;
    }
}



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

    bool operator==(const RgbaPixel& other) const {
        return mPtr[0] == other.R && mPtr[1] == other.G
            && mPtr[2] == other.B && mPtr[3] == other.A;
    }

    bool operator!=(const RgbaPixel& other) const {
        return !(*this == other);
    }


private:
    uint8_t* mPtr;

};



class RgbaBufferIf {
public:
    RgbaBufferIf(uint32_t w, uint32_t h)
        : mWidth(w)
        , mHeight(h)
        , mSize(w * h * 4)
    {}

    virtual ~RgbaBufferIf() {}

    virtual uint8_t* ptr() = 0;
    virtual const uint8_t* ptr() const = 0;

    size_t size() const { return mSize; }
    uint32_t w() const { return mWidth; }
    uint32_t h() const { return mHeight; }

    RgbaPixelRef operator()(uint32_t x, uint32_t y)
    {
        x %= mWidth;
        y %= mHeight;
        return RgbaPixelRef(pixelPtr(x,y));
    }

    RgbaPixel operator()(uint32_t x, uint32_t y) const
    {
        x %= mWidth;
        y %= mHeight;
        return RgbaPixel(pixelPtr(x, y));
    }

    void fill(RgbaPixel pix)
    {
        for (uint32_t y = 0; y < mHeight; ++y) {
            for (uint32_t x = 0; x < mWidth; ++x) {
                (*this)(x, y) = pix;
            }
        }
    }

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(mWidth, mHeight, mSize);
    }

protected:

    uint8_t* pixelPtr(uint32_t x, uint32_t y) {
        return ptr() + (y * mWidth * 4) + (x * 4);
    }

    const uint8_t* pixelPtr(uint32_t x, uint32_t y) const {
        return ptr() + (y * mWidth * 4) + (x * 4);
    }
    


    uint32_t mWidth;
    uint32_t mHeight;
    size_t mSize;
};

CEREAL_CLASS_VERSION(RgbaBufferIf, 1);






class RgbaBuffer : public RgbaBufferIf {
public:
    RgbaBuffer(uint32_t w, uint32_t h)
        : RgbaBufferIf(w, h)
        , mData(std::make_unique<uint8_t[]>(mSize))
    {}

    RgbaBuffer(RgbaBuffer& other)
        : RgbaBufferIf(other)
        , mData(std::make_unique<uint8_t[]>(mSize))
    {
        memcpy(mData.get(), other.mData.get(), other.mSize);
    }

    RgbaBuffer& operator=(RgbaBuffer& other) {
        mData.reset(new uint8_t[other.mSize]);
        memcpy(mData.get(), other.mData.get(), other.mSize);

        return *this;
    }

    uint8_t* ptr() override { return mData.get(); }
    const uint8_t* ptr() const override { return mData.get(); }



    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/) {
        ar(cereal::base_class<RgbaBufferIf>(this));
        ar(cereal::binary_data(mData.get(), mSize));
    }


private:
    std::unique_ptr<uint8_t[]> mData;

};

CEREAL_CLASS_VERSION(RgbaBuffer, 1);





template<size_t W, size_t H>
class RgbaBufferArray : public RgbaBufferIf {
public:
    RgbaBufferArray()
        : RgbaBufferIf(W, H)
        , mData()
    {}

    uint8_t* ptr() override { return mData.data(); }
    const uint8_t* ptr() const override { return mData.data(); }

private:
    std::array<uint8_t, W* H * 4> mData;

};





typedef std::function<RgbaPixel(uint8_t)>   ValToColorFn;


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


    virtual void fillRgbaBuffer(RgbaBufferIf& buf, ValToColorFn convFn = dmgVal2RGB) const
    {
        for (uint32_t y = 0; y < mHeight; ++y) {
            for (uint32_t x = 0; x < mWidth; ++x) {
                buf(x, y) = convFn(get(x, y));
            }
        }
    }


protected:
    virtual uint8_t getImpl(uint32_t x, uint32_t y) const = 0;
    virtual void setImpl(uint32_t x, uint32_t y, uint8_t val) = 0;


    uint32_t mWidth;
    uint32_t mHeight;

};


#endif // GBEMU_SRC_GB_MATRIX_H_

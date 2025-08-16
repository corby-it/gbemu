
#ifndef GBEMU_SRC_GB_RAM_H_
#define GBEMU_SRC_GB_RAM_H_

#include "GbCommons.h"
#include <cstdint>
#include <cstring>
#include <cassert>
#include <memory>
#include <iostream>
#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>



template<size_t Size>
class Ram : public ReadWriteIf {
public:
    Ram(uint16_t startAddr = 0)
        : mStartAddr(startAddr)
        , mData(std::make_unique<uint8_t[]>(Size))
    {
        reset();
    }

    Ram(const Ram& other)
        : mStartAddr(other.mStartAddr)
        , mData(std::make_unique<uint8_t[]>(Size))
    {
        memcpy(mData.get(), other.mData.get(), Size);
    }

    Ram& operator=(const Ram& other)
    {
        mStartAddr = other.mStartAddr;
        memcpy(mData.get(), other.mData.get(), Size);

        return *this;
    }

    virtual ~Ram() {}


    virtual uint8_t read8(uint16_t addr) const override
    {
        addr -= mStartAddr;
        assert(addr < Size);
        return mData[addr];
    }

    virtual void write8(uint16_t addr, uint8_t val) override
    {
        addr -= mStartAddr;
        assert(addr < Size);
        mData[addr] = val;
    }

    const uint8_t& operator[](uint16_t addr) const
    {
        assert(addr < Size);
        return mData[addr];
    }
    
    uint8_t& operator[](uint16_t addr)
    {
        assert(addr < Size);
        return mData[addr];
    }

    constexpr size_t size() const { return Size; }

    virtual void reset()
    {
        memset(mData.get(), 0, Size);
    }

    uint8_t* data() { return mData.get(); }
    uint16_t startAddr() const { return mStartAddr; }

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/)
    {
        ar(mStartAddr, cereal::binary_data(mData.get(), Size));
    }

    uint8_t* getPtr(uint16_t addr) const
    {
        addr -= mStartAddr;
        assert(addr < Size);
        return mData.get() + addr;
    }

protected:
    uint16_t mStartAddr;
    std::unique_ptr<uint8_t[]> mData;

};





class Lockable {
public:
    Lockable()
        : mLocked(false)
    {}

    Lockable(const Lockable&) = default;
    Lockable& operator=(const Lockable&) = default;

    void lock(bool l) { mLocked = l; }
    bool isLocked() const { return mLocked; }

    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/)
    {
        ar(mLocked);
    }

protected: 
    bool mLocked;

};

CEREAL_CLASS_VERSION(Lockable, 1);





template<size_t Size>
class LockableRam : public Ram<Size>, public Lockable {
public:
    LockableRam(uint16_t startAddr = 0)
        : Ram<Size>(startAddr)
    {}

    LockableRam(const LockableRam& other)
        : Ram<Size>(other)
        , Lockable(other)
    {}

    LockableRam& operator=(const LockableRam& other)
    {
        Ram<Size>::operator=(other);
        Lockable::operator=(other);

        return *this;
    }


    virtual void reset() override
    {
        Ram<Size>::reset();
        lock(false);
    }

    uint8_t read8(uint16_t addr) const override
    {
        // when some parts of the memory are locked, reading returns
        // garbage values or FF (source: https://gbdev.io/pandocs/Rendering.html#ppu-modes)
        if (isLocked())
            return 0xFF;

        return Ram<Size>::read8(addr);
    }

    void write8(uint16_t addr, uint8_t val) override
    {
        if (isLocked())
            return;

        Ram<Size>::write8(addr, val);
    }


    template<class Archive>
    void serialize(Archive& ar, uint32_t const /*version*/)
    {
        ar(cereal::base_class<Ram<Size>>(this));
        ar(cereal::base_class<Lockable>(this));
    }

};


#endif // GBEMU_SRC_GB_RAM_H_

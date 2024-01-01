
#ifndef GBEMU_SRC_GB_RAM_H_
#define GBEMU_SRC_GB_RAM_H_

#include <cstdint>
#include <cstring>
#include <cassert>




template<size_t Size>
class Ram {
public:
    Ram(uint16_t startAddr = 0)
        : mStartAddr(startAddr)
    {
        mData = new uint8_t[Size];
        memset(mData, 0, Size);
    }

    virtual ~Ram() {
        delete[] mData;
    }

    virtual uint8_t read8(uint16_t addr) const
    {
        addr -= mStartAddr;
        assert(addr < Size);
        return mData[addr];
    }

    virtual void write8(uint16_t addr, uint8_t val)
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


protected:
    const uint8_t* getPtr(uint16_t addr) const
    {
        addr -= mStartAddr;
        assert(addr < Size);
        return mData + addr;
    }

    uint16_t mStartAddr;
    uint8_t* mData;

};



template<size_t Size>
class LockableRam : public Ram<Size> {
public:
    LockableRam(uint16_t startAddr = 0)
        : Ram(startAddr)
        , mLocked(false)
    {}

    virtual uint8_t read8(uint16_t addr) const override
    {
        // when some parts of the memory are locked, reading returns
        // garbage values or FF (source: https://gbdev.io/pandocs/Rendering.html#ppu-modes)
        if (mLocked)
            return 0xFF;

        return Ram::read8(addr);
    }

    virtual void write8(uint16_t addr, uint8_t val) override
    {
        if (mLocked)
            return;

        Ram::write8(addr, val);
    }

    void lock(bool l) { mLocked = l; }
    bool isLocked() const { return mLocked; }

protected:
    bool mLocked;

};


#endif // GBEMU_SRC_GB_RAM_H_

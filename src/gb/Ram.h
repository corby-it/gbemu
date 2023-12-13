
#ifndef GBEMU_SRC_GB_RAM_H_
#define GBEMU_SRC_GB_RAM_H_

#include <cstdint>
#include <cstring>
#include <cassert>



template<size_t N>
class WRam {
public:
    WRam() {
        mData = new uint8_t[N];
        memset(mData, 0, N);
    }

    ~WRam() {
        delete[] mData;
    }

    uint8_t read8(uint16_t addr) const
    {
        assert(addr < N);
        return mData[addr];
    }

    void write8(uint16_t addr, uint8_t val)
    {
        assert(addr < N);
        mData[addr] = val;
    }


private:
    uint8_t* mData;

};




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

    uint8_t read8(uint16_t addr) const
    {
        addr -= mStartAddr;
        assert(addr < Size);
        return mData[addr];
    }

    void write8(uint16_t addr, uint8_t val)
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


#endif // GBEMU_SRC_GB_RAM_H_

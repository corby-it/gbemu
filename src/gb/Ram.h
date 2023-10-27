
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
        memset(mData, 0, sizeof(mData));
    }

    ~WRam() {
        delete[] mData;
    }

    uint8_t read8(uint16_t addr) const
    {
        assert(addr < N);
        return mData[addr];
    }

    uint16_t read16(uint16_t addr) const
    {
        assert(addr < N - 1);
        // the GB is little endian so:
        // - the byte at [addr] is the lsb
        // - the byte at [addr+1] is the msb
        return mData[addr] | (mData[addr+1] << 8);
    }

    void write8(uint16_t addr, uint8_t val)
    {
        assert(addr < N);
        mData[addr] = val;
    }
  
    void write16(uint16_t addr, uint16_t val)
    {
        assert(addr < N - 1);
        // the GB is little endian so:
        // - the lsb must be written at [addr]
        // - the msb must be written at [addr+1]
        mData[addr] = val & 0xff;
        mData[addr+1] = (val >> 8) & 0xff;
    }


private:
    uint8_t* mData;

};


#endif // GBEMU_SRC_GB_RAM_H_

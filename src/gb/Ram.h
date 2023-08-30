
#ifndef GBEMU_SRC_GB_RAM_H_
#define GBEMU_SRC_GB_RAM_H_

#include <cstdint>
#include <cstring>


class WRam {
public:
    WRam() {
        memset(mData, 0, sizeof(mData));
    }

    uint8_t read8(uint16_t addr) const;
    uint16_t read16(uint16_t addr) const;

    void write8(uint16_t addr, uint8_t val);
    void write16(uint16_t addr, uint16_t val);


private:
    uint8_t mData[64*1024];

};


#endif // GBEMU_SRC_GB_RAM_H_

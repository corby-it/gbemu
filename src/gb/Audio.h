

#ifndef GBEMU_SRC_GB_AUDIO_H_
#define GBEMU_SRC_GB_AUDIO_H_

#include "GbCommons.h"


// Just a dummy implementation to have some registers for read/write operations

class Audio {
public:
    Audio();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);

private:

    uint8_t mData[mmap::regs::audio::end - mmap::regs::audio::start + 1];

};



#endif // GBEMU_SRC_GB_AUDIO_H_


#include "Audio.h"
#include <cstring>


Audio::Audio()
{
    memset(mData, 0, sizeof(mData));

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

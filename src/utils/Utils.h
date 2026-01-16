
#ifndef GBEMU_SRC_UTILS_UTILS_H_
#define GBEMU_SRC_UTILS_UTILS_H_

#include "gb/GameBoyCore.h"
#include <filesystem>


void saveDisplayToFile(const PPU& gb, std::filesystem::path pngPath, uint32_t scaling = 1);

static inline void saveDisplayToFile(const GameBoy& gb, std::filesystem::path pngPath, uint32_t scaling = 1)
{
    saveDisplayToFile(gb.ppu, pngPath, scaling);
}

bool compareDisplayWithFile(const GameBoy& gb, std::filesystem::path pngPath);



#endif // GBEMU_SRC_UTILS_UTILS_H_
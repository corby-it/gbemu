
#ifndef GBEMU_SRC_UTILS_UTILS_H_
#define GBEMU_SRC_UTILS_UTILS_H_

#include "gb/GameBoyCore.h"
#include <filesystem>


void saveDisplayToFile(const GameBoyClassic& gb, std::filesystem::path pngPath, uint32_t scaling = 1);

bool compareDisplayWithFile(const GameBoyClassic& gb, std::filesystem::path pngPath);



#endif // GBEMU_SRC_UTILS_UTILS_H_
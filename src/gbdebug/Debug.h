

#ifndef GBEMU_SRC_DEBUG_DEBUG_H_
#define GBEMU_SRC_DEBUG_DEBUG_H_

#include "gb/Bus.h"
#include <string>

std::string instructionToStr(const Bus& bus, uint16_t pc);



#endif // GBEMU_SRC_DEBUG_DEBUG_H_
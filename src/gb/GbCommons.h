
#ifndef GBEMU_SRC_GB_GBCOMMONS_H_
#define GBEMU_SRC_GB_GBCOMMONS_H_

#include <cstdint>

// the gb cpu actually runs at 4.194304 MHz but, since we are not counting actual clock
// cycles but machine cycles (clock cycles / 4) we have to use the clock frequency
// divided by 4
static constexpr uint32_t clockFreq = 4194304;
static constexpr uint32_t machineFreq = 1048576;



#endif // GBEMU_SRC_GB_GBCOMMONS_H_
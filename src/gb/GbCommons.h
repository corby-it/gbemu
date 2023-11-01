
#ifndef GBEMU_SRC_GB_GBCOMMONS_H_
#define GBEMU_SRC_GB_GBCOMMONS_H_

#include <cstdint>

// the gb cpu actually runs at 4.194304 MHz but, since we are not counting actual clock
// cycles but machine cycles (clock cycles / 4) we have to use the clock frequency
// divided by 4
static constexpr uint32_t clockFreq = 4194304;
static constexpr uint32_t machineFreq = 1048576;



// Memory map of the gameboy address bus:
// - https://gbdev.io/pandocs/Memory_Map.html#io-ranges

#define MMAP_T  static constexpr uint16_t

namespace mmap {
    namespace rom {
        MMAP_T start = 0x0000;
        MMAP_T end = 0x7FFF;
    }
    namespace vram {
        MMAP_T start = 0x8000;
        MMAP_T end = 0x9FFF;
    }
    namespace xram {
        MMAP_T start = 0xA000;
        MMAP_T end = 0xBFFF;
    }
    namespace wram {
        MMAP_T start = 0xC000;
        MMAP_T end = 0xDFFF;
    }
    namespace echoram {
        MMAP_T start = 0xE000;
        MMAP_T end = 0xFDFF;
    }
    namespace oam {
        MMAP_T start = 0xFE00;
        MMAP_T end = 0xFE9F;
    }
    namespace regs {
        MMAP_T start = 0xFF00;

        namespace timer {
            MMAP_T start = 0xFF04;
            MMAP_T DIV = 0xFF04;
            MMAP_T TIMA = 0xFF05;
            MMAP_T TMA = 0xFF06;
            MMAP_T TAC = 0xFF07;
            MMAP_T end = 0xFF07;
        }

        MMAP_T IF = 0xFF0F;

        MMAP_T end = 0xFF7F;
    }
    namespace hram {
        MMAP_T start = 0xFF80;
        MMAP_T end = 0xFFFE;
    }

    MMAP_T IE = 0xFFFF;
}




#endif // GBEMU_SRC_GB_GBCOMMONS_H_
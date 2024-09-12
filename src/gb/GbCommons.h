
#ifndef GBEMU_SRC_GB_GBCOMMONS_H_
#define GBEMU_SRC_GB_GBCOMMONS_H_

#include <cstdint>
#include <cassert>
#include <array>
#include <memory>





// Memory map of the gameboy address bus:
// - https://gbdev.io/pandocs/Memory_Map.html#io-ranges

#define MMAP_T  static constexpr uint16_t

namespace mmap {
    namespace rom {
        MMAP_T start = 0x0000;

        namespace bank0 {
            MMAP_T start = 0x0000;
            MMAP_T end = 0x3FFF;
        }
        namespace bankN {
            MMAP_T start = 0x4000;
            MMAP_T end = 0x7FFF;
        }

        MMAP_T end = 0x7FFF;
    }
    namespace vram {
        MMAP_T start = 0x8000;
        MMAP_T end = 0x9FFF;
    }
    namespace external_ram {
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
    namespace prohibited {
        MMAP_T start = 0xFEA0;
        MMAP_T end = 0xFEFF;
    }
    namespace regs {
        MMAP_T start = 0xFF00;

        MMAP_T joypad = 0xFF00;

        MMAP_T serial_data = 0xFF01;
        MMAP_T serial_ctrl = 0xFF02;

        namespace timer {
            MMAP_T start = 0xFF04;
            MMAP_T DIV = 0xFF04;
            MMAP_T TIMA = 0xFF05;
            MMAP_T TMA = 0xFF06;
            MMAP_T TAC = 0xFF07;
            MMAP_T end = 0xFF07;
        }

        MMAP_T IF = 0xFF0F;

        namespace audio {
            MMAP_T start = 0xFF10;
            MMAP_T end = 0xFF3F;
        }

        namespace lcd {
            MMAP_T start = 0xFF40;
            MMAP_T lcdc = 0xFF40;
            MMAP_T stat = 0xFF41;
            MMAP_T scy = 0xFF42;
            MMAP_T scx = 0xFF43;
            MMAP_T ly = 0xFF44;
            MMAP_T lyc = 0xFF45;
            MMAP_T dma = 0xFF46;
            MMAP_T bgp = 0xFF47;
            MMAP_T obp0 = 0xFF48;
            MMAP_T obp1 = 0xFF49;
            MMAP_T wy = 0xFF4A;
            MMAP_T wx = 0xFF4B;
            MMAP_T end = 0xFF4B;
        }

        MMAP_T end = 0xFF7F;
    }
    namespace hiram {
        MMAP_T start = 0xFF80;
        MMAP_T end = 0xFFFE;
    }

    MMAP_T IE = 0xFFFF;
}




// Generic register class with helper methods
struct RegU8 {
public:
    virtual ~RegU8() {}

    virtual uint8_t asU8() const = 0;

    virtual void fromU8(uint8_t val) = 0;

    virtual operator uint8_t() const { return asU8(); }

    virtual bool operator==(const RegU8& other) const
    {
        return asU8() == other.asU8();
    }
};



#endif // GBEMU_SRC_GB_GBCOMMONS_H_
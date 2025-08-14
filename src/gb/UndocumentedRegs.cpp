
#include "UndocumentedRegs.h"


// for more info see: https://gbdev.io/pandocs/CGB_Registers.html#undocumented-registers


namespace undoc = mmap::regs::undocumented;


UndocumentedRegs::UndocumentedRegs()
    : mIsCgb(false)
{
    reset();
}

void UndocumentedRegs::reset()
{
    mFF72 = 0;
    mFF73 = 0;
    mFF74 = 0;
    mFF75 = 0;
}

uint8_t UndocumentedRegs::read8(uint16_t addr) const
{
    if (mIsCgb && addr >= undoc::start && addr <= undoc::end) {
        switch (addr) {
        case 0xFF72: return mFF72;
        case 0xFF73: return mFF73;
        case 0xFF74: return mFF74;
        case 0xFF75: return mFF75 | 0x8F; // only bits 4, 5 and 6 are r/w
        default:
            return 0xff;
        }
    }
    else {
        return 0xff;
    }
}

void UndocumentedRegs::write8(uint16_t addr, uint8_t val)
{
    if (mIsCgb && addr >= undoc::start && addr <= undoc::end) {
        switch (addr) {
        case 0xFF72: mFF72 = val; break;
        case 0xFF73: mFF73 = val; break;
        case 0xFF74: mFF74 = val; break;
        case 0xFF75: mFF75 = (val | 0x8F); break; // only bits 4, 5 and 6 are r/w
        default:
            break;
        }
    }
}

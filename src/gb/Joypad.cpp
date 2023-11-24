#include "Joypad.h"

Joypad::Joypad()
    : mSelection(Selection::Disabled)
    , mDpadByte(0x0f)
    , mBtnsByte(0x0f)
{}

void Joypad::write(uint8_t val)
{
    // only bits 4 and 5 are actually writable, the other bits are discarded
    
    // bit5 bit4    Result
    // 0    0       disabled?! both?! couldn't find info on this case...
    // 0    1       read buttons
    // 1    0       read d-pad
    // 1    1       disabled (all buttons will read as 1)

    // keep only bits 4 and 5
    val = (val >> 4) & 0x03;

    switch (val) {
    case 0x00: mSelection = Selection::Both; break;
    case 0x01: mSelection = Selection::Buttons; break;
    case 0x02: mSelection = Selection::Dpad; break;
    default:
    case 0x03: mSelection = Selection::Disabled; break;
    }
}

uint8_t Joypad::read() const
{
    // bits 6 and 7 are not used and will read 0
    // bits 4 and 5 will read as the corresponding value from the selection variable
    // bits 0, 1, 2 and 3 will report the value of the corresponding buttons 
    // (1 means 'not pressed' and 0 means 'pressed')

    uint8_t val = static_cast<uint8_t>(mSelection) << 4;

    switch (mSelection) {
    case Joypad::Selection::Dpad:
        val |= mDpadByte;
        break;

    case Joypad::Selection::Buttons:
        val |= mBtnsByte;
        break;
    
    default:
    case Joypad::Selection::Disabled:
    case Joypad::Selection::Both:
        val |= 0x0f;
        break;
    }

    return val;
}

void Joypad::press(Btn bt)
{
    // all buttons are active low, when one of them is 
    // pressed the corresponding bit goes from 1 to 0

    switch (bt) {
    case Joypad::Btn::A: mBtnsByte &= ~(0x01); break;
    case Joypad::Btn::B: mBtnsByte &= ~(0x02); break;
    case Joypad::Btn::Select: mBtnsByte &= ~(0x04); break;
    case Joypad::Btn::Start: mBtnsByte &= ~(0x08); break;

    case Joypad::Btn::Right: mDpadByte &= ~(0x01); break;
    case Joypad::Btn::Left: mDpadByte &= ~(0x02); break;
    case Joypad::Btn::Up: mDpadByte &= ~(0x04); break;
    case Joypad::Btn::Down: mDpadByte &= ~(0x08); break;
    default:
        break;
    }
}

void Joypad::release(Btn bt)
{
    // all buttons are active low, when one of them is 
    // released the corresponding bit goes from 0 to 1

    switch (bt) {
    case Joypad::Btn::A: mBtnsByte |= 0x01; break;
    case Joypad::Btn::B: mBtnsByte |= 0x02; break;
    case Joypad::Btn::Select: mBtnsByte |= 0x04; break;
    case Joypad::Btn::Start: mBtnsByte |= 0x08; break;

    case Joypad::Btn::Right: mDpadByte |= 0x01; break;
    case Joypad::Btn::Left: mDpadByte |= 0x02; break;
    case Joypad::Btn::Up: mDpadByte |= 0x04; break;
    case Joypad::Btn::Down: mDpadByte |= 0x08; break;
    default:
        break;
    }
}

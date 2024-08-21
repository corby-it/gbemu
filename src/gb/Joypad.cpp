
#include "Joypad.h"
#include "Irqs.h"
#include "GbCommons.h"



Joypad::Joypad(Bus& bus)
    : mBus(bus)
{
    reset();
}

void Joypad::reset()
{
    mSelection = Selection::Disabled;
    
    mDpadByte = static_cast<uint8_t>(Selection::Dpad) << 4 | 0x0F;
    mBtnsByte = static_cast<uint8_t>(Selection::Buttons) << 4 | 0x0F;

    // bits 6 and 7 (that are unused) always read as 1
    // source: gekkio gameboy technical manual, page 44
    mDpadByte |= 0xC0;
    mBtnsByte |= 0xC0;

    mCounterEnabled = false;
    mCyclesCounter = 0;
}

void Joypad::step(uint32_t mCycles)
{
    // from the gameboy developer manual (page 25), the joypad interrupt is triggered
    // after 16 clock cycles (4 machine cycles) after a negative edge
    if (mCounterEnabled) {
        mCyclesCounter += mCycles;

        if (mCyclesCounter > 4) {
            // trigger interrupt
            auto currIF = mBus.read8(mmap::regs::IF);
            mBus.write8(mmap::regs::IF, Irqs::mask(Irqs::Type::Joypad) | currIF);
        }
    }
}


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

    Selection newSelection;

    switch (val) {
    case 0x00: newSelection = Selection::Both; break;
    case 0x01: newSelection = Selection::Buttons; break;
    case 0x02: newSelection = Selection::Dpad; break;
    default:
    case 0x03: newSelection = Selection::Disabled; break;
    }

    if (newSelection == Selection::Buttons || newSelection == Selection::Dpad) {
        // when we enable one of the two inputs we also reset the cycles counter
        mCyclesCounter = 0;
    }
    if (newSelection == Selection::Disabled || newSelection == Selection::Both) {
        // when we disable both inputs we also reset the cycles counter
        mCounterEnabled = false;
        mCyclesCounter = 0;
    }

    mSelection = newSelection;
}

uint8_t Joypad::read() const
{
    // bits 6 and 7 are not used and will read 0
    // bits 4 and 5 will read as the corresponding value from the selection variable
    // bits 0, 1, 2 and 3 will report the value of the corresponding buttons 
    // (1 means 'not pressed' and 0 means 'pressed')

    switch (mSelection) {
    case Joypad::Selection::Dpad: return mDpadByte;
    case Joypad::Selection::Buttons: return mBtnsByte;
    default:
    case Joypad::Selection::Disabled: return 0xCF;
    case Joypad::Selection::Both: return 0xFF;
    }
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

    // if the pressed button is also in the active selection we
    // can start the interrupt cycles counter 
    if (inCurrentSelection(bt)) 
        mCounterEnabled = true;
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

    if (inCurrentSelection(bt)) {
        // if the button is released and all the bits in its group are 1 we have to stop the counter
        if ((Joypad::read() & 0x0F) == 0x0F) {
            mCounterEnabled = false;
            mCyclesCounter = 0;
        }
    }
}

void Joypad::action(const PressedButton& pressedBtns)
{
    for(auto btn : allBtns) 
        release(btn);
    
    for (size_t i = 0; i < pressedBtns.count; ++i)
        press(pressedBtns.pressed[i]);
}

bool Joypad::inCurrentSelection(Joypad::Btn btn) const
{
    if ((btn == Btn::Up || btn == Btn::Down || btn == Btn::Left || btn == Btn::Right) && mSelection == Selection::Dpad)
        return true;
    else if ((btn == Btn::A || btn == Btn::B || btn == Btn::Start || btn == Btn::Select) && mSelection == Selection::Buttons)
        return true;
    else 
        return false;
}


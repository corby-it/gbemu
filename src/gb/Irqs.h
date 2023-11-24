
#ifndef GBEMU_SRC_GB_IRQS_H_
#define GBEMU_SRC_GB_IRQS_H_


#include <cstdint>
#include <cassert>


struct Irqs {

    // There are 5 possible interrupts, all of them can be disabled (there are no NMIs)
    // - Vertical blanking interrupt: TODO
    // - LCD status interrupt: TODO
    // - Timer overflow: requested when the TIMA register in the timer overflows
    // - Serial transfer completed: TODO
    // - Joypad: triggered on the falling edge of P10-P13 input signal (a button has been pressed)

    // Interrupts are controlled using the following flags and regsiters:
    // - IME: Interrupt Master Enable
    //      internal CPU flag, when false interrupts are not executed, it's not accesible from the outside 
    //      and can only be manipulated with the EI, DI and RETI instructions
    // 
    // - IF - Interrupt Flags (addr: 0xFF0F)
    //      the interrupt flags register can be used to determine which interrupt has been requested,
    //      see the masks for bit meanings
    // 
    // - IE - Interrupt Enable (addr: 0xFFFF)
    //      the interrupt enable regsiter is used to control which interrupts are requested by the program,
    //      see the masks for bit meanings

    // the registers are not internal to the CPU and can be accessed on the main bus


    enum class Type {
        VBlank,
        Lcd,
        Timer,
        Serial,
        Joypad
    };

    // the bits used in the IF and IE registers are the same for the 5 available interrupts
    static constexpr uint8_t mask(Type type) {
        switch (type) {
        case Irqs::Type::VBlank:  return 0x01;
        case Irqs::Type::Lcd:     return 0x02;
        case Irqs::Type::Timer:   return 0x04;
        case Irqs::Type::Serial:  return 0x08;
        case Irqs::Type::Joypad:  return 0x10;
        default:
            assert(false);
            return 0;
        }
    }

    // the address that will be called when a specific interrupt is requested
    static constexpr uint16_t addr(Type type) {
        switch (type) {
        case Irqs::Type::VBlank:  return 0x0040;
        case Irqs::Type::Lcd:     return 0x0048;
        case Irqs::Type::Timer:   return 0x0050;
        case Irqs::Type::Serial:  return 0x0058;
        case Irqs::Type::Joypad:  return 0x0060;
        default:
            assert(false);
            return 0;
        }
    }

    bool ime;
    uint8_t IF;
    uint8_t IE;

    void reset();
};


#endif // GBEMU_SRC_GB_IRQS_H_

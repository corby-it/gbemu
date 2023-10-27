
#ifndef GBEMU_SRC_GB_CPU_H_
#define GBEMU_SRC_GB_CPU_H_

#include "Bus.h"
#include <cstdint>
#include <optional>
#include <stack>


// the GB uses a CPU similar to the Zilog Z80

struct Flags {
    // the flags register works as follows:
    // bit      function
    // 0..3     unused
    // 4        carry flag: 1 if carry from last math operation or if A is the smaller value when using the CP instruction
    // 5        half-carry flag: 1 if carry occurred in the lower nibble in the last math operation
    // 6        subtract flag: 1 if a subtracion was performed in the last math operation
    // 7        zero flag: 1 if the result of the last math operation is 0 or if two values match when using the CP instruction

    bool Z;
    bool N;
    bool H;
    bool C;

    uint8_t asU8() const {
        uint8_t val = 0;
        
        if (Z) val |= maskZ;
        if (N) val |= maskN;
        if (H) val |= maskH;
        if (C) val |= maskC;

        return val;
    }

    void fromU8(uint8_t val) {
        Z = val & maskZ;
        N = val & maskN;
        H = val & maskH;
        C = val & maskC;
    }

    operator uint8_t() const { return asU8(); }

    void operator=(uint8_t val) { fromU8(val); }

    bool operator==(const Flags& other) const {
        return Z == other.Z
            && N == other.N
            && H == other.H
            && C == other.C;
    }

    static constexpr const uint8_t maskZ = 0b10000000;
    static constexpr const uint8_t maskN = 0b01000000;
    static constexpr const uint8_t maskH = 0b00100000;
    static constexpr const uint8_t maskC = 0b00010000;
};


struct Registers {
    // there are 7 general purpose 8-bit register
    // the A register is also used as the accumulator for math operations,
    // they can also be accessed as 16-bit registers as follow
    // BC   
    // DE
    // HL
    uint8_t A, B, C, D, E, H, L;

    // program counter and stack pointer are 16-bit registers
    uint16_t PC, SP;

    // flags: Z, N, H and C
    Flags flags;


    // Utils --------------------------------------------------------------------------------------
    uint16_t AF() const { return (A << 8) | flags; }
    uint16_t BC() const { return (B << 8) | C; }
    uint16_t DE() const { return (D << 8) | E; }
    uint16_t HL() const { return (H << 8) | L; }

    void setBC(uint16_t val) {
        B = val >> 8;
        C = val & 0xff;
    }
    void setDE(uint16_t val) {
        D = val >> 8;
        E = val & 0xff;
    }
    void setHL(uint16_t val) {
        H = val >> 8;
        L = val & 0xff;
    }
    void setAF(uint16_t val) {
        A = val >> 8;
        flags = val & 0xff;
    }


    void reset();

    bool equal(const Registers& other);
    bool equalSkipPC(const Registers& other);

    static constexpr const uint16_t PCinitialValue = 0x0000;
    static constexpr const uint16_t SPinitialValue = 0x0000;

};





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




class CPU {
public:
    CPU(Bus& bus);

    void reset();

    bool step();

    uint32_t elapsedCycles() const { return mCycles; }
    
    Registers regs;
    Irqs irqs;

    size_t irqNesting() const { return mIrqNesting.size(); }

private:

    std::optional<Irqs::Type> checkIrq();


    uint8_t execute(uint8_t opcode, bool& ok);
    uint8_t executeCb(bool& ok);

    // 8-bit load instructions
    uint8_t opLdRegImm(uint8_t& dst);
    uint8_t opLdRegReg(uint8_t& dst, const uint8_t& src);
    uint8_t opLdRegInd(uint8_t& dst, uint16_t srcAddr);
    uint8_t opLdIndReg(uint16_t dstAddr, const uint8_t& src);
    uint8_t opLdIndImm();
    uint8_t opLdRegIndImm16();
    uint8_t opLdIndImm16Reg();
    uint8_t opLdRegIndImm8();
    uint8_t opLdIndImm8Reg();
    uint8_t opLdAIndDec();
    uint8_t opLdAIndInc();
    uint8_t opLdIndDecA();
    uint8_t opLdIndIncA();

    // 16-bit load instructions
    uint8_t opLdReg16Imm(uint8_t& msb, uint8_t& lsb);
    uint8_t opLdReg16Imm(uint16_t& dst);
    uint8_t opLdIndImm16Sp();
    uint8_t opLdSpHl();
    uint8_t opLdHlSpOffset();
    uint8_t opPushReg16(uint16_t val);

    template<typename T1, typename T2>
    uint8_t opPopReg16(T1& msb, T2& lsb) {
        // (this function is templated to work with flags as well)
        // pop the register onto the stack
        // e.g.: POP BC
        lsb = mBus.read8(regs.SP);
        msb = mBus.read8(regs.SP + 1);
        regs.SP += 2;

        return 3;
    }

    // 8-bit arithmetic and logical instructions
    uint8_t opAdd8Common(uint8_t rhs, uint8_t cycles);
    uint8_t opAddReg(uint8_t reg);
    uint8_t opAddInd();
    uint8_t opAddImm();

    uint8_t opAdcCommon(uint8_t rhs, uint8_t cycles);
    uint8_t opAdcReg(uint8_t reg);
    uint8_t opAdcInd();
    uint8_t opAdcImm();

    uint8_t opSubCommon(uint8_t rhs, uint8_t cycles);
    uint8_t opSubReg(uint8_t reg);
    uint8_t opSubInd();
    uint8_t opSubImm();

    uint8_t opSbcCommon(uint8_t rhs, uint8_t cycles);
    uint8_t opSbcReg(uint8_t reg);
    uint8_t opSbcInd();
    uint8_t opSbcImm();

    uint8_t opAndCommon(uint8_t rhs, uint8_t cycles);
    uint8_t opAndReg(uint8_t reg);
    uint8_t opAndInd();
    uint8_t opAndImm();

    uint8_t opOrCommon(uint8_t rhs, uint8_t cycles);
    uint8_t opOrReg(uint8_t reg);
    uint8_t opOrInd();
    uint8_t opOrImm();

    uint8_t opXorCommon(uint8_t rhs, uint8_t cycles);
    uint8_t opXorReg(uint8_t reg);
    uint8_t opXorInd();
    uint8_t opXorImm();
    
    uint8_t opCpCommon(uint8_t rhs, uint8_t cycles);
    uint8_t opCpReg(uint8_t reg);
    uint8_t opCpInd();
    uint8_t opCpImm();

    uint8_t opIncReg(uint8_t& reg);
    uint8_t opIncInd();

    uint8_t opDecReg(uint8_t& reg);
    uint8_t opDecInd();

    uint8_t opCcf();
    uint8_t opScf();
    uint8_t opCpl();
    uint8_t opDaa();

    uint8_t opAddReg16(uint16_t rhs);
    uint8_t opAddSpImm();

    uint8_t opIncReg16(uint8_t& msb, uint8_t& lsb);
    uint8_t opDecReg16(uint8_t& msb, uint8_t& lsb);
    uint8_t opIncSp();
    uint8_t opDecSp();
    
    uint8_t opRlca();
    uint8_t opRla();
    uint8_t opRrca();
    uint8_t opRra();

    uint8_t opCbRlcReg(uint8_t& reg);
    uint8_t opCbRlcInd();
    uint8_t opCbRlReg(uint8_t& reg);
    uint8_t opCbRlInd();

    uint8_t opCbRrcReg(uint8_t& reg);
    uint8_t opCbRrcInd();
    uint8_t opCbRrReg(uint8_t& reg);
    uint8_t opCbRrInd();

    uint8_t opCbSlaReg(uint8_t& reg);
    uint8_t opCbSlaInd();
    uint8_t opCbSraReg(uint8_t& reg);
    uint8_t opCbSraInd();
    uint8_t opCbSrlReg(uint8_t& reg);
    uint8_t opCbSrlInd();

    uint8_t opCbSwapReg(uint8_t& reg);
    uint8_t opCbSwapInd();

    uint8_t opCbBitReg(uint8_t b, uint8_t& reg);
    uint8_t opCbBitInd(uint8_t b);
    uint8_t opCbSetReg(uint8_t b, uint8_t& reg);
    uint8_t opCbSetInd(uint8_t b);
    uint8_t opCbResReg(uint8_t b, uint8_t& reg);
    uint8_t opCbResInd(uint8_t b);

    uint8_t opJpImm();
    uint8_t opJpHL();
    uint8_t opJpCond(bool cond);
    uint8_t opJrImm();
    uint8_t opJrCond(bool cond);

    uint8_t opCallImm();
    uint8_t opCallCond(bool cond);
    uint8_t opRst(uint8_t offset);
    uint8_t opRet();
    uint8_t opRetCond(bool cond);
    uint8_t opReti();

    uint8_t opEi();
    uint8_t opDi();

    uint8_t opCallIrq(Irqs::Type type);



    
    // Utils --------------------------------------------------------------------------------------
    uint8_t compl2(uint8_t val) {
        return ~val + 1;
    }

    uint8_t hnib(uint8_t val) {
        return val >> 4;
    }

    uint8_t lnib(uint8_t val) {
        return val & 0x0F;
    }

    bool checkCarry(uint16_t lhs, uint16_t rhs, bool carry = false) {
        return (lhs & 0x00ff) + (rhs & 0x00ff) + carry > 0x00ff;
    }

    bool checkCarry16(uint32_t lhs, uint32_t rhs) {
        return (lhs & 0x0000ffff) + (rhs & 0x0000ffff) > 0x0000ffff;
    }

    bool checkHalfCarry16(uint16_t lhs, uint16_t rhs) {
        return (lhs & 0x0fff) + (rhs & 0x0fff) > 0x0fff;
    }

    bool checkHalfCarry(uint8_t lhs, uint8_t rhs, bool carry = false) {
        return ((lhs & 0x0f) + (rhs & 0x0f) + carry) > 0x0f;
    }

    bool checkBorrow(uint8_t lhs, uint8_t rhs) {
        return lhs < rhs;
    }

    bool checkHalfBorrow(uint8_t lhs, uint8_t rhs, bool carry = false) {
        return (lhs & 0x0f) < (rhs & 0x0f) + carry;
    }


    // Members ------------------------------------------------------------------------------------


    // each instruction of the gameboy uses a number of clock cycles that is 
    // divisible by 4, here we use that number already divided by 4 (usually called m-cycles,
    // as in "machine cycles")
    uint32_t mCycles;

    // When the EI instruction is executed the IME flag is not immediately set, it is set only after
    // the instruction following EI is exectued
    bool mImeScheduled;

    Bus& mBus;

    // We monitor the irq nesting level, every time an interrupt is serviced we push the old PC to this
    // stack, when we call a RET or RETI we check if the PC we are restoring is on the top of the stack
    // if so, we know that one irq routing is done. The size of the stack is the nesting depth
    std::stack<uint16_t> mIrqNesting;

};


#endif // GBEMU_SRC_GB_CPU_H_

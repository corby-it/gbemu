
#ifndef GBEMU_SRC_GB_CPU_H_
#define GBEMU_SRC_GB_CPU_H_

#include "Bus.h"
#include <cstdint>


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




class CPU {
public:
    CPU(Bus& bus);

    void reset();

    bool step();

    uint32_t elapsedCycles() const { return mCycles; }
    
    Registers regs;

    // the gb cpu actually runs at 4.194304 MHz but, since we are not counting actual clock
    // cycles but machine cycles (clock cycles / 4) we have to use the clock frequency
    // divided by 4
    static constexpr uint32_t freq = 1048576;

private:

    uint8_t execute(uint8_t opcode, bool& err);

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
    
    uint8_t opJpImm();
    uint8_t opJpInd();
    uint8_t opJpCondImm(bool cond);
    uint8_t opCpInd();



    
    // Utils --------------------------------------------------------------------------------------
    uint8_t compl2(uint8_t val) {
        return ~val + 1;
    }

    bool checkCarry(uint16_t mathRes) {
        return mathRes & 0xff00;
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

    Bus& mBus;

};


#endif // GBEMU_SRC_GB_CPU_H_


#ifndef GBEMU_SRC_GB_CPU_H_
#define GBEMU_SRC_GB_CPU_H_

#include "Bus.h"
#include <cstdint>


// the GB uses a CPU similar to the Zilog Z80



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

    // the flags register works as follows:
    // bit      function
    // 0..3     unused
    // 4        carry flag: 1 if carry from last math operation or if A is the smaller value when using the CP instruction
    // 5        half-carry flag: 1 if carry occurred in the lower nibble in the last math operation
    // 6        subtract flag: 1 if a subtracion was performed in the last math operation
    // 7        zero flag: 1 if the result of the last math operation is 0 or if two values match when using the CP instruction
    uint8_t flags;


    // Utils --------------------------------------------------------------------------------------
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

    bool getZ() const { return flags >> 7; }
    bool getN() const { return (flags >> 6) & 0x1; }
    bool getH() const { return (flags >> 5) & 0x1; }
    bool getC() const { return (flags >> 4) & 0x1; }

    void setZ() { flags |= maskZ; }
    void setN() { flags |= maskN; }
    void setH() { flags |= maskH; }
    void setC() { flags |= maskC; }

    void clearZ() { flags &= ~maskZ; }
    void clearN() { flags &= ~maskN; }
    void clearH() { flags &= ~maskH; }
    void clearC() { flags &= ~maskC; }
    

    void reset();

    bool equal(const Registers& other);
    bool equalSkipPC(const Registers& other);

    static constexpr const uint16_t PCinitialValue = 0x0000;

private:

    static constexpr const uint8_t maskZ = 0b10000000;
    static constexpr const uint8_t maskN = 0b01000000;
    static constexpr const uint8_t maskH = 0b00100000;
    static constexpr const uint8_t maskC = 0b00010000;
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

    // instructions
    uint8_t opLdRegImm(uint8_t& dst);
    uint8_t opLdRegReg(uint8_t& dst, const uint8_t& src);
    uint8_t opLdRegInd(uint8_t& dst);
    uint8_t opLdIndReg(const uint8_t& src);
    uint8_t opLdIndImm();
    uint8_t opLdAInd(const uint16_t& addr);

    uint8_t opAddRegReg(const uint8_t& reg);
    uint8_t opAddInd();
    
    uint8_t opJpImm();
    uint8_t opJpInd();
    uint8_t opJpCondImm(bool cond);
    uint8_t opCpInd();



    
    // Utils --------------------------------------------------------------------------------------
    uint8_t compl2(uint8_t val) { return ~val + 1;}
    bool checkCarry(uint16_t mathRes) { return mathRes & 0xff00; }
    bool checkHalfCarry(uint16_t mathRes) { return mathRes & 0x0010; }

    // Members ------------------------------------------------------------------------------------


    // each instruction of the gameboy uses a number of clock cycles that is 
    // divisible by 4, here we use that number already divided by 4 (usually called m-cycles,
    // as in "machine cycles")
    uint32_t mCycles;

    Bus& mBus;

};


#endif // GBEMU_SRC_GB_CPU_H_

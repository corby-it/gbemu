
#include "Cpu.h"
#include <cassert>

void Registers::reset()
{
    // at reset we set all registers to 0, the actual hardware 
    // set the values depending on the model (gameboy, gameboy color, etc.)
    A = 0;
    B = 0;
    C = 0;
    D = 0;
    E = 0;
    H = 0;
    L = 0;
    PC = 0;
    SP = 0;
    flags = 0;
}



CPU::CPU(Bus& bus)
    : mBus(bus)
{
    reset();
}


void CPU::reset()
{
    mCycles = 0;

    mRegs.reset();
}

bool CPU::step()
{
    // in one step we execute one instruction, the instruction can take 1 or more machine cycles

    // fetch opcode 
    auto opcode = mBus.read8(mRegs.PC++);

    bool ok = true;
    mCycles += execute(opcode, ok);

    return ok;
}


uint8_t CPU::execute(uint8_t opcode, bool& ok)
{
    // opcode reference: 
    // - https://gbdev.io/gb-opcodes/optables/
    // - https://gekkio.fi/files/gb-docs/gbctr.pdf

    // this function return the number of machine cycles used to execute the instruction


    switch(opcode) {
    case 0x00: return 1; // NOP

    case 0x06: return opLdImm(mRegs.B); // LD B,n8
    case 0x0E: return opLdImm(mRegs.C); // LD C,n8
    case 0x16: return opLdImm(mRegs.D); // LD D,n8
    case 0x1E: return opLdImm(mRegs.E); // LD E,n8
    case 0x26: return opLdImm(mRegs.H); // LD H,n8
    case 0x2E: return opLdImm(mRegs.L); // LD L,n8
    case 0x3E: return opLdImm(mRegs.A); // LD A,n8

    // Load in B
    case 0x40: return opLdReg(mRegs.B, mRegs.B); // LD B,B
    case 0x41: return opLdReg(mRegs.B, mRegs.C); // LD B,C
    case 0x42: return opLdReg(mRegs.B, mRegs.D); // LD B,D
    case 0x43: return opLdReg(mRegs.B, mRegs.E); // LD B,E
    case 0x44: return opLdReg(mRegs.B, mRegs.H); // LD B,H
    case 0x45: return opLdReg(mRegs.B, mRegs.L); // LD B,L
    case 0x46: return opLdInd(mRegs.B); // LD B,[HL]
    case 0x47: return opLdReg(mRegs.B, mRegs.A); // LD B,A

    // Load in C
    case 0x48: return opLdReg(mRegs.C, mRegs.B); // LD C,B
    case 0x49: return opLdReg(mRegs.C, mRegs.C); // LD C,C
    case 0x4A: return opLdReg(mRegs.C, mRegs.D); // LD C,D
    case 0x4B: return opLdReg(mRegs.C, mRegs.E); // LD C,E
    case 0x4C: return opLdReg(mRegs.C, mRegs.H); // LD C,H
    case 0x4D: return opLdReg(mRegs.C, mRegs.L); // LD C,L
    case 0x4E: return opLdInd(mRegs.C); // LD C,[HL]
    case 0x4F: return opLdReg(mRegs.C, mRegs.A); // LD C,A

    // Addition
    case 0x80: return opAddReg(mRegs.B); // ADD A,B
    case 0x81: return opAddReg(mRegs.C); // ADD A,C
    case 0x82: return opAddReg(mRegs.D); // ADD A,D
    case 0x83: return opAddReg(mRegs.E); // ADD A,E
    case 0x84: return opAddReg(mRegs.H); // ADD A,H
    case 0x85: return opAddReg(mRegs.L); // ADD A,L
    case 0x86: return opAddInd(); // ADD A,[HL]

    case 0xBE: return opCpInd(); // CP A,[HL]

    case 0xC2: return opJpCondImm(!mRegs.getZ()); // JP NZ,a16
    case 0xC3: return opJpImm(); // JP a16

    case 0xCA: return opJpCondImm(mRegs.getZ()); // JP Z,a16
    case 0xD2: return opJpCondImm(!mRegs.getC()); // JP NC,a16
    case 0xDA: return opJpCondImm(mRegs.getC()); // JP C,a16

    case 0xE9: return opJpInd(); // JP HL

    default: {
        // unrecognized opcode
        ok = false;
        return 1;
    }
    }

    return 1;
}

uint8_t CPU::opLdImm(uint8_t& dst)
{
    // immediate load into register
    // e.g.: LD A,n8
    // 2 cycles required
    dst = mBus.read8(mRegs.PC++);
    return 2;
}


uint8_t CPU::opLdReg(uint8_t& dst, const uint8_t& src)
{
    // Load register into another register (too many opcodes)
    // 1 cycle required
    dst = src;
    return 1;
}

uint8_t CPU::opLdInd(uint8_t& dst)
{
    // load memory into a register, address is stored in HL
    // 2 cycles required
    dst = mBus.read8(mRegs.HL());
    return 2;
}


uint8_t CPU::opAddReg(const uint8_t& reg)
{
    // opcodes 0x80..0x85
    // addition between registers, sum A and another register, store the result in A
    // and set the appropriate flags
    // 0x80: ADD A,B
    // 0x81: ADD A,C
    // 0x82: ADD A,D
    // 0x83: ADD A,D
    // 0x84: ADD A,D
    // 0x85: ADD A,D
    // 1 cycle required

    uint16_t res = mRegs.A + reg;
    mRegs.A = (uint8_t)res;

    // check flags
    mRegs.clearN();

    if(checkCarry(res))
        mRegs.setC();
    if(checkHalfCarry(res))
        mRegs.setH();
    if(res == 0)
        mRegs.setZ();

    return 1;
}

uint8_t CPU::opAddInd()
{
    // ADD A,[HL]
    // A = A + mem[HL], 2 cycles
    uint16_t addr = mRegs.HL();
    uint16_t res = mRegs.A + mBus.read8(addr);
    
    mRegs.A = (uint8_t)res;

    // check flags
    mRegs.clearN();

    if(checkCarry(res))
        mRegs.setC();
    if(checkHalfCarry(res))
        mRegs.setH();
    if((uint8_t)res == 0)
        mRegs.setZ();

    return 2;
}

uint8_t CPU::opJpImm()
{
    // JP a16
    // immediate unconditional jump, jump to address specified in the instruction 
    // len: 3 bytes
    // cycles: 4
    uint16_t addr = mBus.read8(mRegs.PC++);
    addr |= mBus.read8(mRegs.PC++) << 8;

    mRegs.PC = addr;
    return 4;
}

uint8_t CPU::opJpInd()
{
    // JP HL
    // unconditional jump, jump to address specified in HL (NOT IN mem[HL]!!)
    mRegs.PC = mRegs.HL();
    return 1;
}

uint8_t CPU::opJpCondImm(bool cond)
{
    // JP Z,a16
    // immediate conditional jump, jump to address specified in the instruction if condition is true (aka flag is set)
    // len: 3 bytes
    // cycles: 3 if branch not taken, 4 if taken

    uint16_t addr = mBus.read8(mRegs.PC++);
    addr |= mBus.read8(mRegs.PC++) << 8;
    if(cond) {
        mRegs.PC = addr;
        return 4;
    }
    else {
        return 3;
    }
}

uint8_t CPU::opCpInd() 
{
    // CP A,[HL]
    // compares A with the value in mem[HL], it performs A - mem[HL]
    // but the result is not stored, all flags are updated accordingly
    uint16_t res = mRegs.A + compl2(mBus.read8(mRegs.HL()));

    // set N because a subtraction just happened
    mRegs.setN();

    if((uint8_t)res == 0)
        mRegs.setZ();
    if(checkCarry(res))
        mRegs.setC();
    if(checkHalfCarry(res))
        mRegs.setH();

    return 2;
}



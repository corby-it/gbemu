
#include "Cpu.h"
#include "Opcodes.h"
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
    PC = PCinitialValue;
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

    // this function returns the number of machine cycles used to execute the instruction

    switch(opcode) {
        // 0x0*
        case op::NOP          : return 1; // nothing to do here
        //case op::LD_BC_n16    : return 1;
        //case op::LD_inBC_A    : return 1;
        //case op::INC_BC       : return 1;
        //case op::INC_B        : return 1;
        //case op::DEC_B        : return 1;
        case op::LD_B_n8      : return opLdImm(mRegs.B); // LD B,n8
        //case op::RLCA         : return 1;
        //case op::LD_a16_SP    : return 1;
        //case op::ADD_HL_BC    : return 1;
        //case op::LD_A_inBC    : return 1;
        //case op::DEC_BC       : return 1;
        //case op::INC_C        : return 1;
        //case op::DEC_C        : return 1;
        case op::LD_C_n8      : return opLdImm(mRegs.C); // LD C,n8
        //case op::RRCA         : return 1;

        // 0x1*
        //case op::STOP_n8      : return 1;
        //case op::LD_DE_n16    : return 1;
        //case op::LD_inDE_A    : return 1;
        //case op::INC_DE       : return 1;
        //case op::INC_D        : return 1;
        //case op::DEC_D        : return 1;
        case op::LD_D_n8      : return opLdImm(mRegs.D); // LD D,n8
        //case op::RLA          : return 1;
        //case op::JR_e8        : return 1;
        //case op::ADD_HL_DE    : return 1;
        //case op::LD_A_inDE    : return 1;
        //case op::DEC_DE       : return 1;
        //case op::INC_E        : return 1;
        //case op::DEC_E        : return 1;
        case op::LD_E_n8      : return opLdImm(mRegs.E); // LD E,n8
        //case op::RRA          : return 1;

        // 0x2*
        //case op::JR_NZ_e8     : return 1;
        //case op::LD_HL_n16    : return 1;
        //case op::LD_inHLp_A   : return 1;
        //case op::INC_HL       : return 1;
        //case op::INC_H        : return 1;
        //case op::DEC_H        : return 1;
        case op::LD_H_n8      : return opLdImm(mRegs.H); // LD H,n8
        //case op::DAA          : return 1;
        //case op::JR_Z_e8      : return 1;
        //case op::ADD_HL_HL    : return 1;
        //case op::LD_A_inHLp   : return 1;
        //case op::DEC_HL       : return 1;
        //case op::INC_L        : return 1;
        //case op::DEC_L        : return 1;
        case op::LD_L_n8      : return opLdImm(mRegs.L); // LD L,n8
        //case op::CPL          : return 1;

        // 0x3*
        //case op::JR_NC_e8     : return 1;
        //case op::LD_SP_n16    : return 1;
        //case op::LD_inHLm_A   : return 1;
        //case op::INC_SP       : return 1;
        //case op::INC_inHL     : return 1;
        //case op::DEC_inHL     : return 1;
        //case op::LD_inHL_n8   : return 1;
        //case op::SCF          : return 1;
        //case op::JR_C_e8      : return 1;
        //case op::ADD_HL_SP    : return 1;
        //case op::LD_A_inHLm   : return 1;
        //case op::DEC_SP       : return 1;
        //case op::INC_A        : return 1;
        //case op::DEC_A        : return 1;
        case op::LD_A_n8      : return opLdImm(mRegs.A); // LD A,n8
        //case op::CCF          : return 1;

        // 0x4*
        case op::LD_B_B       : return opLdReg(mRegs.B, mRegs.B); // LD B,B
        case op::LD_B_C       : return opLdReg(mRegs.B, mRegs.C); // LD B,C
        case op::LD_B_D       : return opLdReg(mRegs.B, mRegs.D); // LD B,D
        case op::LD_B_E       : return opLdReg(mRegs.B, mRegs.E); // LD B,E
        case op::LD_B_H       : return opLdReg(mRegs.B, mRegs.H); // LD B,H
        case op::LD_B_L       : return opLdReg(mRegs.B, mRegs.L); // LD B,L
        case op::LD_B_inHL    : return opLdInd(mRegs.B); // LD B,[HL]
        case op::LD_B_A       : return opLdReg(mRegs.B, mRegs.A); // LD B,A
        case op::LD_C_B       : return opLdReg(mRegs.C, mRegs.B); // LD C,B
        case op::LD_C_C       : return opLdReg(mRegs.C, mRegs.C); // LD C,C
        case op::LD_C_D       : return opLdReg(mRegs.C, mRegs.D); // LD C,D
        case op::LD_C_E       : return opLdReg(mRegs.C, mRegs.E); // LD C,E
        case op::LD_C_H       : return opLdReg(mRegs.C, mRegs.H); // LD C,H
        case op::LD_C_L       : return opLdReg(mRegs.C, mRegs.L); // LD C,L
        case op::LD_C_inHL    : return opLdInd(mRegs.C); // LD C,[HL]
        case op::LD_C_A       : return opLdReg(mRegs.C, mRegs.A); // LD C,A

        // 0x5*
        //case op::LD_D_B       : return 1;
        //case op::LD_D_C       : return 1;
        //case op::LD_D_D       : return 1;
        //case op::LD_D_E       : return 1;
        //case op::LD_D_H       : return 1;
        //case op::LD_D_L       : return 1;
        //case op::LD_D_inHL    : return 1;
        //case op::LD_D_A       : return 1;
        //case op::LD_E_B       : return 1;
        //case op::LD_E_C       : return 1;
        //case op::LD_E_D       : return 1;
        //case op::LD_E_E       : return 1;
        //case op::LD_E_H       : return 1;
        //case op::LD_E_L       : return 1;
        //case op::LD_E_inHL    : return 1;
        //case op::LD_E_A       : return 1;

        // 0x6*
        //case op::LD_H_B       : return 1;
        //case op::LD_H_C       : return 1;
        //case op::LD_H_D       : return 1;
        //case op::LD_H_E       : return 1;
        //case op::LD_H_H       : return 1;
        //case op::LD_H_L       : return 1;
        //case op::LD_H_inHL    : return 1;
        //case op::LD_H_A       : return 1;
        //case op::LD_L_B       : return 1;
        //case op::LD_L_C       : return 1;
        //case op::LD_L_D       : return 1;
        //case op::LD_L_E       : return 1;
        //case op::LD_L_H       : return 1;
        //case op::LD_L_L       : return 1;
        //case op::LD_L_inHL    : return 1;
        //case op::LD_L_A       : return 1;

        // 0x7*
        //case op::LD_inHl_B    : return 1;
        //case op::LD_inHl_C    : return 1;
        //case op::LD_inHl_D    : return 1;
        //case op::LD_inHl_E    : return 1;
        //case op::LD_inHl_H    : return 1;
        //case op::LD_inHl_L    : return 1;
        //case op::HALT         : return 1;
        //case op::LD_inHl_A    : return 1;
        //case op::LD_A_B       : return 1;
        //case op::LD_A_C       : return 1;
        //case op::LD_A_D       : return 1;
        //case op::LD_A_E       : return 1;
        //case op::LD_A_H       : return 1;
        //case op::LD_A_L       : return 1;
        //case op::LD_A_inHL    : return 1;
        //case op::LD_A_A       : return 1;

        // 0x8*
        case op::ADD_A_B      : return opAddReg(mRegs.B); // ADD A,B
        case op::ADD_A_C      : return opAddReg(mRegs.C); // ADD A,C
        case op::ADD_A_D      : return opAddReg(mRegs.D); // ADD A,D
        case op::ADD_A_E      : return opAddReg(mRegs.E); // ADD A,E
        case op::ADD_A_H      : return opAddReg(mRegs.H); // ADD A,H
        case op::ADD_A_L      : return opAddReg(mRegs.L); // ADD A,L
        case op::ADD_A_inHL   : return opAddInd(); // ADD A,[HL]
        case op::ADD_A_A      : return opAddReg(mRegs.A); // ADD A,A
        //case op::ADC_A_B      : return 1;
        //case op::ADC_A_C      : return 1;
        //case op::ADC_A_D      : return 1;
        //case op::ADC_A_E      : return 1;
        //case op::ADC_A_H      : return 1;
        //case op::ADC_A_L      : return 1;
        //case op::ADC_A_inHL   : return 1;
        //case op::ADC_A_A      : return 1;

        // 0x9*
        //case op::SUB_A_B      : return 1;
        //case op::SUB_A_C      : return 1;
        //case op::SUB_A_D      : return 1;
        //case op::SUB_A_E      : return 1;
        //case op::SUB_A_H      : return 1;
        //case op::SUB_A_L      : return 1;
        //case op::SUB_A_inHL   : return 1;
        //case op::SUB_A_A      : return 1;
        //case op::SBC_A_B      : return 1;
        //case op::SBC_A_C      : return 1;
        //case op::SBC_A_D      : return 1;
        //case op::SBC_A_E      : return 1;
        //case op::SBC_A_H      : return 1;
        //case op::SBC_A_L      : return 1;
        //case op::SBC_A_inHL   : return 1;
        //case op::SBC_A_A      : return 1;

        // 0xA*
        //case op::AND_A_B      : return 1;
        //case op::AND_A_C      : return 1;
        //case op::AND_A_D      : return 1;
        //case op::AND_A_E      : return 1;
        //case op::AND_A_H      : return 1;
        //case op::AND_A_L      : return 1;
        //case op::AND_A_inHL   : return 1;
        //case op::AND_A_A      : return 1;
        //case op::XOR_A_B      : return 1;
        //case op::XOR_A_C      : return 1;
        //case op::XOR_A_D      : return 1;
        //case op::XOR_A_E      : return 1;
        //case op::XOR_A_H      : return 1;
        //case op::XOR_A_L      : return 1;
        //case op::XOR_A_inHL   : return 1;
        //case op::XOR_A_A      : return 1;

        // 0xB*
        //case op::OR_A_B       : return 1;
        //case op::OR_A_C       : return 1;
        //case op::OR_A_D       : return 1;
        //case op::OR_A_E       : return 1;
        //case op::OR_A_H       : return 1;
        //case op::OR_A_L       : return 1;
        //case op::OR_A_inHL    : return 1;
        //case op::OR_A_A       : return 1;
        //case op::CP_A_B       : return 1;
        //case op::CP_A_C       : return 1;
        //case op::CP_A_D       : return 1;
        //case op::CP_A_E       : return 1;
        //case op::CP_A_H       : return 1;
        //case op::CP_A_L       : return 1;
        case op::CP_A_inHL    : return opCpInd(); // CP A,[HL]
        //case op::CP_A_A       : return 1;

        // 0xC*
        // case op::RET_NZ       : return 1;
        // case op::POP_BC       : return 1;
        case op::JP_NZ_a16    : return opJpCondImm(!mRegs.getZ()); // JP NZ,a16
        case op::JP_a16       : return opJpImm(); // JP a16
        // case op::CALL_NZ_a16  : return 1;
        // case op::PUSH_BC      : return 1;
        // case op::ADD_A_n8     : return 1;
        // case op::RST_00       : return 1;
        // case op::RET_Z        : return 1;
        // case op::RET          : return 1;
        case op::JP_Z_a16     : return opJpCondImm(mRegs.getZ()); // JP Z,a16
        // case op::CB_PREFIX    : return 1;
        // case op::CALL_Z_a16   : return 1;
        // case op::CALL_a16     : return 1;
        // case op::ADC_A_n8     : return 1;
        // case op::RST_08       : return 1;

        // 0xD*
        // case op::RET_NC       : return 1;
        // case op::POP_DE       : return 1;
        case op::JP_NC_a16    : return opJpCondImm(!mRegs.getC()); // JP NC,a16
        // case op:: 0xD3 not implemented
        // case op::CALL_NC_a16  : return 1;
        // case op::PUSH_DE      : return 1;
        // case op::SUB_A_n8     : return 1;
        // case op::RST_10       : return 1;
        // case op::RET_C        : return 1;
        // case op::RETI         : return 1;
        case op::JP_C_a16     : return opJpCondImm(mRegs.getC()); // JP C,a16
        // case op:: 0xDB not implemented
        // case op::CALL_C_a16   : return 1;
        // case op:: 0xDD not implemented
        // case op::SBC_A_n8     : return 1;
        // case op::RST_18       : return 1;

    // 0xE*
        // case op::LDH_ina8_A   : return 1;
        // case op::POP_HL       : return 1;
        // case op::LD_inC_A     : return 1;
        // case op:: 0xE3 not implemented
        // case op:: 0xE4 not implemented
        // case op::PUSH_HL      : return 1;
        // case op::AND_A_n8     : return 1;
        // case op::RST_20       : return 1;
        // case op::ADD_SP_e8    : return 1;
        case op::JP_HL        : return opJpInd(); // JP HL
        // case op::LD_ina16_A   : return 1;
        // case op:: 0xEB not implemented
        // case op:: 0xEC not implemented
        // case op:: 0xED not implemented
        // case op::XOR_A_n8     : return 1;
        // case op::RST_28       : return 1;

    // 0xF*
        // case op::LDH_A_ina8   : return 1;
        // case op::POP_AF       : return 1;
        // case op::LD_A_inC     : return 1;
        // case op::DI           : return 1;
        // case op:: 0xF4 not implemented
        // case op::PUSH_AF      : return 1;
        // case op::OR_A_n8      : return 1;
        // case op::RST_30       : return 1;
        // case op::LD_HL_SPpe8  : return 1;
        // case op::LD_SP_HL     : return 1;
        // case op::LD_A_ina16   : return 1;
        // case op::EI           : return 1;
        // case op:: 0xFB not implemented
        // case op:: 0xFC not implemented
        // case op::CP_A_n8      : return 1;
        // case op::RST_38       : return 1;

        default:
            // unrecognized opcode
            ok = false;
            return 1;
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




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
    SP = SPinitialValue;
    flags = 0;
}

bool Registers::equalSkipPC(const Registers& other)
{
    return A == other.A
        && B == other.B
        && C == other.C
        && D == other.D
        && E == other.E
        && H == other.H
        && L == other.L
        && SP == other.SP
        && flags == other.flags;
}

bool Registers::equal(const Registers& other)
{
    return equalSkipPC(other) && PC == other.PC;
}




CPU::CPU(Bus& bus)
    : mBus(bus)
{
    reset();
}


void CPU::reset()
{
    mCycles = 0;

    regs.reset();
}

bool CPU::step()
{
    // in one step we execute one instruction, the instruction can take 1 or more machine cycles

    // fetch opcode 
    auto opcode = mBus.read8(regs.PC++);

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
        case op::LD_BC_n16    : return opLdReg16Imm(regs.B, regs.C);
        case op::LD_inBC_A    : return opLdIndReg(regs.BC(), regs.A);
        //case op::INC_BC       : return 1;
        //case op::INC_B        : return 1;
        //case op::DEC_B        : return 1;
        case op::LD_B_n8      : return opLdRegImm(regs.B); // LD B,n8
        //case op::RLCA         : return 1;
        case op::LD_ina16_SP  : return opLdIndImm16Sp();
        //case op::ADD_HL_BC    : return 1;
        case op::LD_A_inBC    : return opLdRegInd(regs.A, regs.BC());
        //case op::DEC_BC       : return 1;
        //case op::INC_C        : return 1;
        //case op::DEC_C        : return 1;
        case op::LD_C_n8      : return opLdRegImm(regs.C); // LD C,n8
        //case op::RRCA         : return 1;

        // 0x1*
        //case op::STOP_n8      : return 1;
        case op::LD_DE_n16    : return opLdReg16Imm(regs.D, regs.E);
        case op::LD_inDE_A    : return opLdIndReg(regs.DE(), regs.A);
        //case op::INC_DE       : return 1;
        //case op::INC_D        : return 1;
        //case op::DEC_D        : return 1;
        case op::LD_D_n8      : return opLdRegImm(regs.D); // LD D,n8
        //case op::RLA          : return 1;
        //case op::JR_e8        : return 1;
        //case op::ADD_HL_DE    : return 1;
        case op::LD_A_inDE    : return opLdRegInd(regs.A, regs.DE());
        //case op::DEC_DE       : return 1;
        //case op::INC_E        : return 1;
        //case op::DEC_E        : return 1;
        case op::LD_E_n8      : return opLdRegImm(regs.E); // LD E,n8
        //case op::RRA          : return 1;

        // 0x2*
        //case op::JR_NZ_e8     : return 1;
        case op::LD_HL_n16    : return opLdReg16Imm(regs.H, regs.L);
        case op::LD_inHLp_A   : return opLdIndIncA();
        //case op::INC_HL       : return 1;
        //case op::INC_H        : return 1;
        //case op::DEC_H        : return 1;
        case op::LD_H_n8      : return opLdRegImm(regs.H); // LD H,n8
        //case op::DAA          : return 1;
        //case op::JR_Z_e8      : return 1;
        //case op::ADD_HL_HL    : return 1;
        case op::LD_A_inHLp   : return opLdAIndInc();
        //case op::DEC_HL       : return 1;
        //case op::INC_L        : return 1;
        //case op::DEC_L        : return 1;
        case op::LD_L_n8      : return opLdRegImm(regs.L); // LD L,n8
        //case op::CPL          : return 1;

        // 0x3*
        //case op::JR_NC_e8     : return 1;
        case op::LD_SP_n16    : return opLdReg16Imm(regs.SP);
        case op::LD_inHLm_A   : return opLdIndDecA();
        //case op::INC_SP       : return 1;
        //case op::INC_inHL     : return 1;
        //case op::DEC_inHL     : return 1;
        case op::LD_inHL_n8   : return opLdIndImm();
        //case op::SCF          : return 1;
        //case op::JR_C_e8      : return 1;
        //case op::ADD_HL_SP    : return 1;
        case op::LD_A_inHLm   : return opLdAIndDec();
        //case op::DEC_SP       : return 1;
        //case op::INC_A        : return 1;
        //case op::DEC_A        : return 1;
        case op::LD_A_n8      : return opLdRegImm(regs.A); // LD A,n8
        //case op::CCF          : return 1;

        // 0x4*
        case op::LD_B_B       : return opLdRegReg(regs.B, regs.B); // LD B,B
        case op::LD_B_C       : return opLdRegReg(regs.B, regs.C); // LD B,C
        case op::LD_B_D       : return opLdRegReg(regs.B, regs.D); // LD B,D
        case op::LD_B_E       : return opLdRegReg(regs.B, regs.E); // LD B,E
        case op::LD_B_H       : return opLdRegReg(regs.B, regs.H); // LD B,H
        case op::LD_B_L       : return opLdRegReg(regs.B, regs.L); // LD B,L
        case op::LD_B_inHL    : return opLdRegInd(regs.B, regs.HL()); // LD B,[HL]
        case op::LD_B_A       : return opLdRegReg(regs.B, regs.A); // LD B,A
        case op::LD_C_B       : return opLdRegReg(regs.C, regs.B); // LD C,B
        case op::LD_C_C       : return opLdRegReg(regs.C, regs.C); // LD C,C
        case op::LD_C_D       : return opLdRegReg(regs.C, regs.D); // LD C,D
        case op::LD_C_E       : return opLdRegReg(regs.C, regs.E); // LD C,E
        case op::LD_C_H       : return opLdRegReg(regs.C, regs.H); // LD C,H
        case op::LD_C_L       : return opLdRegReg(regs.C, regs.L); // LD C,L
        case op::LD_C_inHL    : return opLdRegInd(regs.C, regs.HL()); // LD C,[HL]
        case op::LD_C_A       : return opLdRegReg(regs.C, regs.A); // LD C,A

        // 0x5*
        case op::LD_D_B       : return opLdRegReg(regs.D, regs.B); // LD D,B
        case op::LD_D_C       : return opLdRegReg(regs.D, regs.C); // LD D,C
        case op::LD_D_D       : return opLdRegReg(regs.D, regs.D); // LD D,D
        case op::LD_D_E       : return opLdRegReg(regs.D, regs.E); // LD D,E
        case op::LD_D_H       : return opLdRegReg(regs.D, regs.H); // LD D,H
        case op::LD_D_L       : return opLdRegReg(regs.D, regs.L); // LD D,L
        case op::LD_D_inHL    : return opLdRegInd(regs.D, regs.HL()); // LD A,[HL]
        case op::LD_D_A       : return opLdRegReg(regs.D, regs.A); // LD D,A
        case op::LD_E_B       : return opLdRegReg(regs.E, regs.B); // LD E,B
        case op::LD_E_C       : return opLdRegReg(regs.E, regs.C); // LD E,C
        case op::LD_E_D       : return opLdRegReg(regs.E, regs.D); // LD E,D
        case op::LD_E_E       : return opLdRegReg(regs.E, regs.E); // LD E,E
        case op::LD_E_H       : return opLdRegReg(regs.E, regs.H); // LD E,H
        case op::LD_E_L       : return opLdRegReg(regs.E, regs.L); // LD E,L
        case op::LD_E_inHL    : return opLdRegInd(regs.E, regs.HL()); // LD E,[HL]
        case op::LD_E_A       : return opLdRegReg(regs.E, regs.A); // LD E,A

        // 0x6*
        case op::LD_H_B       : return opLdRegReg(regs.H, regs.B); // LD H,B
        case op::LD_H_C       : return opLdRegReg(regs.H, regs.C); // LD H,C
        case op::LD_H_D       : return opLdRegReg(regs.H, regs.D); // LD H,D
        case op::LD_H_E       : return opLdRegReg(regs.H, regs.E); // LD H,E
        case op::LD_H_H       : return opLdRegReg(regs.H, regs.H); // LD H,H
        case op::LD_H_L       : return opLdRegReg(regs.H, regs.L); // LD H,L
        case op::LD_H_inHL    : return opLdRegInd(regs.H, regs.HL()); // LD H,[HL]
        case op::LD_H_A       : return opLdRegReg(regs.H, regs.A); // LD H,A
        case op::LD_L_B       : return opLdRegReg(regs.L, regs.B); // LD L,B
        case op::LD_L_C       : return opLdRegReg(regs.L, regs.C); // LD L,C
        case op::LD_L_D       : return opLdRegReg(regs.L, regs.D); // LD L,D
        case op::LD_L_E       : return opLdRegReg(regs.L, regs.E); // LD L,E
        case op::LD_L_H       : return opLdRegReg(regs.L, regs.H); // LD L,H
        case op::LD_L_L       : return opLdRegReg(regs.L, regs.L); // LD L,L
        case op::LD_L_inHL    : return opLdRegInd(regs.L, regs.HL()); // LD L,[HL]
        case op::LD_L_A       : return opLdRegReg(regs.L, regs.A); // LD L,A

        // 0x7*
        case op::LD_inHl_B    : return opLdIndReg(regs.HL(), regs.B); // LD [HL],B
        case op::LD_inHl_C    : return opLdIndReg(regs.HL(), regs.C); // LD [HL],C
        case op::LD_inHl_D    : return opLdIndReg(regs.HL(), regs.D); // LD [HL],D
        case op::LD_inHl_E    : return opLdIndReg(regs.HL(), regs.E); // LD [HL],E
        case op::LD_inHl_H    : return opLdIndReg(regs.HL(), regs.H); // LD [HL],H
        case op::LD_inHl_L    : return opLdIndReg(regs.HL(), regs.L); // LD [HL],L
        // case op::HALT         : return 1;
        case op::LD_inHl_A    : return opLdIndReg(regs.HL(), regs.A); // LD [HL],A
        case op::LD_A_B       : return opLdRegReg(regs.A, regs.B); // LD A,B
        case op::LD_A_C       : return opLdRegReg(regs.A, regs.C); // LD A,B
        case op::LD_A_D       : return opLdRegReg(regs.A, regs.D); // LD A,B
        case op::LD_A_E       : return opLdRegReg(regs.A, regs.E); // LD A,B
        case op::LD_A_H       : return opLdRegReg(regs.A, regs.H); // LD A,B
        case op::LD_A_L       : return opLdRegReg(regs.A, regs.L); // LD A,B
        case op::LD_A_inHL    : return opLdRegInd(regs.A, regs.HL()); // LD A,[HL]
        case op::LD_A_A       : return opLdRegReg(regs.A, regs.A); // LD A,A

        // 0x8*
        case op::ADD_A_B      : return opAddReg(regs.B); // ADD A,B
        case op::ADD_A_C      : return opAddReg(regs.C); // ADD A,C
        case op::ADD_A_D      : return opAddReg(regs.D); // ADD A,D
        case op::ADD_A_E      : return opAddReg(regs.E); // ADD A,E
        case op::ADD_A_H      : return opAddReg(regs.H); // ADD A,H
        case op::ADD_A_L      : return opAddReg(regs.L); // ADD A,L
        case op::ADD_A_inHL   : return opAddInd(); // ADD A,[HL]
        case op::ADD_A_A      : return opAddReg(regs.A); // ADD A,A
        case op::ADC_A_B      : return opAdcReg(regs.B);
        case op::ADC_A_C      : return opAdcReg(regs.C);
        case op::ADC_A_D      : return opAdcReg(regs.D);
        case op::ADC_A_E      : return opAdcReg(regs.E);
        case op::ADC_A_H      : return opAdcReg(regs.H);
        case op::ADC_A_L      : return opAdcReg(regs.L);
        case op::ADC_A_inHL   : return opAdcInd();
        case op::ADC_A_A      : return opAdcReg(regs.A);

        // 0x9*
        case op::SUB_A_B      : return opSubReg(regs.B);
        case op::SUB_A_C      : return opSubReg(regs.C);
        case op::SUB_A_D      : return opSubReg(regs.D);
        case op::SUB_A_E      : return opSubReg(regs.E);
        case op::SUB_A_H      : return opSubReg(regs.H);
        case op::SUB_A_L      : return opSubReg(regs.L);
        case op::SUB_A_inHL   : return opSubInd();
        case op::SUB_A_A      : return opSubReg(regs.A);
        case op::SBC_A_B      : return opSbcReg(regs.B);
        case op::SBC_A_C      : return opSbcReg(regs.C);
        case op::SBC_A_D      : return opSbcReg(regs.D);
        case op::SBC_A_E      : return opSbcReg(regs.E);
        case op::SBC_A_H      : return opSbcReg(regs.H);
        case op::SBC_A_L      : return opSbcReg(regs.L);
        case op::SBC_A_inHL   : return opSbcInd();
        case op::SBC_A_A      : return opSbcReg(regs.A);

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
        case op::POP_BC       : return opPopReg16(regs.B, regs.C);
        case op::JP_NZ_a16    : return opJpCondImm(!regs.flags.Z); // JP NZ,a16
        case op::JP_a16       : return opJpImm(); // JP a16
        // case op::CALL_NZ_a16  : return 1;
        case op::PUSH_BC      : return opPushReg16(regs.BC());
        case op::ADD_A_n8     : return opAddImm();
        // case op::RST_00       : return 1;
        // case op::RET_Z        : return 1;
        // case op::RET          : return 1;
        case op::JP_Z_a16     : return opJpCondImm(regs.flags.Z); // JP Z,a16
        // case op::CB_PREFIX    : return 1;
        // case op::CALL_Z_a16   : return 1;
        // case op::CALL_a16     : return 1;
        case op::ADC_A_n8     : return opAdcImm();
        // case op::RST_08       : return 1;

        // 0xD*
        // case op::RET_NC       : return 1;
        case op::POP_DE       : return opPopReg16(regs.D, regs.E);
        case op::JP_NC_a16    : return opJpCondImm(!regs.flags.C); // JP NC,a16
        // case op:: 0xD3 not implemented
        // case op::CALL_NC_a16  : return 1;
        case op::PUSH_DE      : return opPushReg16(regs.DE());
        case op::SUB_A_n8     : return opSubImm();
        // case op::RST_10       : return 1;
        // case op::RET_C        : return 1;
        // case op::RETI         : return 1;
        case op::JP_C_a16     : return opJpCondImm(regs.flags.C); // JP C,a16
        // case op:: 0xDB not implemented
        // case op::CALL_C_a16   : return 1;
        // case op:: 0xDD not implemented
        case op::SBC_A_n8     : return opSbcImm();
        // case op::RST_18       : return 1;

        // 0xE*
        case op::LDH_ina8_A   : return opLdIndImm8Reg();
        case op::POP_HL       : return opPopReg16(regs.H, regs.L);
        case op::LDH_inC_A    : return opLdIndReg(0xFF00 + regs.C, regs.A); // LD [C],A  (address is 0xff00 + C)
        // case op:: 0xE3 not implemented
        // case op:: 0xE4 not implemented
        case op::PUSH_HL      : return opPushReg16(regs.HL());
        // case op::AND_A_n8     : return 1;
        // case op::RST_20       : return 1;
        // case op::ADD_SP_e8    : return 1;
        case op::JP_HL        : return opJpInd(); // JP HL
        case op::LD_ina16_A   : return opLdIndImm16Reg();
        // case op:: 0xEB not implemented
        // case op:: 0xEC not implemented
        // case op:: 0xED not implemented
        // case op::XOR_A_n8     : return 1;
        // case op::RST_28       : return 1;

        // 0xF*
        case op::LDH_A_ina8   : return opLdRegIndImm8();
        case op::POP_AF       : return opPopReg16(regs.A, regs.flags);
        case op::LDH_A_inC    : return opLdRegInd(regs.A, 0xFF00 + regs.C);  // LD A,[C]  (actual address is 0xff00 + C)
        // case op::DI           : return 1;
        // case op:: 0xF4 not implemented
        case op::PUSH_AF      : return opPushReg16(regs.AF());
        // case op::OR_A_n8      : return 1;
        // case op::RST_30       : return 1;
        // case op::LD_HL_SPpe8  : return 1;
        case op::LD_SP_HL     : return opLdSpHl();
        case op::LD_A_ina16   : return opLdRegIndImm16();
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
}

uint8_t CPU::opLdRegImm(uint8_t& dst)
{
    // immediate load into register
    // e.g.: LD A,n8
    // 2 cycles required
    dst = mBus.read8(regs.PC++);
    return 2;
}


uint8_t CPU::opLdRegReg(uint8_t& dst, const uint8_t& src)
{
    // Load register into another register (too many opcodes)
    // 1 cycle required
    dst = src;
    return 1;
}

uint8_t CPU::opLdRegInd(uint8_t& dst, uint16_t srcAddr)
{
    // load memory into a register, address could be stored in HL, BC, DE
    // 2 cycles required
    dst = mBus.read8(srcAddr);
    return 2;
}

uint8_t CPU::opLdIndReg(uint16_t dstAddr, const uint8_t& src)
{
    // load memory into register, address is stored in HL
    // 2 cycles required
    mBus.write8(dstAddr, src);
    return 2;
}

uint8_t CPU::opLdIndImm()
{   
    // load immediate into memory
    // LD [HL],n8
    mBus.write8(regs.HL(), mBus.read8(regs.PC++));

    return 3;
}

uint8_t CPU::opLdRegIndImm16()
{
    // load in register from memory at immediate address (only works with the A register)
    // LD A,[a16]
    uint16_t addr = mBus.read8(regs.PC++);
    addr |= mBus.read8(regs.PC++) << 8;

    regs.A = mBus.read8(addr);

    return 4;
}

uint8_t CPU::opLdIndImm16Reg()
{
    // load in memory at immediate address from register (only works with the A register)
    // LD [a16],A
    uint16_t addr = mBus.read8(regs.PC++);
    addr |= mBus.read8(regs.PC++) << 8;
    
    mBus.write8(addr, regs.A);

    return 4;
}

uint8_t CPU::opLdRegIndImm8()
{
    // load in register from memory at immediate 8-bit address
    // only works with the A register, the actual address is 0xff00 + a8
    // LDH A,[a8]
    auto addrLsb = mBus.read8(regs.PC++);
    regs.A = mBus.read8(0xff00 + addrLsb);

    return 3;
}

uint8_t CPU::opLdIndImm8Reg()
{
    // load in memory at immediate 8-bit address from register
    // only works with the A register, the actual address is 0xff00 + a8
    // LDH [a8],A
    auto addrLsb = mBus.read8(regs.PC++);
    mBus.write8(0xff00 + addrLsb, regs.A);

    return 3;
}

uint8_t CPU::opLdAIndDec()
{
    // load in A the value in mem[HL] and decrement the value of HL
    // LD A,[HL-]
    auto addr = regs.HL();
    regs.A = mBus.read8(addr);
    regs.setHL(addr - 1);

    return 2;
}

uint8_t CPU::opLdAIndInc()
{
    // load in A the value in mem[HL] and increment the value of HL
    // LD A,[HL+]
    auto addr = regs.HL();
    regs.A = mBus.read8(addr);
    regs.setHL(addr + 1);

    return 2;
}

uint8_t CPU::opLdIndDecA()
{
    // load in mem[HL] the value of A and decrement the value of HL
    // LD [HL-],A
    auto addr = regs.HL();
    mBus.write8(addr, regs.A);
    regs.setHL(addr - 1);

    return 2;
}

uint8_t CPU::opLdIndIncA()
{
    // load in mem[HL] the value of A and increment the value of HL
    // LD [HL+],A
    auto addr = regs.HL();
    mBus.write8(addr, regs.A);
    regs.setHL(addr + 1);

    return 2;
}

uint8_t CPU::opLdReg16Imm(uint8_t& msb, uint8_t& lsb)
{
    // load immediate 16-bit value into BC, DE or HL
    // e.g.: LD BC,n16    
    lsb = mBus.read8(regs.PC++);
    msb = mBus.read8(regs.PC++);

    return 3;
}

uint8_t CPU::opLdReg16Imm(uint16_t& dst)
{
    // load immediate 16-bit value into SP
    // e.g.: LD SP,n16
    uint16_t val = mBus.read8(regs.PC++);
    val |= mBus.read8(regs.PC++) << 8;

    dst = val;

    return 3;
}

uint8_t CPU::opLdIndImm16Sp()
{
    // load the SP value in memory to the immediate 16-bit address
    // LD [a16],SP
    uint16_t addr = mBus.read8(regs.PC++);
    addr |= mBus.read8(regs.PC++) << 8;

    mBus.write16(addr, regs.SP);

    return 5;
}

uint8_t CPU::opLdSpHl()
{
    // load the HL value into the SP register
    // LD SP,HL
    regs.SP = regs.HL();

    return 2;
}

uint8_t CPU::opPushReg16(uint16_t val)
{
    // push the register onto the stack
    // e.g.: PUSH BC

    regs.SP -= 2;
    mBus.write16(regs.SP, val);

    return 4;
}





uint8_t CPU::opAdd8Common(uint8_t rhs, uint8_t cycles)
{
    uint16_t res = regs.A + rhs;

    // check flags
    regs.flags.Z = (uint8_t)res == 0;
    regs.flags.H = checkHalfCarry(regs.A, rhs);
    regs.flags.N = false;
    regs.flags.C = checkCarry(res);

    regs.A = (uint8_t)res;

    return cycles;
}

uint8_t CPU::opAddReg(uint8_t reg)
{
    // opcodes 0x80..0x85
    // addition between registers, sum A and another register, store the result in A
    // and set the appropriate flags
    // 0x80: ADD A,B
    // 0x81: ADD A,C
    // 0x82: ADD A,D
    // 0x83: ADD A,E
    // 0x84: ADD A,H
    // 0x85: ADD A,L
    // 0x87: ADD A,A
    // 1 cycle required
    return opAdd8Common(reg, 1);
}

uint8_t CPU::opAddInd()
{
    // ADD A,[HL]
    // A = A + mem[HL], 2 cycles
  
    return opAdd8Common(mBus.read8(regs.HL()), 2);
}

uint8_t CPU::opAddImm()
{
    // ADD A,n8
    // 2 cycles

    return opAdd8Common(mBus.read8(regs.PC++), 2);
}



uint8_t CPU::opAdcCommon(uint8_t rhs, uint8_t cycles)
{
    uint16_t res = regs.A + rhs + regs.flags.C;

    // check flags
    regs.flags.Z = (uint8_t)res == 0;
    regs.flags.H = checkHalfCarry(regs.A, rhs, regs.flags.C);
    regs.flags.N = false;
    regs.flags.C = checkCarry(res);

    regs.A = (uint8_t)res;

    return cycles;
}

uint8_t CPU::opAdcReg(uint8_t reg)
{
    // addition between registers and carry:
    // sum A, the carry flag and another register, store the result in A
    // and set the appropriate flags
    // ADC A,A
    // ADC A,B
    // ADC A,C
    // ADC A,D
    // ADC A,E
    // ADC A,H
    // ADC A,L
    // 1 cycle required

    return opAdcCommon(reg, 1);
}

uint8_t CPU::opAdcInd()
{
    // ADC A,[HL]
    // A = A + carry flag + mem[HL], 2 cycles
    
    return opAdcCommon(mBus.read8(regs.HL()), 2);
}

uint8_t CPU::opAdcImm()
{
    // ADC A,n8
    // 2 cycles
    
    return opAdcCommon(mBus.read8(regs.PC++), 2);
}



uint8_t CPU::opSubCommon(uint8_t rhs, uint8_t cycles)
{
    // generic subtraction, lhs is always reg A
    int16_t res = regs.A - rhs;

    // set N because a subtraction just happened
    regs.flags.Z = (uint8_t)res == 0;
    regs.flags.H = checkHalfBorrow(regs.A, rhs);
    regs.flags.N = true;
    regs.flags.C = checkBorrow(regs.A, rhs);

    regs.A = (uint8_t)res;

    return cycles;
}

uint8_t CPU::opSubReg(uint8_t reg)
{
    // SUB A,reg
    // it performs A = A - reg
    // all flags are updated accordingly

    return opSubCommon(reg, 1);
}

uint8_t CPU::opSubInd()
{
    // SUB A,[HL]
    // it performs A = A - mem[HL]
    // all flags are updated accordingly
    
    return opSubCommon(mBus.read8(regs.HL()), 2);
}

uint8_t CPU::opSubImm()
{
    // SUB A,n8
    // it performs A = A - val
    // all flags are updated accordingly

    return opSubCommon(mBus.read8(regs.PC++), 2);
}

uint8_t CPU::opSbcCommon(uint8_t rhs, uint8_t cycles)
{
    // in SBC we subtract an operand from reg A and we also subtract 1 if
    // the C flag is active, the result is stored in A
    // e.g.: SBC A,0x12  -->  A = A - 0x12 - carry

    int16_t res = regs.A - (rhs + regs.flags.C);

    // set N because a subtraction just happened
    regs.flags.Z = (uint8_t)res == 0;
    regs.flags.H = checkHalfBorrow(regs.A, rhs, regs.flags.C);
    regs.flags.N = true;
    regs.flags.C = checkBorrow(regs.A, rhs);

    regs.A = (uint8_t)res;

    return cycles;
}

uint8_t CPU::opSbcReg(uint8_t reg)
{
    // SBC A,reg

    return opSbcCommon(reg, 1);
}

uint8_t CPU::opSbcInd()
{
    // SBC A,[HL]

    return opSbcCommon(mBus.read8(regs.HL()), 2);
}

uint8_t CPU::opSbcImm()
{
    // SBC A,n8

    return opSbcCommon(mBus.read8(regs.PC++), 2);
}



uint8_t CPU::opJpImm()
{
    // JP a16
    // immediate unconditional jump, jump to address specified in the instruction 
    // len: 3 bytes
    // cycles: 4
    uint16_t addr = mBus.read8(regs.PC++);
    addr |= mBus.read8(regs.PC++) << 8;

    regs.PC = addr;
    return 4;
}

uint8_t CPU::opJpInd()
{
    // JP HL
    // unconditional jump, jump to address specified in HL (NOT IN mem[HL]!!)
    regs.PC = regs.HL();
    return 1;
}

uint8_t CPU::opJpCondImm(bool cond)
{
    // JP Z,a16
    // immediate conditional jump, jump to address specified in the instruction if condition is true (aka flag is set)
    // len: 3 bytes
    // cycles: 3 if branch not taken, 4 if taken

    uint16_t addr = mBus.read8(regs.PC++);
    addr |= mBus.read8(regs.PC++) << 8;
    if(cond) {
        regs.PC = addr;
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
    auto rhs = compl2(mBus.read8(regs.HL()));
    uint16_t res = regs.A + rhs;

    // set N because a subtraction just happened
    regs.flags.Z = (uint8_t)res == 0;
    regs.flags.H = checkHalfCarry(regs.A, rhs);
    regs.flags.N = true;
    regs.flags.C = checkCarry(res);

    return 2;
}





#include "Debug.h"
#include "gb/Opcodes.h"


static constexpr char hexDict[] = {
        '0', '1', '2', '3', '4', '5', '6' , '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};


std::string uintToHex(uint8_t val)
{
    std::string ret = "$00";

    ret[1] = hexDict[(val >> 4) & 0x0F];
    ret[2] = hexDict[val & 0x0F];

    return ret;
}

std::string uintToHex(uint16_t val)
{
    std::string ret = "$0000";

    ret[1] = hexDict[(val >> 12) & 0x0F];
    ret[2] = hexDict[(val >> 8) & 0x0F];
    ret[3] = hexDict[(val >> 4) & 0x0F];
    ret[4] = hexDict[val & 0x0F];

    return ret;
}



static std::string opJpImmToStr(const Bus& bus, uint16_t pc)
{
    uint16_t addr = bus.read16(pc);

    return std::string("jp ") + uintToHex(addr);
}


std::string instructionToStr(const Bus& bus, uint16_t pc)
{
    // opcode reference: 
    // - https://gbdev.io/gb-opcodes/optables/
    // - https://gekkio.fi/files/gb-docs/gbctr.pdf

    uint8_t opcode = bus.read8(pc++);

    switch (opcode) {
    // 0x0*
    case op::NOP: return "nop";
    //case op::LD_BC_n16: return opLdReg16Imm(regs.B, regs.C);
    //case op::LD_inBC_A: return opLdIndReg(regs.BC(), regs.A);
    //case op::INC_BC: return opIncReg16(regs.B, regs.C);
    //case op::INC_B: return opIncReg(regs.B);
    //case op::DEC_B: return opDecReg(regs.B);
    //case op::LD_B_n8: return opLdRegImm(regs.B); // LD B,n8
    //case op::RLCA: return opRlca();
    //case op::LD_ina16_SP: return opLdIndImm16Sp();
    //case op::ADD_HL_BC: return opAddReg16(regs.BC());
    //case op::LD_A_inBC: return opLdRegInd(regs.A, regs.BC());
    //case op::DEC_BC: return opDecReg16(regs.B, regs.C);
    //case op::INC_C: return opIncReg(regs.C);
    //case op::DEC_C: return opDecReg(regs.C);
    //case op::LD_C_n8: return opLdRegImm(regs.C); // LD C,n8
    //case op::RRCA: return opRrca();

    //    // 0x1*
    //case op::STOP: return opStop();
    //case op::LD_DE_n16: return opLdReg16Imm(regs.D, regs.E);
    //case op::LD_inDE_A: return opLdIndReg(regs.DE(), regs.A);
    //case op::INC_DE: return opIncReg16(regs.D, regs.E);
    //case op::INC_D: return opIncReg(regs.D);
    //case op::DEC_D: return opDecReg(regs.D);
    //case op::LD_D_n8: return opLdRegImm(regs.D); // LD D,n8
    //case op::RLA: return opRla();
    //case op::JR_e8: return opJrImm();
    //case op::ADD_HL_DE: return opAddReg16(regs.DE());
    //case op::LD_A_inDE: return opLdRegInd(regs.A, regs.DE());
    //case op::DEC_DE: return opDecReg16(regs.D, regs.E);
    //case op::INC_E: return opIncReg(regs.E);
    //case op::DEC_E: return opDecReg(regs.E);
    //case op::LD_E_n8: return opLdRegImm(regs.E); // LD E,n8
    //case op::RRA: return opRra();

    //    // 0x2*
    //case op::JR_NZ_e8: return opJrCond(!regs.flags.Z);
    //case op::LD_HL_n16: return opLdReg16Imm(regs.H, regs.L);
    //case op::LD_inHLp_A: return opLdIndIncA();
    //case op::INC_HL: return opIncReg16(regs.H, regs.L);
    //case op::INC_H: return opIncReg(regs.H);
    //case op::DEC_H: return opDecReg(regs.H);
    //case op::LD_H_n8: return opLdRegImm(regs.H); // LD H,n8
    //case op::DAA: return opDaa();
    //case op::JR_Z_e8: return opJrCond(regs.flags.Z);
    //case op::ADD_HL_HL: return opAddReg16(regs.HL());
    //case op::LD_A_inHLp: return opLdAIndInc();
    //case op::DEC_HL: return opDecReg16(regs.H, regs.L);
    //case op::INC_L: return opIncReg(regs.L);
    //case op::DEC_L: return opDecReg(regs.L);
    //case op::LD_L_n8: return opLdRegImm(regs.L); // LD L,n8
    //case op::CPL: return opCpl();

    //    // 0x3*
    //case op::JR_NC_e8: return opJrCond(!regs.flags.C);
    //case op::LD_SP_n16: return opLdReg16Imm(regs.SP);
    //case op::LD_inHLm_A: return opLdIndDecA();
    //case op::INC_SP: return opIncSp();
    //case op::INC_inHL: return opIncInd();
    //case op::DEC_inHL: return opDecInd();
    //case op::LD_inHL_n8: return opLdIndImm();
    //case op::SCF: return opScf();
    //case op::JR_C_e8: return opJrCond(regs.flags.C);
    //case op::ADD_HL_SP: return opAddReg16(regs.SP);
    //case op::LD_A_inHLm: return opLdAIndDec();
    //case op::DEC_SP: return opDecSp();
    //case op::INC_A: return opIncReg(regs.A);
    //case op::DEC_A: return opDecReg(regs.A);
    //case op::LD_A_n8: return opLdRegImm(regs.A); // LD A,n8
    //case op::CCF: return opCcf();

    //    // 0x4*
    //case op::LD_B_B: return opLdRegReg(regs.B, regs.B); // LD B,B
    //case op::LD_B_C: return opLdRegReg(regs.B, regs.C); // LD B,C
    //case op::LD_B_D: return opLdRegReg(regs.B, regs.D); // LD B,D
    //case op::LD_B_E: return opLdRegReg(regs.B, regs.E); // LD B,E
    //case op::LD_B_H: return opLdRegReg(regs.B, regs.H); // LD B,H
    //case op::LD_B_L: return opLdRegReg(regs.B, regs.L); // LD B,L
    //case op::LD_B_inHL: return opLdRegInd(regs.B, regs.HL()); // LD B,[HL]
    //case op::LD_B_A: return opLdRegReg(regs.B, regs.A); // LD B,A
    //case op::LD_C_B: return opLdRegReg(regs.C, regs.B); // LD C,B
    //case op::LD_C_C: return opLdRegReg(regs.C, regs.C); // LD C,C
    //case op::LD_C_D: return opLdRegReg(regs.C, regs.D); // LD C,D
    //case op::LD_C_E: return opLdRegReg(regs.C, regs.E); // LD C,E
    //case op::LD_C_H: return opLdRegReg(regs.C, regs.H); // LD C,H
    //case op::LD_C_L: return opLdRegReg(regs.C, regs.L); // LD C,L
    //case op::LD_C_inHL: return opLdRegInd(regs.C, regs.HL()); // LD C,[HL]
    //case op::LD_C_A: return opLdRegReg(regs.C, regs.A); // LD C,A

    //    // 0x5*
    //case op::LD_D_B: return opLdRegReg(regs.D, regs.B); // LD D,B
    //case op::LD_D_C: return opLdRegReg(regs.D, regs.C); // LD D,C
    //case op::LD_D_D: return opLdRegReg(regs.D, regs.D); // LD D,D
    //case op::LD_D_E: return opLdRegReg(regs.D, regs.E); // LD D,E
    //case op::LD_D_H: return opLdRegReg(regs.D, regs.H); // LD D,H
    //case op::LD_D_L: return opLdRegReg(regs.D, regs.L); // LD D,L
    //case op::LD_D_inHL: return opLdRegInd(regs.D, regs.HL()); // LD A,[HL]
    //case op::LD_D_A: return opLdRegReg(regs.D, regs.A); // LD D,A
    //case op::LD_E_B: return opLdRegReg(regs.E, regs.B); // LD E,B
    //case op::LD_E_C: return opLdRegReg(regs.E, regs.C); // LD E,C
    //case op::LD_E_D: return opLdRegReg(regs.E, regs.D); // LD E,D
    //case op::LD_E_E: return opLdRegReg(regs.E, regs.E); // LD E,E
    //case op::LD_E_H: return opLdRegReg(regs.E, regs.H); // LD E,H
    //case op::LD_E_L: return opLdRegReg(regs.E, regs.L); // LD E,L
    //case op::LD_E_inHL: return opLdRegInd(regs.E, regs.HL()); // LD E,[HL]
    //case op::LD_E_A: return opLdRegReg(regs.E, regs.A); // LD E,A

    //    // 0x6*
    //case op::LD_H_B: return opLdRegReg(regs.H, regs.B); // LD H,B
    //case op::LD_H_C: return opLdRegReg(regs.H, regs.C); // LD H,C
    //case op::LD_H_D: return opLdRegReg(regs.H, regs.D); // LD H,D
    //case op::LD_H_E: return opLdRegReg(regs.H, regs.E); // LD H,E
    //case op::LD_H_H: return opLdRegReg(regs.H, regs.H); // LD H,H
    //case op::LD_H_L: return opLdRegReg(regs.H, regs.L); // LD H,L
    //case op::LD_H_inHL: return opLdRegInd(regs.H, regs.HL()); // LD H,[HL]
    //case op::LD_H_A: return opLdRegReg(regs.H, regs.A); // LD H,A
    //case op::LD_L_B: return opLdRegReg(regs.L, regs.B); // LD L,B
    //case op::LD_L_C: return opLdRegReg(regs.L, regs.C); // LD L,C
    //case op::LD_L_D: return opLdRegReg(regs.L, regs.D); // LD L,D
    //case op::LD_L_E: return opLdRegReg(regs.L, regs.E); // LD L,E
    //case op::LD_L_H: return opLdRegReg(regs.L, regs.H); // LD L,H
    //case op::LD_L_L: return opLdRegReg(regs.L, regs.L); // LD L,L
    //case op::LD_L_inHL: return opLdRegInd(regs.L, regs.HL()); // LD L,[HL]
    //case op::LD_L_A: return opLdRegReg(regs.L, regs.A); // LD L,A

    //    // 0x7*
    //case op::LD_inHl_B: return opLdIndReg(regs.HL(), regs.B); // LD [HL],B
    //case op::LD_inHl_C: return opLdIndReg(regs.HL(), regs.C); // LD [HL],C
    //case op::LD_inHl_D: return opLdIndReg(regs.HL(), regs.D); // LD [HL],D
    //case op::LD_inHl_E: return opLdIndReg(regs.HL(), regs.E); // LD [HL],E
    //case op::LD_inHl_H: return opLdIndReg(regs.HL(), regs.H); // LD [HL],H
    //case op::LD_inHl_L: return opLdIndReg(regs.HL(), regs.L); // LD [HL],L
    //case op::HALT: return opHalt();
    //case op::LD_inHl_A: return opLdIndReg(regs.HL(), regs.A); // LD [HL],A
    //case op::LD_A_B: return opLdRegReg(regs.A, regs.B); // LD A,B
    //case op::LD_A_C: return opLdRegReg(regs.A, regs.C); // LD A,B
    //case op::LD_A_D: return opLdRegReg(regs.A, regs.D); // LD A,B
    //case op::LD_A_E: return opLdRegReg(regs.A, regs.E); // LD A,B
    //case op::LD_A_H: return opLdRegReg(regs.A, regs.H); // LD A,B
    //case op::LD_A_L: return opLdRegReg(regs.A, regs.L); // LD A,B
    //case op::LD_A_inHL: return opLdRegInd(regs.A, regs.HL()); // LD A,[HL]
    //case op::LD_A_A: return opLdRegReg(regs.A, regs.A); // LD A,A

    //    // 0x8*
    //case op::ADD_A_B: return opAddReg(regs.B); // ADD A,B
    //case op::ADD_A_C: return opAddReg(regs.C); // ADD A,C
    //case op::ADD_A_D: return opAddReg(regs.D); // ADD A,D
    //case op::ADD_A_E: return opAddReg(regs.E); // ADD A,E
    //case op::ADD_A_H: return opAddReg(regs.H); // ADD A,H
    //case op::ADD_A_L: return opAddReg(regs.L); // ADD A,L
    //case op::ADD_A_inHL: return opAddInd(); // ADD A,[HL]
    //case op::ADD_A_A: return opAddReg(regs.A); // ADD A,A
    //case op::ADC_A_B: return opAdcReg(regs.B);
    //case op::ADC_A_C: return opAdcReg(regs.C);
    //case op::ADC_A_D: return opAdcReg(regs.D);
    //case op::ADC_A_E: return opAdcReg(regs.E);
    //case op::ADC_A_H: return opAdcReg(regs.H);
    //case op::ADC_A_L: return opAdcReg(regs.L);
    //case op::ADC_A_inHL: return opAdcInd();
    //case op::ADC_A_A: return opAdcReg(regs.A);

    //    // 0x9*
    //case op::SUB_A_B: return opSubReg(regs.B);
    //case op::SUB_A_C: return opSubReg(regs.C);
    //case op::SUB_A_D: return opSubReg(regs.D);
    //case op::SUB_A_E: return opSubReg(regs.E);
    //case op::SUB_A_H: return opSubReg(regs.H);
    //case op::SUB_A_L: return opSubReg(regs.L);
    //case op::SUB_A_inHL: return opSubInd();
    //case op::SUB_A_A: return opSubReg(regs.A);
    //case op::SBC_A_B: return opSbcReg(regs.B);
    //case op::SBC_A_C: return opSbcReg(regs.C);
    //case op::SBC_A_D: return opSbcReg(regs.D);
    //case op::SBC_A_E: return opSbcReg(regs.E);
    //case op::SBC_A_H: return opSbcReg(regs.H);
    //case op::SBC_A_L: return opSbcReg(regs.L);
    //case op::SBC_A_inHL: return opSbcInd();
    //case op::SBC_A_A: return opSbcReg(regs.A);

    //    // 0xA*
    //case op::AND_A_B: return opAndReg(regs.B);
    //case op::AND_A_C: return opAndReg(regs.C);
    //case op::AND_A_D: return opAndReg(regs.D);
    //case op::AND_A_E: return opAndReg(regs.E);
    //case op::AND_A_H: return opAndReg(regs.H);
    //case op::AND_A_L: return opAndReg(regs.L);
    //case op::AND_A_inHL: return opAndInd();
    //case op::AND_A_A: return opAndReg(regs.A);
    //case op::XOR_A_B: return opXorReg(regs.B);
    //case op::XOR_A_C: return opXorReg(regs.C);
    //case op::XOR_A_D: return opXorReg(regs.D);
    //case op::XOR_A_E: return opXorReg(regs.E);
    //case op::XOR_A_H: return opXorReg(regs.H);
    //case op::XOR_A_L: return opXorReg(regs.L);
    //case op::XOR_A_inHL: return opXorInd();
    //case op::XOR_A_A: return opXorReg(regs.A);

    //    // 0xB*
    //case op::OR_A_B: return opOrReg(regs.B);
    //case op::OR_A_C: return opOrReg(regs.C);
    //case op::OR_A_D: return opOrReg(regs.D);
    //case op::OR_A_E: return opOrReg(regs.E);
    //case op::OR_A_H: return opOrReg(regs.H);
    //case op::OR_A_L: return opOrReg(regs.L);
    //case op::OR_A_inHL: return opOrInd();
    //case op::OR_A_A: return opOrReg(regs.A);
    //case op::CP_A_B: return opCpReg(regs.B);
    //case op::CP_A_C: return opCpReg(regs.C);
    //case op::CP_A_D: return opCpReg(regs.D);
    //case op::CP_A_E: return opCpReg(regs.E);
    //case op::CP_A_H: return opCpReg(regs.H);
    //case op::CP_A_L: return opCpReg(regs.L);
    //case op::CP_A_inHL: return opCpInd(); // CP A,[HL]
    //case op::CP_A_A: return opCpReg(regs.A);

    //    // 0xC*
    //case op::RET_NZ: return opRetCond(!regs.flags.Z);
    //case op::POP_BC: return opPopReg16(regs.B, regs.C);
    //case op::JP_NZ_a16: return opJpCond(!regs.flags.Z); // JP NZ,a16
    case op::JP_a16: return opJpImmToStr(bus, pc); // JP a16
    //case op::CALL_NZ_a16: return opCallCond(!regs.flags.Z);
    //case op::PUSH_BC: return opPushReg16(regs.BC());
    //case op::ADD_A_n8: return opAddImm();
    //case op::RST_00: return opRst(0x00);
    //case op::RET_Z: return opRetCond(regs.flags.Z);
    //case op::RET: return opRet();
    //case op::JP_Z_a16: return opJpCond(regs.flags.Z); // JP Z,a16
    //case op::CB_PREFIX: return executeCb(ok);
    //case op::CALL_Z_a16: return opCallCond(regs.flags.Z);
    //case op::CALL_a16: return opCallImm();
    //case op::ADC_A_n8: return opAdcImm();
    //case op::RST_08: return opRst(0x08);

    //    // 0xD*
    //case op::RET_NC: return opRetCond(!regs.flags.C);
    //case op::POP_DE: return opPopReg16(regs.D, regs.E);
    //case op::JP_NC_a16: return opJpCond(!regs.flags.C); // JP NC,a16
    //    // case op:: 0xD3 not implemented
    //case op::CALL_NC_a16: return opCallCond(!regs.flags.C);
    //case op::PUSH_DE: return opPushReg16(regs.DE());
    //case op::SUB_A_n8: return opSubImm();
    //case op::RST_10: return opRst(0x10);
    //case op::RET_C: return opRetCond(regs.flags.C);
    //case op::RETI: return opReti();
    //case op::JP_C_a16: return opJpCond(regs.flags.C); // JP C,a16
    //    // case op:: 0xDB not implemented
    //case op::CALL_C_a16: return opCallCond(regs.flags.C);
    //    // case op:: 0xDD not implemented
    //case op::SBC_A_n8: return opSbcImm();
    //case op::RST_18: return opRst(0x18);

    //    // 0xE*
    //case op::LDH_ina8_A: return opLdIndImm8Reg();
    //case op::POP_HL: return opPopReg16(regs.H, regs.L);
    //case op::LDH_inC_A: return opLdIndReg(0xFF00 + regs.C, regs.A); // LD [C],A  (address is 0xff00 + C)
    //    // case op:: 0xE3 not implemented
    //    // case op:: 0xE4 not implemented
    //case op::PUSH_HL: return opPushReg16(regs.HL());
    //case op::AND_A_n8: return opAndImm();
    //case op::RST_20: return opRst(0x20);
    //case op::ADD_SP_e8: return opAddSpImm();
    //case op::JP_HL: return opJpHL(); // JP HL
    //case op::LD_ina16_A: return opLdIndImm16Reg();
    //    // case op:: 0xEB not implemented
    //    // case op:: 0xEC not implemented
    //    // case op:: 0xED not implemented
    //case op::XOR_A_n8: return opXorImm();
    //case op::RST_28: return opRst(0x28);

    //    // 0xF*
    //case op::LDH_A_ina8: return opLdRegIndImm8();
    //case op::POP_AF: return opPopReg16(regs.A, regs.flags);
    //case op::LDH_A_inC: return opLdRegInd(regs.A, 0xFF00 + regs.C);  // LD A,[C]  (actual address is 0xff00 + C)
    //case op::DI: return opDi();
    //    // case op:: 0xF4 not implemented
    //case op::PUSH_AF: return opPushReg16(regs.AF());
    //case op::OR_A_n8: return opOrImm();
    //case op::RST_30: return opRst(0x30);
    //case op::LD_HL_SPpe8: return opLdHlSpOffset();
    //case op::LD_SP_HL: return opLdSpHl();
    //case op::LD_A_ina16: return opLdRegIndImm16();
    //case op::EI: return opEi();
    //    // case op:: 0xFB not implemented
    //    // case op:: 0xFC not implemented
    //case op::CP_A_n8: return opCpImm();
    //case op::RST_38: return opRst(0x38);

    default:
        // unrecognized opcode
        return "unrecognized";
    }
}
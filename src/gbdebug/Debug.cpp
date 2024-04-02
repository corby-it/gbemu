

#include "Debug.h"
#include "gb/Opcodes.h"
#include "gb/GameBoyCore.h"
#include <string>



GBDebug::GBDebug()
    : enabled(true)
    , breakOnLdbb(false)
    , breakOnRet(false)
    , targetCallNesting(0)
{}






static constexpr char hexDict[] = {
        '0', '1', '2', '3', '4', '5', '6' , '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};


static std::string uintToHex(uint8_t val)
{
    std::string ret = "$00";

    ret[1] = hexDict[(val >> 4) & 0x0F];
    ret[2] = hexDict[val & 0x0F];

    return ret;
}

static std::string uintToHex(uint16_t val)
{
    std::string ret = "$0000";

    ret[1] = hexDict[(val >> 12) & 0x0F];
    ret[2] = hexDict[(val >> 8) & 0x0F];
    ret[3] = hexDict[(val >> 4) & 0x0F];
    ret[4] = hexDict[val & 0x0F];

    return ret;
}


static std::string immValU8(const Bus& bus, uint16_t pc)
{
    return uintToHex(bus.read8(pc));
}

static std::string immValS8(const Bus& bus, uint16_t pc)
{
    return std::to_string((int8_t)bus.read8(pc));
}



std::string GBDebug::symbolOrU16(const GameBoyClassic& gb, uint16_t pc)
{
    uint16_t addr = gb.bus.read16(pc);

    auto sym = symTable.getSymbol(gb.cartridge.mbc->getRomBankId(), gb.cartridge.mbc->getRamBankId(), addr);

    return sym ? *sym : uintToHex(addr);
}

std::string GBDebug::symbolOrS8(const GameBoyClassic& gb, uint16_t pc)
{
    int8_t offset = (int8_t)gb.bus.read8(pc++);
    uint16_t addr = pc + offset;

    auto sym = symTable.getSymbol(gb.cartridge.mbc->getRomBankId(), gb.cartridge.mbc->getRamBankId(), addr);

    return sym ? *sym : std::to_string(offset);
}




std::string GBDebug::updateInstructionToStr(const GameBoyClassic& gb)
{
    mCurrInstruction = instructionToStr(gb);
    return mCurrInstruction;
}



std::string GBDebug::instructionToStr(const GameBoyClassic& gb)
{
    // opcode reference: 
    // - https://gbdev.io/gb-opcodes/optables/
    // - https://gekkio.fi/files/gb-docs/gbctr.pdf

    uint16_t pc = gb.cpu.regs.PC;
    const auto& bus = gb.bus;
    
    uint8_t opcode = bus.read8(pc++);

    std::string ret("(");
    ret += uintToHex(opcode) + ")     ";

    switch (opcode) {
    // 0x0*
    case op::NOP: return ret + "nop";
    case op::LD_BC_n16: return ret + "ld bc, " + symbolOrU16(gb, pc);
    case op::LD_inBC_A: return ret + "ld (bc), a";
    case op::INC_BC: return ret + "inc bc";
    case op::INC_B: return ret + "inc b";
    case op::DEC_B: return ret + "dec b";
    case op::LD_B_n8: return ret + "ld b, " + immValU8(bus, pc);
    case op::RLCA: return ret + "rlca";
    case op::LD_ina16_SP: return ret + "ld (" + symbolOrU16(gb, pc) + "), sp";
    case op::ADD_HL_BC: return ret + "add hl, bc";
    case op::LD_A_inBC: return ret + "ld a, (bc)";
    case op::DEC_BC: return ret + "dec bc";
    case op::INC_C: return ret + "inc c";
    case op::DEC_C: return ret + "dec c";
    case op::LD_C_n8: return ret + "ld c, " + immValU8(bus, pc);
    case op::RRCA: return ret + "rrca";

    // 0x1*
    case op::STOP: return ret + "stop";
    case op::LD_DE_n16: return ret + "ld de, " + symbolOrU16(gb, pc);
    case op::LD_inDE_A: return ret + "ld (de), a";
    case op::INC_DE: return ret + "inc de";
    case op::INC_D: return ret + "inc d";
    case op::DEC_D: return ret + "dec d";
    case op::LD_D_n8: return ret + "ld d, " + immValU8(bus, pc);
    case op::RLA: return ret + "rla";
    case op::JR_e8: return ret + "jr " + symbolOrS8(gb, pc);
    case op::ADD_HL_DE: return ret + "add hl, de";
    case op::LD_A_inDE: return ret + "ld a, (de)";
    case op::DEC_DE: return ret + "dec de";
    case op::INC_E: return ret + "inc e";
    case op::DEC_E: return ret + "dec e";
    case op::LD_E_n8: return ret + "ld e, " + immValU8(bus, pc);
    case op::RRA: return ret + "rra";

    // 0x2*
    case op::JR_NZ_e8: return ret + "jr nz " + symbolOrS8(gb, pc);
    case op::LD_HL_n16: return ret + "ld hl, " + symbolOrU16(gb, pc);
    case op::LD_inHLp_A: return ret + "ld (hl+), a";
    case op::INC_HL: return ret + "inc hl";
    case op::INC_H: return ret + "inc h";
    case op::DEC_H: return ret + "dec h";
    case op::LD_H_n8: return ret + "ld h, " + immValU8(bus, pc);
    case op::DAA: return ret + "daa";
    case op::JR_Z_e8: return ret + "jr z " + symbolOrS8(gb, pc);
    case op::ADD_HL_HL: return ret + "add hl, hl";
    case op::LD_A_inHLp: return ret + "ld a, (hl+)";
    case op::DEC_HL: return ret + "dec hl";
    case op::INC_L: return ret + "inc l";
    case op::DEC_L: return ret + "dec l";
    case op::LD_L_n8: return ret + "ld l, " + immValU8(bus, pc);
    case op::CPL: return ret + "cpl";

    // 0x3*
    case op::JR_NC_e8: return ret + "jr nc " + symbolOrS8(gb, pc);
    case op::LD_SP_n16: return ret + "ld sp, " + symbolOrU16(gb, pc);
    case op::LD_inHLm_A: return ret + "ld (hl-), a";
    case op::INC_SP: return ret + "inc sp";
    case op::INC_inHL: return ret + "inc (hl)";
    case op::DEC_inHL: return ret + "dec (hl)";
    case op::LD_inHL_n8: return ret + "ld (hl), " + immValU8(bus, pc);
    case op::SCF: return ret + "scf";
    case op::JR_C_e8: return ret + "jr c " + symbolOrS8(gb, pc);
    case op::ADD_HL_SP: return ret + "add hl, sp";
    case op::LD_A_inHLm: return ret + "ld a, (hl-)";
    case op::DEC_SP: return ret + "dec sp";
    case op::INC_A: return ret + "inc a";
    case op::DEC_A: return ret + "dec a";
    case op::LD_A_n8: return ret + "ld a, " + immValU8(bus, pc);
    case op::CCF: return ret + "ccf";

    // 0x4*
    case op::LD_B_B: return ret + "ld b, b";
    case op::LD_B_C: return ret + "ld b, c";
    case op::LD_B_D: return ret + "ld b, d";
    case op::LD_B_E: return ret + "ld b, e";
    case op::LD_B_H: return ret + "ld b, h";
    case op::LD_B_L: return ret + "ld b, l";
    case op::LD_B_inHL: return ret + "ld b, (hl)";
    case op::LD_B_A: return ret + "ld b, a";
    case op::LD_C_B: return ret + "ld c, b";
    case op::LD_C_C: return ret + "ld c, c";
    case op::LD_C_D: return ret + "ld c, d";
    case op::LD_C_E: return ret + "ld c, e";
    case op::LD_C_H: return ret + "ld c, h";
    case op::LD_C_L: return ret + "ld c, l";
    case op::LD_C_inHL: return ret + "ld c, (hl)";
    case op::LD_C_A: return ret + "ld c, a";

    // 0x5*
    case op::LD_D_B: return ret + "ld d, b";
    case op::LD_D_C: return ret + "ld d, c";
    case op::LD_D_D: return ret + "ld d, d";
    case op::LD_D_E: return ret + "ld d, e";
    case op::LD_D_H: return ret + "ld d, h";
    case op::LD_D_L: return ret + "ld d, l";
    case op::LD_D_inHL: return ret + "ld d, (hl)";
    case op::LD_D_A: return ret + "ld d, a";
    case op::LD_E_B: return ret + "ld e, b";
    case op::LD_E_C: return ret + "ld e, b";
    case op::LD_E_D: return ret + "ld e, b";
    case op::LD_E_E: return ret + "ld e, b";
    case op::LD_E_H: return ret + "ld e, b";
    case op::LD_E_L: return ret + "ld e, b";
    case op::LD_E_inHL: return ret + "ld e, (hl)";
    case op::LD_E_A: return ret + "ld e, a";

    // 0x6*
    case op::LD_H_B: return ret + "ld h, b";
    case op::LD_H_C: return ret + "ld h, c";
    case op::LD_H_D: return ret + "ld h, d";
    case op::LD_H_E: return ret + "ld h, e";
    case op::LD_H_H: return ret + "ld h, h";
    case op::LD_H_L: return ret + "ld h, l";
    case op::LD_H_inHL: return ret + "ld h, (hl)";
    case op::LD_H_A: return ret + "ld h, a";
    case op::LD_L_B: return ret + "ld l, b";
    case op::LD_L_C: return ret + "ld l, c";
    case op::LD_L_D: return ret + "ld l, d";
    case op::LD_L_E: return ret + "ld l, e";
    case op::LD_L_H: return ret + "ld l, h";
    case op::LD_L_L: return ret + "ld l, l";
    case op::LD_L_inHL: return ret + "ld l, (hl)";
    case op::LD_L_A: return ret + "ld l, a";

    // 0x7*
    case op::LD_inHl_B: return ret + "ld (hl), b";
    case op::LD_inHl_C: return ret + "ld (hl), c";
    case op::LD_inHl_D: return ret + "ld (hl), d";
    case op::LD_inHl_E: return ret + "ld (hl), e";
    case op::LD_inHl_H: return ret + "ld (hl), h";
    case op::LD_inHl_L: return ret + "ld (hl), l";
    case op::HALT: return ret + "halt";
    case op::LD_inHl_A: return ret + "ld (hl), a";
    case op::LD_A_B: return ret + "ld a, b";
    case op::LD_A_C: return ret + "ld a, c";
    case op::LD_A_D: return ret + "ld a, d";
    case op::LD_A_E: return ret + "ld a, e";
    case op::LD_A_H: return ret + "ld a, h";
    case op::LD_A_L: return ret + "ld a, l";
    case op::LD_A_inHL: return ret + "ld a, (hl)";
    case op::LD_A_A: return ret + "ld a, a";

    // 0x8*
    case op::ADD_A_B: return ret + "add a, b";
    case op::ADD_A_C: return ret + "add a, c";
    case op::ADD_A_D: return ret + "add a, d";
    case op::ADD_A_E: return ret + "add a, e";
    case op::ADD_A_H: return ret + "add a, h";
    case op::ADD_A_L: return ret + "add a, l";
    case op::ADD_A_inHL: return ret + "add a, (hl)";
    case op::ADD_A_A: return ret + "add a, a";
    case op::ADC_A_B: return ret + "adc a, b";
    case op::ADC_A_C: return ret + "adc a, c";
    case op::ADC_A_D: return ret + "adc a, d";
    case op::ADC_A_E: return ret + "adc a, e";
    case op::ADC_A_H: return ret + "adc a, h";
    case op::ADC_A_L: return ret + "adc a, l";
    case op::ADC_A_inHL: return ret + "adc a, (hl)";
    case op::ADC_A_A: return ret + "adc a, a";

    // 0x9*
    case op::SUB_A_B: return ret + "sub a, b";
    case op::SUB_A_C: return ret + "sub a, c";
    case op::SUB_A_D: return ret + "sub a, d";
    case op::SUB_A_E: return ret + "sub a, e";
    case op::SUB_A_H: return ret + "sub a, h";
    case op::SUB_A_L: return ret + "sub a, l";
    case op::SUB_A_inHL: return ret + "sub a, (hl)";
    case op::SUB_A_A: return ret + "sub a, a";
    case op::SBC_A_B: return ret + "sbc a, b";
    case op::SBC_A_C: return ret + "sbc a, c";
    case op::SBC_A_D: return ret + "sbc a, d";
    case op::SBC_A_E: return ret + "sbc a, e";
    case op::SBC_A_H: return ret + "sbc a, h";
    case op::SBC_A_L: return ret + "sbc a, l";
    case op::SBC_A_inHL: return ret + "sbc a, (hl)";
    case op::SBC_A_A: return ret + "sbc a, a";

    // 0xA*
    case op::AND_A_B: return ret + "and a, b";
    case op::AND_A_C: return ret + "and a, c";
    case op::AND_A_D: return ret + "and a, d";
    case op::AND_A_E: return ret + "and a, e";
    case op::AND_A_H: return ret + "and a, h";
    case op::AND_A_L: return ret + "and a, l";
    case op::AND_A_inHL: return ret + "and a, (hl)";
    case op::AND_A_A: return ret + "and a, a";
    case op::XOR_A_B: return ret + "xor a, b";
    case op::XOR_A_C: return ret + "xor a, c";
    case op::XOR_A_D: return ret + "xor a, d";
    case op::XOR_A_E: return ret + "xor a, e";
    case op::XOR_A_H: return ret + "xor a, h";
    case op::XOR_A_L: return ret + "xor a, l";
    case op::XOR_A_inHL: return ret + "xor a, (hl)";
    case op::XOR_A_A: return ret + "xor a, a";

    // 0xB*
    case op::OR_A_B: return ret + "or a, b";
    case op::OR_A_C: return ret + "or a, c";
    case op::OR_A_D: return ret + "or a, d";
    case op::OR_A_E: return ret + "or a, e";
    case op::OR_A_H: return ret + "or a, h";
    case op::OR_A_L: return ret + "or a, l";
    case op::OR_A_inHL: return ret + "or a, (hl)";
    case op::OR_A_A: return ret + "or a, a";
    case op::CP_A_B: return ret + "cp a, b";
    case op::CP_A_C: return ret + "cp a, c";
    case op::CP_A_D: return ret + "cp a, d";
    case op::CP_A_E: return ret + "cp a, e";
    case op::CP_A_H: return ret + "cp a, h";
    case op::CP_A_L: return ret + "cp a, l";
    case op::CP_A_inHL: return ret + "cp a, (hl)";
    case op::CP_A_A: return ret + "cp a, a";

    // 0xC*
    case op::RET_NZ: return ret + "ret nz";
    case op::POP_BC: return ret + "pop bc";
    case op::JP_NZ_a16: return ret + "jp nz " + symbolOrU16(gb, pc);
    case op::JP_a16: return ret + "jp " + symbolOrU16(gb, pc);
    case op::CALL_NZ_a16: return ret + "call nz " + symbolOrU16(gb, pc);
    case op::PUSH_BC: return ret + "push bc";
    case op::ADD_A_n8: return ret + "add a, " + immValU8(bus, pc);
    case op::RST_00: return ret + "rst $00";
    case op::RET_Z: return ret + "ret z";
    case op::RET: return ret + "ret";
    case op::JP_Z_a16: return ret + "jp z " + symbolOrU16(gb, pc);
    case op::CB_PREFIX: return instructionCBToStr(gb);
    case op::CALL_Z_a16: return ret + "call z " + symbolOrU16(gb, pc);
    case op::CALL_a16: return ret + "call " + symbolOrU16(gb, pc);
    case op::ADC_A_n8: return ret + "adc a, " + immValU8(bus, pc);
    case op::RST_08: return ret + "rst $08";

    // 0xD*
    case op::RET_NC: return ret + "ret nc";
    case op::POP_DE: return ret + "pop de";
    case op::JP_NC_a16: return ret + "jp nc " + symbolOrU16(gb, pc);
    // case op:: 0xD3 not implemented
    case op::CALL_NC_a16: return ret + "call nc " + symbolOrU16(gb, pc);
    case op::PUSH_DE: return ret + "push de";
    case op::SUB_A_n8: return ret + "sub a, " + immValU8(bus, pc);
    case op::RST_10: return ret + "rst $10";
    case op::RET_C: return ret + "ret c";
    case op::RETI: return ret + "reti";
    case op::JP_C_a16: return ret + "jp c " + symbolOrU16(gb, pc);
    // case op:: 0xDB not implemented
    case op::CALL_C_a16: return ret + "call c " + symbolOrU16(gb, pc);
    // case op:: 0xDD not implemented
    case op::SBC_A_n8: return ret + "sbc a, " + immValU8(bus, pc);
    case op::RST_18: return ret + "rst $18";

    // 0xE*
    case op::LDH_ina8_A: return ret + "ldh ($FF00 + " + immValU8(bus, pc) + "), a";
    case op::POP_HL: return ret + "pop hl";
    case op::LDH_inC_A: return ret + "ld ($FF00 + c), a";
    // case op:: 0xE3 not implemented
    // case op:: 0xE4 not implemented
    case op::PUSH_HL: return ret + "push hl";
    case op::AND_A_n8: return ret + "and a, " + immValU8(bus, pc);
    case op::RST_20: return ret + "rst $20";
    case op::ADD_SP_e8: return ret + "add sp, " + immValS8(bus, pc);
    case op::JP_HL: return ret + "jp hl";
    case op::LD_ina16_A: return ret + "ld (" + symbolOrU16(gb, pc) + "), a";
    // case op:: 0xEB not implemented
    // case op:: 0xEC not implemented
    // case op:: 0xED not implemented
    case op::XOR_A_n8: return ret + "xor a, " + immValU8(bus, pc);
    case op::RST_28: return ret + "rst $28";

    // 0xF*
    case op::LDH_A_ina8: return ret + "ldh a, ($FF00 + " + immValU8(bus, pc) + ')';
    case op::POP_AF: return ret + "pop af";
    case op::LDH_A_inC: return ret + "ld a, ($FF00 + c)";
    case op::DI: return ret + "di";
    // case op:: 0xF4 not implemented
    case op::PUSH_AF: return ret + "push af";
    case op::OR_A_n8: return ret + "or a, " + immValU8(bus, pc);
    case op::RST_30: return ret + "rst $30";
    case op::LD_HL_SPpe8: return ret + "ld hl, sp+(" + immValS8(bus, pc) + ')';
    case op::LD_SP_HL: return "ld sp, hl";
    case op::LD_A_ina16: return ret + "ld a, (" + symbolOrU16(gb, pc) + ')';
    case op::EI: return ret + "ei";
    // case op:: 0xFC not implemented
    // case op:: 0xFD not implemented
    case op::CP_A_n8: return ret + "cp a, " + immValU8(bus, pc);
    case op::RST_38: return "rst $38";

    default:
        // unrecognized opcode
        return ret + "???";
    }
}


std::string GBDebug::instructionCBToStr(const GameBoyClassic& gb)
{
    // to correctly execute one of the instructions prefixed with CB we 
    // have to read another byte from PC to get the actual opcode
    uint16_t pc = gb.cpu.regs.PC + 1;
    const auto& bus = gb.bus;

    uint8_t cbOpcode = bus.read8(pc++);

    std::string ret("(");
    ret += uintToHex((uint16_t)(0xCB << 8 | cbOpcode)) + ")   ";

    switch (cbOpcode) {
    // 0x0*
    case op_cb::RLC_B: return ret + "rlc b";
    case op_cb::RLC_C: return ret + "rlc c";
    case op_cb::RLC_D: return ret + "rlc d";
    case op_cb::RLC_E: return ret + "rlc e";
    case op_cb::RLC_H: return ret + "rlc h";
    case op_cb::RLC_L: return ret + "rlc l";
    case op_cb::RLC_inHL: return ret + "rlc (hl)";
    case op_cb::RLC_A: return ret + "rlc a";
    case op_cb::RRC_B: return ret + "rrc b";
    case op_cb::RRC_C: return ret + "rrc c";
    case op_cb::RRC_D: return ret + "rrc d";
    case op_cb::RRC_E: return ret + "rrc e";
    case op_cb::RRC_H: return ret + "rrc h";
    case op_cb::RRC_L: return ret + "rrc l";
    case op_cb::RRC_inHL: return ret + "rrc (hl)";
    case op_cb::RRC_A: return ret + "rrc a";

    // 0x1*
    case op_cb::RL_B: return ret + "rl b";
    case op_cb::RL_C: return ret + "rl c";
    case op_cb::RL_D: return ret + "rl d";
    case op_cb::RL_E: return ret + "rl e";
    case op_cb::RL_H: return ret + "rl h";
    case op_cb::RL_L: return ret + "rl l";
    case op_cb::RL_inHL: return ret + "rl (hl)";
    case op_cb::RL_A: return ret + "rl a";
    case op_cb::RR_B: return ret + "rr b";
    case op_cb::RR_C: return ret + "rr c";
    case op_cb::RR_D: return ret + "rr d";
    case op_cb::RR_E: return ret + "rr e";
    case op_cb::RR_H: return ret + "rr h";
    case op_cb::RR_L: return ret + "rr l";
    case op_cb::RR_inHL: return ret + "rr (hl)";
    case op_cb::RR_A: return ret + "rr a";

    // 0x2*
    case op_cb::SLA_B: return ret + "sla b";
    case op_cb::SLA_C: return ret + "sla c";
    case op_cb::SLA_D: return ret + "sla d";
    case op_cb::SLA_E: return ret + "sla e";
    case op_cb::SLA_H: return ret + "sla h";
    case op_cb::SLA_L: return ret + "sla l";
    case op_cb::SLA_inHL: return ret + "sla (hl)";
    case op_cb::SLA_A: return ret + "sla a";
    case op_cb::SRA_B: return ret + "sra b";
    case op_cb::SRA_C: return ret + "sra c";
    case op_cb::SRA_D: return ret + "sra d";
    case op_cb::SRA_E: return ret + "sra e";
    case op_cb::SRA_H: return ret + "sra h";
    case op_cb::SRA_L: return ret + "sra l";
    case op_cb::SRA_inHL: return ret + "sra (hl)";
    case op_cb::SRA_A: return ret + "sra a";

    // 0x3*
    case op_cb::SWAP_B: return ret + "swap b";
    case op_cb::SWAP_C: return ret + "swap c";
    case op_cb::SWAP_D: return ret + "swap d";
    case op_cb::SWAP_E: return ret + "swap e";
    case op_cb::SWAP_H: return ret + "swap h";
    case op_cb::SWAP_L: return ret + "swap l";
    case op_cb::SWAP_inHL: return ret + "swap (hl)";
    case op_cb::SWAP_A: return ret + "swap a";
    case op_cb::SRL_B: return ret + "srl b";
    case op_cb::SRL_C: return ret + "srl c";
    case op_cb::SRL_D: return ret + "srl d";
    case op_cb::SRL_E: return ret + "srl e";
    case op_cb::SRL_H: return ret + "srl h";
    case op_cb::SRL_L: return ret + "srl l";
    case op_cb::SRL_inHL: return ret + "srl (hl)";
    case op_cb::SRL_A: return ret + "srl a";

    // 0x4*
    case op_cb::BIT_0_B: return ret + "bit 0, b";
    case op_cb::BIT_0_C: return ret + "bit 0, c";
    case op_cb::BIT_0_D: return ret + "bit 0, d";
    case op_cb::BIT_0_E: return ret + "bit 0, e";
    case op_cb::BIT_0_H: return ret + "bit 0, h";
    case op_cb::BIT_0_L: return ret + "bit 0, l";
    case op_cb::BIT_0_inHL: return ret + "bit 0, (hl)";
    case op_cb::BIT_0_A: return ret + "bit 0, a";
    case op_cb::BIT_1_B: return ret + "bit 1, b";
    case op_cb::BIT_1_C: return ret + "bit 1, c";
    case op_cb::BIT_1_D: return ret + "bit 1, d";
    case op_cb::BIT_1_E: return ret + "bit 1, e";
    case op_cb::BIT_1_H: return ret + "bit 1, h";
    case op_cb::BIT_1_L: return ret + "bit 1, l";
    case op_cb::BIT_1_inHL: return ret + "bit 1, (hl)";
    case op_cb::BIT_1_A: return ret + "bit 1, a";

    // 0x5*
    case op_cb::BIT_2_B: return ret + "bit 2, b";
    case op_cb::BIT_2_C: return ret + "bit 2, c";
    case op_cb::BIT_2_D: return ret + "bit 2, d";
    case op_cb::BIT_2_E: return ret + "bit 2, e";
    case op_cb::BIT_2_H: return ret + "bit 2, h";
    case op_cb::BIT_2_L: return ret + "bit 2, l";
    case op_cb::BIT_2_inHL: return ret + "bit 2, (hl)";
    case op_cb::BIT_2_A: return ret + "bit 2, l";
    case op_cb::BIT_3_B: return ret + "bit 3, b";
    case op_cb::BIT_3_C: return ret + "bit 3, c";
    case op_cb::BIT_3_D: return ret + "bit 3, d";
    case op_cb::BIT_3_E: return ret + "bit 3, e";
    case op_cb::BIT_3_H: return ret + "bit 3, h";
    case op_cb::BIT_3_L: return ret + "bit 3, l";
    case op_cb::BIT_3_inHL: return ret + "bit 3, (hl)";
    case op_cb::BIT_3_A: return ret + "bit 3, a";

    // 0x6*
    case op_cb::BIT_4_B: return ret + "bit 4, b";
    case op_cb::BIT_4_C: return ret + "bit 4, c";
    case op_cb::BIT_4_D: return ret + "bit 4, d";
    case op_cb::BIT_4_E: return ret + "bit 4, e";
    case op_cb::BIT_4_H: return ret + "bit 4, h";
    case op_cb::BIT_4_L: return ret + "bit 4, l";
    case op_cb::BIT_4_inHL: return ret + "bit 4, (hl)";
    case op_cb::BIT_4_A: return ret + "bit 4, a";
    case op_cb::BIT_5_B: return ret + "bit 5, b";
    case op_cb::BIT_5_C: return ret + "bit 5, c";
    case op_cb::BIT_5_D: return ret + "bit 5, d";
    case op_cb::BIT_5_E: return ret + "bit 5, e";
    case op_cb::BIT_5_H: return ret + "bit 5, h";
    case op_cb::BIT_5_L: return ret + "bit 5, l";
    case op_cb::BIT_5_inHL: return ret + "bit 5, (hl)";
    case op_cb::BIT_5_A: return ret + "bit 5, a";

    // 0x7*
    case op_cb::BIT_6_B: return ret + "bit 6, b";
    case op_cb::BIT_6_C: return ret + "bit 6, c";
    case op_cb::BIT_6_D: return ret + "bit 6, d";
    case op_cb::BIT_6_E: return ret + "bit 6, e";
    case op_cb::BIT_6_H: return ret + "bit 6, h";
    case op_cb::BIT_6_L: return ret + "bit 6, l";
    case op_cb::BIT_6_inHL: return ret + "bit 6, (hl)";
    case op_cb::BIT_6_A: return ret + "bit 6, a";
    case op_cb::BIT_7_B: return ret + "bit 7, b";
    case op_cb::BIT_7_C: return ret + "bit 7, c";
    case op_cb::BIT_7_D: return ret + "bit 7, d";
    case op_cb::BIT_7_E: return ret + "bit 7, e";
    case op_cb::BIT_7_H: return ret + "bit 7, h";
    case op_cb::BIT_7_L: return ret + "bit 7, l";
    case op_cb::BIT_7_inHL: return ret + "bit 7, (hl)";
    case op_cb::BIT_7_A: return ret + "bit 7, a";

    // 0x8*
    case op_cb::RES_0_B: return ret + "res 0, b";
    case op_cb::RES_0_C: return ret + "res 0, c";
    case op_cb::RES_0_D: return ret + "res 0, d";
    case op_cb::RES_0_E: return ret + "res 0, e";
    case op_cb::RES_0_H: return ret + "res 0, h";
    case op_cb::RES_0_L: return ret + "res 0, l";
    case op_cb::RES_0_inHL: return ret + "res 0, (hl)";
    case op_cb::RES_0_A: return ret + "res 0, a";
    case op_cb::RES_1_B: return ret + "res 1, b";
    case op_cb::RES_1_C: return ret + "res 1, c";
    case op_cb::RES_1_D: return ret + "res 1, d";
    case op_cb::RES_1_E: return ret + "res 1, e";
    case op_cb::RES_1_H: return ret + "res 1, h";
    case op_cb::RES_1_L: return ret + "res 1, l";
    case op_cb::RES_1_inHL: return ret + "res 1, (hl)";
    case op_cb::RES_1_A: return ret + "res 1, a";

    // 0x9*
    case op_cb::RES_2_B: return ret + "res 2, b";
    case op_cb::RES_2_C: return ret + "res 2, c";
    case op_cb::RES_2_D: return ret + "res 2, d";
    case op_cb::RES_2_E: return ret + "res 2, e";
    case op_cb::RES_2_H: return ret + "res 2, h";
    case op_cb::RES_2_L: return ret + "res 2, l";
    case op_cb::RES_2_inHL: return ret + "res 2, (hl)";
    case op_cb::RES_2_A: return ret + "res 2, a";
    case op_cb::RES_3_B: return ret + "res 3, b";
    case op_cb::RES_3_C: return ret + "res 3, c";
    case op_cb::RES_3_D: return ret + "res 3, d";
    case op_cb::RES_3_E: return ret + "res 3, e";
    case op_cb::RES_3_H: return ret + "res 3, h";
    case op_cb::RES_3_L: return ret + "res 3, l";
    case op_cb::RES_3_inHL: return ret + "res 3, (hl)";
    case op_cb::RES_3_A: return ret + "res 3, a";

    // 0xA*
    case op_cb::RES_4_B: return ret + "res 4, b";
    case op_cb::RES_4_C: return ret + "res 4, c";
    case op_cb::RES_4_D: return ret + "res 4, d";
    case op_cb::RES_4_E: return ret + "res 4, e";
    case op_cb::RES_4_H: return ret + "res 4, h";
    case op_cb::RES_4_L: return ret + "res 4, l";
    case op_cb::RES_4_inHL: return ret + "res 4, (hl)";
    case op_cb::RES_4_A: return ret + "res 4, a";
    case op_cb::RES_5_B: return ret + "res 5, b";
    case op_cb::RES_5_C: return ret + "res 5, c";
    case op_cb::RES_5_D: return ret + "res 5, d";
    case op_cb::RES_5_E: return ret + "res 5, e";
    case op_cb::RES_5_H: return ret + "res 5, h";
    case op_cb::RES_5_L: return ret + "res 5, l";
    case op_cb::RES_5_inHL: return ret + "res 5, (hl)";
    case op_cb::RES_5_A: return ret + "res 5, a";

    // 0xB*
    case op_cb::RES_6_B: return ret + "res 6, b";
    case op_cb::RES_6_C: return ret + "res 6, c";
    case op_cb::RES_6_D: return ret + "res 6, d";
    case op_cb::RES_6_E: return ret + "res 6, e";
    case op_cb::RES_6_H: return ret + "res 6, h";
    case op_cb::RES_6_L: return ret + "res 6, l";
    case op_cb::RES_6_inHL: return ret + "res 6, (hl)";
    case op_cb::RES_6_A: return ret + "res 6, a";
    case op_cb::RES_7_B: return ret + "res 7, b";
    case op_cb::RES_7_C: return ret + "res 7, c";
    case op_cb::RES_7_D: return ret + "res 7, d";
    case op_cb::RES_7_E: return ret + "res 7, e";
    case op_cb::RES_7_H: return ret + "res 7, h";
    case op_cb::RES_7_L: return ret + "res 7, l";
    case op_cb::RES_7_inHL: return ret + "res 7, (hl)";
    case op_cb::RES_7_A: return ret + "res 7, a";

        // 0xC*
    case op_cb::SET_0_B: return ret + "set 0, b";
    case op_cb::SET_0_C: return ret + "set 0, c";
    case op_cb::SET_0_D: return ret + "set 0, d";
    case op_cb::SET_0_E: return ret + "set 0, e";
    case op_cb::SET_0_H: return ret + "set 0, h";
    case op_cb::SET_0_L: return ret + "set 0, l";
    case op_cb::SET_0_inHL: return ret + "set 0, (hl)";
    case op_cb::SET_0_A: return ret + "set 0, a";
    case op_cb::SET_1_B: return ret + "set 1, b";
    case op_cb::SET_1_C: return ret + "set 1, c";
    case op_cb::SET_1_D: return ret + "set 1, d";
    case op_cb::SET_1_E: return ret + "set 1, e";
    case op_cb::SET_1_H: return ret + "set 1, h";
    case op_cb::SET_1_L: return ret + "set 1, l";
    case op_cb::SET_1_inHL: return ret + "set 1, (hl)";
    case op_cb::SET_1_A: return ret + "set 1, a";

        // 0xD*
    case op_cb::SET_2_B: return ret + "set 2, b";
    case op_cb::SET_2_C: return ret + "set 2, c";
    case op_cb::SET_2_D: return ret + "set 2, d";
    case op_cb::SET_2_E: return ret + "set 2, e";
    case op_cb::SET_2_H: return ret + "set 2, h";
    case op_cb::SET_2_L: return ret + "set 2, l";
    case op_cb::SET_2_inHL: return ret + "set 2, (hl)";
    case op_cb::SET_2_A: return ret + "set 2, a";
    case op_cb::SET_3_B: return ret + "set 3, b";
    case op_cb::SET_3_C: return ret + "set 3, c";
    case op_cb::SET_3_D: return ret + "set 3, d";
    case op_cb::SET_3_E: return ret + "set 3, e";
    case op_cb::SET_3_H: return ret + "set 3, h";
    case op_cb::SET_3_L: return ret + "set 3, l";
    case op_cb::SET_3_inHL: return ret + "set 3, (hl)";
    case op_cb::SET_3_A: return ret + "set 3, a";

        // 0xE*
    case op_cb::SET_4_B: return ret + "set 4, b";
    case op_cb::SET_4_C: return ret + "set 4, c";
    case op_cb::SET_4_D: return ret + "set 4, d";
    case op_cb::SET_4_E: return ret + "set 4, e";
    case op_cb::SET_4_H: return ret + "set 4, h";
    case op_cb::SET_4_L: return ret + "set 4, l";
    case op_cb::SET_4_inHL: return ret + "set 4, (hl)";
    case op_cb::SET_4_A: return ret + "set 4, a";
    case op_cb::SET_5_B: return ret + "set 5, b";
    case op_cb::SET_5_C: return ret + "set 5, c";
    case op_cb::SET_5_D: return ret + "set 5, d";
    case op_cb::SET_5_E: return ret + "set 5, e";
    case op_cb::SET_5_H: return ret + "set 5, h";
    case op_cb::SET_5_L: return ret + "set 5, l";
    case op_cb::SET_5_inHL: return ret + "set 5, (hl)";
    case op_cb::SET_5_A: return ret + "set 5, a";

        // 0xF*
    case op_cb::SET_6_B: return ret + "set 6, b";
    case op_cb::SET_6_C: return ret + "set 6, c";
    case op_cb::SET_6_D: return ret + "set 6, d";
    case op_cb::SET_6_E: return ret + "set 6, e";
    case op_cb::SET_6_H: return ret + "set 6, h";
    case op_cb::SET_6_L: return ret + "set 6, l";
    case op_cb::SET_6_inHL: return ret + "set 6, (hl)";
    case op_cb::SET_6_A: return ret + "set 6, a";
    case op_cb::SET_7_B: return ret + "set 7, b";
    case op_cb::SET_7_C: return ret + "set 7, c";
    case op_cb::SET_7_D: return ret + "set 7, d";
    case op_cb::SET_7_E: return ret + "set 7, e";
    case op_cb::SET_7_H: return ret + "set 7, h";
    case op_cb::SET_7_L: return ret + "set 7, l";
    case op_cb::SET_7_inHL: return ret + "set 7, (hl)";
    case op_cb::SET_7_A: return ret + "set 7, a";
    default:
        // unrecognized opcode
        // shouldn't happen with CB prefixed instructions
        return ret + "???";
    }
}



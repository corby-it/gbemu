

#include <cstdint>


namespace op {

    // from: https://gbdev.io/gb-opcodes/optables/

    // - n8 means immediate 8-bit data
    // - n16 means immediate little-endian 16-bit data
    // - a8 means 8-bit unsigned data, which is added to $FF00 in certain instructions to create a 16-bit address in HRAM (High RAM)
    // - a16 means little-endian 16-bit address
    // - e8 means 8-bit signed data
    // - "in" means indirect access, for example "inBC" means access the memory at address stored in BC
    // - HLp means increment the value of HL after use, equivalent to A = mem[HL++]
    // - HLm means decrement the value of HL after use, equivalent to A = mem[HL--]

    // 0x0*
    static constexpr const uint8_t NOP          = 0x00;
    static constexpr const uint8_t LD_BC_n16    = 0x01;
    static constexpr const uint8_t LD_inBC_A    = 0x02;
    static constexpr const uint8_t INC_BC       = 0x03;
    static constexpr const uint8_t INC_B        = 0x04;
    static constexpr const uint8_t DEC_B        = 0x05;
    static constexpr const uint8_t LD_B_n8      = 0x06;
    static constexpr const uint8_t RLCA         = 0x07;
    static constexpr const uint8_t LD_ina16_SP  = 0x08;
    static constexpr const uint8_t ADD_HL_BC    = 0x09;
    static constexpr const uint8_t LD_A_inBC    = 0x0A;
    static constexpr const uint8_t DEC_BC       = 0x0B;
    static constexpr const uint8_t INC_C        = 0x0C;
    static constexpr const uint8_t DEC_C        = 0x0D;
    static constexpr const uint8_t LD_C_n8      = 0x0E;
    static constexpr const uint8_t RRCA         = 0x0F;

    // 0x1*
    static constexpr const uint8_t STOP_n8      = 0x10;
    static constexpr const uint8_t LD_DE_n16    = 0x11;
    static constexpr const uint8_t LD_inDE_A    = 0x12;
    static constexpr const uint8_t INC_DE       = 0x13;
    static constexpr const uint8_t INC_D        = 0x14;
    static constexpr const uint8_t DEC_D        = 0x15;
    static constexpr const uint8_t LD_D_n8      = 0x16;
    static constexpr const uint8_t RLA          = 0x17;
    static constexpr const uint8_t JR_e8        = 0x18;
    static constexpr const uint8_t ADD_HL_DE    = 0x19;
    static constexpr const uint8_t LD_A_inDE    = 0x1A;
    static constexpr const uint8_t DEC_DE       = 0x1B;
    static constexpr const uint8_t INC_E        = 0x1C;
    static constexpr const uint8_t DEC_E        = 0x1D;
    static constexpr const uint8_t LD_E_n8      = 0x1E;
    static constexpr const uint8_t RRA          = 0x1F;

    // 0x2*
    static constexpr const uint8_t JR_NZ_e8     = 0x20;
    static constexpr const uint8_t LD_HL_n16    = 0x21;
    static constexpr const uint8_t LD_inHLp_A   = 0x22;
    static constexpr const uint8_t INC_HL       = 0x23;
    static constexpr const uint8_t INC_H        = 0x24;
    static constexpr const uint8_t DEC_H        = 0x25;
    static constexpr const uint8_t LD_H_n8      = 0x26;
    static constexpr const uint8_t DAA          = 0x27;
    static constexpr const uint8_t JR_Z_e8      = 0x28;
    static constexpr const uint8_t ADD_HL_HL    = 0x29;
    static constexpr const uint8_t LD_A_inHLp   = 0x2A;
    static constexpr const uint8_t DEC_HL       = 0x2B;
    static constexpr const uint8_t INC_L        = 0x2C;
    static constexpr const uint8_t DEC_L        = 0x2D;
    static constexpr const uint8_t LD_L_n8      = 0x2E;
    static constexpr const uint8_t CPL          = 0x2F;

    // 0x3*
    static constexpr const uint8_t JR_NC_e8     = 0x30;
    static constexpr const uint8_t LD_SP_n16    = 0x31;
    static constexpr const uint8_t LD_inHLm_A   = 0x32;
    static constexpr const uint8_t INC_SP       = 0x33;
    static constexpr const uint8_t INC_inHL     = 0x34;
    static constexpr const uint8_t DEC_inHL     = 0x35;
    static constexpr const uint8_t LD_inHL_n8   = 0x36;
    static constexpr const uint8_t SCF          = 0x37;
    static constexpr const uint8_t JR_C_e8      = 0x38;
    static constexpr const uint8_t ADD_HL_SP    = 0x39;
    static constexpr const uint8_t LD_A_inHLm   = 0x3A;
    static constexpr const uint8_t DEC_SP       = 0x3B;
    static constexpr const uint8_t INC_A        = 0x3C;
    static constexpr const uint8_t DEC_A        = 0x3D;
    static constexpr const uint8_t LD_A_n8      = 0x3E;
    static constexpr const uint8_t CCF          = 0x3F;

    // 0x4*
    static constexpr const uint8_t LD_B_B       = 0x40;
    static constexpr const uint8_t LD_B_C       = 0x41;
    static constexpr const uint8_t LD_B_D       = 0x42;
    static constexpr const uint8_t LD_B_E       = 0x43;
    static constexpr const uint8_t LD_B_H       = 0x44;
    static constexpr const uint8_t LD_B_L       = 0x45;
    static constexpr const uint8_t LD_B_inHL    = 0x46;
    static constexpr const uint8_t LD_B_A       = 0x47;
    static constexpr const uint8_t LD_C_B       = 0x48;
    static constexpr const uint8_t LD_C_C       = 0x49;
    static constexpr const uint8_t LD_C_D       = 0x4A;
    static constexpr const uint8_t LD_C_E       = 0x4B;
    static constexpr const uint8_t LD_C_H       = 0x4C;
    static constexpr const uint8_t LD_C_L       = 0x4D;
    static constexpr const uint8_t LD_C_inHL    = 0x4E;
    static constexpr const uint8_t LD_C_A       = 0x4F;

    // 0x5*
    static constexpr const uint8_t LD_D_B       = 0x50;
    static constexpr const uint8_t LD_D_C       = 0x51;
    static constexpr const uint8_t LD_D_D       = 0x52;
    static constexpr const uint8_t LD_D_E       = 0x53;
    static constexpr const uint8_t LD_D_H       = 0x54;
    static constexpr const uint8_t LD_D_L       = 0x55;
    static constexpr const uint8_t LD_D_inHL    = 0x56;
    static constexpr const uint8_t LD_D_A       = 0x57;
    static constexpr const uint8_t LD_E_B       = 0x58;
    static constexpr const uint8_t LD_E_C       = 0x59;
    static constexpr const uint8_t LD_E_D       = 0x5A;
    static constexpr const uint8_t LD_E_E       = 0x5B;
    static constexpr const uint8_t LD_E_H       = 0x5C;
    static constexpr const uint8_t LD_E_L       = 0x5D;
    static constexpr const uint8_t LD_E_inHL    = 0x5E;
    static constexpr const uint8_t LD_E_A       = 0x5F;

    // 0x6*
    static constexpr const uint8_t LD_H_B       = 0x60;
    static constexpr const uint8_t LD_H_C       = 0x61;
    static constexpr const uint8_t LD_H_D       = 0x62;
    static constexpr const uint8_t LD_H_E       = 0x63;
    static constexpr const uint8_t LD_H_H       = 0x64;
    static constexpr const uint8_t LD_H_L       = 0x65;
    static constexpr const uint8_t LD_H_inHL    = 0x66;
    static constexpr const uint8_t LD_H_A       = 0x67;
    static constexpr const uint8_t LD_L_B       = 0x68;
    static constexpr const uint8_t LD_L_C       = 0x69;
    static constexpr const uint8_t LD_L_D       = 0x6A;
    static constexpr const uint8_t LD_L_E       = 0x6B;
    static constexpr const uint8_t LD_L_H       = 0x6C;
    static constexpr const uint8_t LD_L_L       = 0x6D;
    static constexpr const uint8_t LD_L_inHL    = 0x6E;
    static constexpr const uint8_t LD_L_A       = 0x6F;

    // 0x7*
    static constexpr const uint8_t LD_inHl_B    = 0x70;
    static constexpr const uint8_t LD_inHl_C    = 0x71;
    static constexpr const uint8_t LD_inHl_D    = 0x72;
    static constexpr const uint8_t LD_inHl_E    = 0x73;
    static constexpr const uint8_t LD_inHl_H    = 0x74;
    static constexpr const uint8_t LD_inHl_L    = 0x75;
    static constexpr const uint8_t HALT         = 0x76;
    static constexpr const uint8_t LD_inHl_A    = 0x77;
    static constexpr const uint8_t LD_A_B       = 0x78;
    static constexpr const uint8_t LD_A_C       = 0x79;
    static constexpr const uint8_t LD_A_D       = 0x7A;
    static constexpr const uint8_t LD_A_E       = 0x7B;
    static constexpr const uint8_t LD_A_H       = 0x7C;
    static constexpr const uint8_t LD_A_L       = 0x7D;
    static constexpr const uint8_t LD_A_inHL    = 0x7E;
    static constexpr const uint8_t LD_A_A       = 0x7F;

    // 0x8*
    static constexpr const uint8_t ADD_A_B      = 0x80;
    static constexpr const uint8_t ADD_A_C      = 0x81;
    static constexpr const uint8_t ADD_A_D      = 0x82;
    static constexpr const uint8_t ADD_A_E      = 0x83;
    static constexpr const uint8_t ADD_A_H      = 0x84;
    static constexpr const uint8_t ADD_A_L      = 0x85;
    static constexpr const uint8_t ADD_A_inHL   = 0x86;
    static constexpr const uint8_t ADD_A_A      = 0x87;
    static constexpr const uint8_t ADC_A_B      = 0x88;
    static constexpr const uint8_t ADC_A_C      = 0x89;
    static constexpr const uint8_t ADC_A_D      = 0x8A;
    static constexpr const uint8_t ADC_A_E      = 0x8B;
    static constexpr const uint8_t ADC_A_H      = 0x8C;
    static constexpr const uint8_t ADC_A_L      = 0x8D;
    static constexpr const uint8_t ADC_A_inHL   = 0x8E;
    static constexpr const uint8_t ADC_A_A      = 0x8F;

    // 0x9*
    static constexpr const uint8_t SUB_A_B      = 0x90;
    static constexpr const uint8_t SUB_A_C      = 0x91;
    static constexpr const uint8_t SUB_A_D      = 0x92;
    static constexpr const uint8_t SUB_A_E      = 0x93;
    static constexpr const uint8_t SUB_A_H      = 0x94;
    static constexpr const uint8_t SUB_A_L      = 0x95;
    static constexpr const uint8_t SUB_A_inHL   = 0x96;
    static constexpr const uint8_t SUB_A_A      = 0x97;
    static constexpr const uint8_t SBC_A_B      = 0x98;
    static constexpr const uint8_t SBC_A_C      = 0x99;
    static constexpr const uint8_t SBC_A_D      = 0x9A;
    static constexpr const uint8_t SBC_A_E      = 0x9B;
    static constexpr const uint8_t SBC_A_H      = 0x9C;
    static constexpr const uint8_t SBC_A_L      = 0x9D;
    static constexpr const uint8_t SBC_A_inHL   = 0x9E;
    static constexpr const uint8_t SBC_A_A      = 0x9F;

    // 0xA*
    static constexpr const uint8_t AND_A_B      = 0xA0;
    static constexpr const uint8_t AND_A_C      = 0xA1;
    static constexpr const uint8_t AND_A_D      = 0xA2;
    static constexpr const uint8_t AND_A_E      = 0xA3;
    static constexpr const uint8_t AND_A_H      = 0xA4;
    static constexpr const uint8_t AND_A_L      = 0xA5;
    static constexpr const uint8_t AND_A_inHL   = 0xA6;
    static constexpr const uint8_t AND_A_A      = 0xA7;
    static constexpr const uint8_t XOR_A_B      = 0xA8;
    static constexpr const uint8_t XOR_A_C      = 0xA9;
    static constexpr const uint8_t XOR_A_D      = 0xAA;
    static constexpr const uint8_t XOR_A_E      = 0xAB;
    static constexpr const uint8_t XOR_A_H      = 0xAC;
    static constexpr const uint8_t XOR_A_L      = 0xAD;
    static constexpr const uint8_t XOR_A_inHL   = 0xAE;
    static constexpr const uint8_t XOR_A_A      = 0xAF;

    // 0xB*
    static constexpr const uint8_t OR_A_B       = 0xB0;
    static constexpr const uint8_t OR_A_C       = 0xB1;
    static constexpr const uint8_t OR_A_D       = 0xB2;
    static constexpr const uint8_t OR_A_E       = 0xB3;
    static constexpr const uint8_t OR_A_H       = 0xB4;
    static constexpr const uint8_t OR_A_L       = 0xB5;
    static constexpr const uint8_t OR_A_inHL    = 0xB6;
    static constexpr const uint8_t OR_A_A       = 0xB7;
    static constexpr const uint8_t CP_A_B       = 0xB8;
    static constexpr const uint8_t CP_A_C       = 0xB9;
    static constexpr const uint8_t CP_A_D       = 0xBA;
    static constexpr const uint8_t CP_A_E       = 0xBB;
    static constexpr const uint8_t CP_A_H       = 0xBC;
    static constexpr const uint8_t CP_A_L       = 0xBD;
    static constexpr const uint8_t CP_A_inHL    = 0xBE;
    static constexpr const uint8_t CP_A_A       = 0xBF;

    // 0xC*
    static constexpr const uint8_t RET_NZ       = 0xC0;
    static constexpr const uint8_t POP_BC       = 0xC1;
    static constexpr const uint8_t JP_NZ_a16    = 0xC2;
    static constexpr const uint8_t JP_a16       = 0xC3;
    static constexpr const uint8_t CALL_NZ_a16  = 0xC4;
    static constexpr const uint8_t PUSH_BC      = 0xC5;
    static constexpr const uint8_t ADD_A_n8     = 0xC6;
    static constexpr const uint8_t RST_00       = 0xC7;
    static constexpr const uint8_t RET_Z        = 0xC8;
    static constexpr const uint8_t RET          = 0xC9;
    static constexpr const uint8_t JP_Z_a16     = 0xCA;
    static constexpr const uint8_t CB_PREFIX    = 0xCB;
    static constexpr const uint8_t CALL_Z_a16   = 0xCC;
    static constexpr const uint8_t CALL_a16     = 0xCD;
    static constexpr const uint8_t ADC_A_n8     = 0xCE;
    static constexpr const uint8_t RST_08       = 0xCF;

    // 0xD*
    static constexpr const uint8_t RET_NC       = 0xD0;
    static constexpr const uint8_t POP_DE       = 0xD1;
    static constexpr const uint8_t JP_NC_a16    = 0xD2;
    //static constexpr const uint8_t            = 0xD3;
    static constexpr const uint8_t CALL_NC_a16  = 0xD4;
    static constexpr const uint8_t PUSH_DE      = 0xD5;
    static constexpr const uint8_t SUB_A_n8     = 0xD6;
    static constexpr const uint8_t RST_10       = 0xD7;
    static constexpr const uint8_t RET_C        = 0xD8;
    static constexpr const uint8_t RETI         = 0xD9;
    static constexpr const uint8_t JP_C_a16     = 0xDA;
    //static constexpr const uint8_t            = 0xDB;
    static constexpr const uint8_t CALL_C_a16   = 0xDC;
    //static constexpr const uint8_t            = 0xDD;
    static constexpr const uint8_t SBC_A_n8     = 0xDE;
    static constexpr const uint8_t RST_18       = 0xDF;

    // 0xE*
    static constexpr const uint8_t LDH_ina8_A   = 0xE0;
    static constexpr const uint8_t POP_HL       = 0xE1;
    static constexpr const uint8_t LDH_inC_A    = 0xE2;
    //static constexpr const uint8_t            = 0xE3;
    //static constexpr const uint8_t            = 0xE4;
    static constexpr const uint8_t PUSH_HL      = 0xE5;
    static constexpr const uint8_t AND_A_n8     = 0xE6;
    static constexpr const uint8_t RST_20       = 0xE7;
    static constexpr const uint8_t ADD_SP_e8    = 0xE8;
    static constexpr const uint8_t JP_HL        = 0xE9;
    static constexpr const uint8_t LD_ina16_A   = 0xEA;
    //static constexpr const uint8_t            = 0xEB;
    //static constexpr const uint8_t            = 0xEC;
    //static constexpr const uint8_t            = 0xED;
    static constexpr const uint8_t XOR_A_n8     = 0xEE;
    static constexpr const uint8_t RST_28       = 0xEF;

    // 0xF*
    static constexpr const uint8_t LDH_A_ina8   = 0xF0;
    static constexpr const uint8_t POP_AF       = 0xF1;
    static constexpr const uint8_t LDH_A_inC    = 0xF2;
    static constexpr const uint8_t DI           = 0xF3;
    //static constexpr const uint8_t            = 0xF4;
    static constexpr const uint8_t PUSH_AF      = 0xF5;
    static constexpr const uint8_t OR_A_n8      = 0xF6;
    static constexpr const uint8_t RST_30       = 0xF7;
    static constexpr const uint8_t LD_HL_SPpe8  = 0xF8;
    static constexpr const uint8_t LD_SP_HL     = 0xF9;
    static constexpr const uint8_t LD_A_ina16   = 0xFA;
    static constexpr const uint8_t EI           = 0xFB;
    //static constexpr const uint8_t            = 0xFC;
    //static constexpr const uint8_t            = 0xFD;
    static constexpr const uint8_t CP_A_n8      = 0xFE;
    static constexpr const uint8_t RST_38       = 0xFF;

}



namespace op_cb {

    // these are the opcodes starting with 0xCB, when the cpu fetches CB as the opcode it 
    // then parses another opcode to find the actual instruction
    // these are all bit operations on registers

    // 0x0*
    static constexpr const uint8_t RLC_B        = 0x00;
    static constexpr const uint8_t RLC_C        = 0x01;
    static constexpr const uint8_t RLC_D        = 0x02;
    static constexpr const uint8_t RLC_E        = 0x03;
    static constexpr const uint8_t RLC_H        = 0x04;
    static constexpr const uint8_t RLC_L        = 0x05;
    static constexpr const uint8_t RLC_inHL     = 0x06;
    static constexpr const uint8_t RLC_A        = 0x07;
    static constexpr const uint8_t RRC_B        = 0x08;
    static constexpr const uint8_t RRC_C        = 0x09;
    static constexpr const uint8_t RRC_D        = 0x0A;
    static constexpr const uint8_t RRC_E        = 0x0B;
    static constexpr const uint8_t RRC_H        = 0x0C;
    static constexpr const uint8_t RRC_L        = 0x0D;
    static constexpr const uint8_t RRC_inHL     = 0x0E;
    static constexpr const uint8_t RRC_A        = 0x0F;

    // 0x1*
    static constexpr const uint8_t RL_B         = 0x10;
    static constexpr const uint8_t RL_C         = 0x11;
    static constexpr const uint8_t RL_D         = 0x12;
    static constexpr const uint8_t RL_E         = 0x13;
    static constexpr const uint8_t RL_H         = 0x14;
    static constexpr const uint8_t RL_L         = 0x15;
    static constexpr const uint8_t RL_inHL      = 0x16;
    static constexpr const uint8_t RL_A         = 0x17;
    static constexpr const uint8_t RR_B         = 0x18;
    static constexpr const uint8_t RR_C         = 0x19;
    static constexpr const uint8_t RR_D         = 0x1A;
    static constexpr const uint8_t RR_E         = 0x1B;
    static constexpr const uint8_t RR_H         = 0x1C;
    static constexpr const uint8_t RR_L         = 0x1D;
    static constexpr const uint8_t RR_inHL      = 0x1E;
    static constexpr const uint8_t RR_A         = 0x1F;

    // 0x2*
    static constexpr const uint8_t SLA_B        = 0x20;
    static constexpr const uint8_t SLA_C        = 0x21;
    static constexpr const uint8_t SLA_D        = 0x22;
    static constexpr const uint8_t SLA_E        = 0x23;
    static constexpr const uint8_t SLA_H        = 0x24;
    static constexpr const uint8_t SLA_L        = 0x25;
    static constexpr const uint8_t SLA_inHL     = 0x26;
    static constexpr const uint8_t SLA_A        = 0x27;
    static constexpr const uint8_t SRA_B        = 0x28;
    static constexpr const uint8_t SRA_C        = 0x29;
    static constexpr const uint8_t SRA_D        = 0x2A;
    static constexpr const uint8_t SRA_E        = 0x2B;
    static constexpr const uint8_t SRA_H        = 0x2C;
    static constexpr const uint8_t SRA_L        = 0x2D;
    static constexpr const uint8_t SRA_inHL     = 0x2E;
    static constexpr const uint8_t SRA_A        = 0x2F;

    // 0x3*
    static constexpr const uint8_t SWAP_B       = 0x30;
    static constexpr const uint8_t SWAP_C       = 0x31;
    static constexpr const uint8_t SWAP_D       = 0x32;
    static constexpr const uint8_t SWAP_E       = 0x33;
    static constexpr const uint8_t SWAP_H       = 0x34;
    static constexpr const uint8_t SWAP_L       = 0x35;
    static constexpr const uint8_t SWAP_inHL    = 0x36;
    static constexpr const uint8_t SWAP_A       = 0x37;
    static constexpr const uint8_t SRL_B        = 0x38;
    static constexpr const uint8_t SRL_C        = 0x39;
    static constexpr const uint8_t SRL_D        = 0x3A;
    static constexpr const uint8_t SRL_E        = 0x3B;
    static constexpr const uint8_t SRL_H        = 0x3C;
    static constexpr const uint8_t SRL_L        = 0x3D;
    static constexpr const uint8_t SRL_inHL     = 0x3E;
    static constexpr const uint8_t SRL_A        = 0x3F;

    // 0x4*
    static constexpr const uint8_t BIT_0_B      = 0x40;
    static constexpr const uint8_t BIT_0_C      = 0x41;
    static constexpr const uint8_t BIT_0_D      = 0x42;
    static constexpr const uint8_t BIT_0_E      = 0x43;
    static constexpr const uint8_t BIT_0_H      = 0x44;
    static constexpr const uint8_t BIT_0_L      = 0x45;
    static constexpr const uint8_t BIT_0_inHL   = 0x46;
    static constexpr const uint8_t BIT_0_A      = 0x47;
    static constexpr const uint8_t BIT_1_B      = 0x48;
    static constexpr const uint8_t BIT_1_C      = 0x49;
    static constexpr const uint8_t BIT_1_D      = 0x4A;
    static constexpr const uint8_t BIT_1_E      = 0x4B;
    static constexpr const uint8_t BIT_1_H      = 0x4C;
    static constexpr const uint8_t BIT_1_L      = 0x4D;
    static constexpr const uint8_t BIT_1_inHL   = 0x4E;
    static constexpr const uint8_t BIT_1_A      = 0x4F;

    // 0x5*
    static constexpr const uint8_t BIT_2_B      = 0x50;
    static constexpr const uint8_t BIT_2_C      = 0x51;
    static constexpr const uint8_t BIT_2_D      = 0x52;
    static constexpr const uint8_t BIT_2_E      = 0x53;
    static constexpr const uint8_t BIT_2_H      = 0x54;
    static constexpr const uint8_t BIT_2_L      = 0x55;
    static constexpr const uint8_t BIT_2_inHL   = 0x56;
    static constexpr const uint8_t BIT_2_A      = 0x57;
    static constexpr const uint8_t BIT_3_B      = 0x58;
    static constexpr const uint8_t BIT_3_C      = 0x59;
    static constexpr const uint8_t BIT_3_D      = 0x5A;
    static constexpr const uint8_t BIT_3_E      = 0x5B;
    static constexpr const uint8_t BIT_3_H      = 0x5C;
    static constexpr const uint8_t BIT_3_L      = 0x5D;
    static constexpr const uint8_t BIT_3_inHL   = 0x5E;
    static constexpr const uint8_t BIT_3_A      = 0x5F;

    // 0x6*
    static constexpr const uint8_t BIT_4_B      = 0x60;
    static constexpr const uint8_t BIT_4_C      = 0x61;
    static constexpr const uint8_t BIT_4_D      = 0x62;
    static constexpr const uint8_t BIT_4_E      = 0x63;
    static constexpr const uint8_t BIT_4_H      = 0x64;
    static constexpr const uint8_t BIT_4_L      = 0x65;
    static constexpr const uint8_t BIT_4_inHL   = 0x66;
    static constexpr const uint8_t BIT_4_A      = 0x67;
    static constexpr const uint8_t BIT_5_B      = 0x68;
    static constexpr const uint8_t BIT_5_C      = 0x69;
    static constexpr const uint8_t BIT_5_D      = 0x6A;
    static constexpr const uint8_t BIT_5_E      = 0x6B;
    static constexpr const uint8_t BIT_5_H      = 0x6C;
    static constexpr const uint8_t BIT_5_L      = 0x6D;
    static constexpr const uint8_t BIT_5_inHL   = 0x6E;
    static constexpr const uint8_t BIT_5_A      = 0x6F;

    // 0x7*
    static constexpr const uint8_t BIT_6_B      = 0x70;
    static constexpr const uint8_t BIT_6_C      = 0x71;
    static constexpr const uint8_t BIT_6_D      = 0x72;
    static constexpr const uint8_t BIT_6_E      = 0x73;
    static constexpr const uint8_t BIT_6_H      = 0x74;
    static constexpr const uint8_t BIT_6_L      = 0x75;
    static constexpr const uint8_t BIT_6_inHL   = 0x76;
    static constexpr const uint8_t BIT_6_A      = 0x77;
    static constexpr const uint8_t BIT_7_B      = 0x78;
    static constexpr const uint8_t BIT_7_C      = 0x79;
    static constexpr const uint8_t BIT_7_D      = 0x7A;
    static constexpr const uint8_t BIT_7_E      = 0x7B;
    static constexpr const uint8_t BIT_7_H      = 0x7C;
    static constexpr const uint8_t BIT_7_L      = 0x7D;
    static constexpr const uint8_t BIT_7_inHL   = 0x7E;
    static constexpr const uint8_t BIT_7_A      = 0x7F;

    // 0x8*
    static constexpr const uint8_t RES_0_B      = 0x80;
    static constexpr const uint8_t RES_0_C      = 0x81;
    static constexpr const uint8_t RES_0_D      = 0x82;
    static constexpr const uint8_t RES_0_E      = 0x83;
    static constexpr const uint8_t RES_0_H      = 0x84;
    static constexpr const uint8_t RES_0_L      = 0x85;
    static constexpr const uint8_t RES_0_inHL   = 0x86;
    static constexpr const uint8_t RES_0_A      = 0x87;
    static constexpr const uint8_t RES_1_B      = 0x88;
    static constexpr const uint8_t RES_1_C      = 0x89;
    static constexpr const uint8_t RES_1_D      = 0x8A;
    static constexpr const uint8_t RES_1_E      = 0x8B;
    static constexpr const uint8_t RES_1_H      = 0x8C;
    static constexpr const uint8_t RES_1_L      = 0x8D;
    static constexpr const uint8_t RES_1_inHL   = 0x8E;
    static constexpr const uint8_t RES_1_A      = 0x8F;

    // 0x9*
    static constexpr const uint8_t RES_2_B      = 0x90;
    static constexpr const uint8_t RES_2_C      = 0x91;
    static constexpr const uint8_t RES_2_D      = 0x92;
    static constexpr const uint8_t RES_2_E      = 0x93;
    static constexpr const uint8_t RES_2_H      = 0x94;
    static constexpr const uint8_t RES_2_L      = 0x95;
    static constexpr const uint8_t RES_2_inHL   = 0x96;
    static constexpr const uint8_t RES_2_A      = 0x97;
    static constexpr const uint8_t RES_3_B      = 0x98;
    static constexpr const uint8_t RES_3_C      = 0x99;
    static constexpr const uint8_t RES_3_D      = 0x9A;
    static constexpr const uint8_t RES_3_E      = 0x9B;
    static constexpr const uint8_t RES_3_H      = 0x9C;
    static constexpr const uint8_t RES_3_L      = 0x9D;
    static constexpr const uint8_t RES_3_inHL   = 0x9E;
    static constexpr const uint8_t RES_3_A      = 0x9F;

    // 0xA*
    static constexpr const uint8_t RES_4_B      = 0xA0;
    static constexpr const uint8_t RES_4_C      = 0xA1;
    static constexpr const uint8_t RES_4_D      = 0xA2;
    static constexpr const uint8_t RES_4_E      = 0xA3;
    static constexpr const uint8_t RES_4_H      = 0xA4;
    static constexpr const uint8_t RES_4_L      = 0xA5;
    static constexpr const uint8_t RES_4_inHL   = 0xA6;
    static constexpr const uint8_t RES_4_A      = 0xA7;
    static constexpr const uint8_t RES_5_B      = 0xA8;
    static constexpr const uint8_t RES_5_C      = 0xA9;
    static constexpr const uint8_t RES_5_D      = 0xAA;
    static constexpr const uint8_t RES_5_E      = 0xAB;
    static constexpr const uint8_t RES_5_H      = 0xAC;
    static constexpr const uint8_t RES_5_L      = 0xAD;
    static constexpr const uint8_t RES_5_inHL   = 0xAE;
    static constexpr const uint8_t RES_5_A      = 0xAF;

    // 0xB*
    static constexpr const uint8_t RES_6_B      = 0xB0;
    static constexpr const uint8_t RES_6_C      = 0xB1;
    static constexpr const uint8_t RES_6_D      = 0xB2;
    static constexpr const uint8_t RES_6_E      = 0xB3;
    static constexpr const uint8_t RES_6_H      = 0xB4;
    static constexpr const uint8_t RES_6_L      = 0xB5;
    static constexpr const uint8_t RES_6_inHL   = 0xB6;
    static constexpr const uint8_t RES_6_A      = 0xB7;
    static constexpr const uint8_t RES_7_B      = 0xB8;
    static constexpr const uint8_t RES_7_C      = 0xB9;
    static constexpr const uint8_t RES_7_D      = 0xBA;
    static constexpr const uint8_t RES_7_E      = 0xBB;
    static constexpr const uint8_t RES_7_H      = 0xBC;
    static constexpr const uint8_t RES_7_L      = 0xBD;
    static constexpr const uint8_t RES_7_inHL   = 0xBE;
    static constexpr const uint8_t RES_7_A      = 0xBF;

    // 0xC*
    static constexpr const uint8_t SET_0_B      = 0xC0;
    static constexpr const uint8_t SET_0_C      = 0xC1;
    static constexpr const uint8_t SET_0_D      = 0xC2;
    static constexpr const uint8_t SET_0_E      = 0xC3;
    static constexpr const uint8_t SET_0_H      = 0xC4;
    static constexpr const uint8_t SET_0_L      = 0xC5;
    static constexpr const uint8_t SET_0_inHL   = 0xC6;
    static constexpr const uint8_t SET_0_A      = 0xC7;
    static constexpr const uint8_t SET_1_B      = 0xC8;
    static constexpr const uint8_t SET_1_C      = 0xC9;
    static constexpr const uint8_t SET_1_D      = 0xCA;
    static constexpr const uint8_t SET_1_E      = 0xCB;
    static constexpr const uint8_t SET_1_H      = 0xCC;
    static constexpr const uint8_t SET_1_L      = 0xCD;
    static constexpr const uint8_t SET_1_inHL   = 0xCE;
    static constexpr const uint8_t SET_1_A      = 0xCF;

    // 0xD*
    static constexpr const uint8_t SET_2_B      = 0xD0;
    static constexpr const uint8_t SET_2_C      = 0xD1;
    static constexpr const uint8_t SET_2_D      = 0xD2;
    static constexpr const uint8_t SET_2_E      = 0xD3;
    static constexpr const uint8_t SET_2_H      = 0xD4;
    static constexpr const uint8_t SET_2_L      = 0xD5;
    static constexpr const uint8_t SET_2_inHL   = 0xD6;
    static constexpr const uint8_t SET_2_A      = 0xD7;
    static constexpr const uint8_t SET_3_B      = 0xD8;
    static constexpr const uint8_t SET_3_C      = 0xD9;
    static constexpr const uint8_t SET_3_D      = 0xDA;
    static constexpr const uint8_t SET_3_E      = 0xDB;
    static constexpr const uint8_t SET_3_H      = 0xDC;
    static constexpr const uint8_t SET_3_L      = 0xDD;
    static constexpr const uint8_t SET_3_inHL   = 0xDE;
    static constexpr const uint8_t SET_3_A      = 0xDF;

    // 0xE*
    static constexpr const uint8_t SET_4_B      = 0xE0;
    static constexpr const uint8_t SET_4_C      = 0xE1;
    static constexpr const uint8_t SET_4_D      = 0xE2;
    static constexpr const uint8_t SET_4_E      = 0xE3;
    static constexpr const uint8_t SET_4_H      = 0xE4;
    static constexpr const uint8_t SET_4_L      = 0xE5;
    static constexpr const uint8_t SET_4_inHL   = 0xE6;
    static constexpr const uint8_t SET_4_A      = 0xE7;
    static constexpr const uint8_t SET_5_B      = 0xE8;
    static constexpr const uint8_t SET_5_C      = 0xE9;
    static constexpr const uint8_t SET_5_D      = 0xEA;
    static constexpr const uint8_t SET_5_E      = 0xEB;
    static constexpr const uint8_t SET_5_H      = 0xEC;
    static constexpr const uint8_t SET_5_L      = 0xED;
    static constexpr const uint8_t SET_5_inHL   = 0xEE;
    static constexpr const uint8_t SET_5_A      = 0xEF;

    // 0xF*
    static constexpr const uint8_t SET_6_B      = 0xF0;
    static constexpr const uint8_t SET_6_C      = 0xF1;
    static constexpr const uint8_t SET_6_D      = 0xF2;
    static constexpr const uint8_t SET_6_E      = 0xF3;
    static constexpr const uint8_t SET_6_H      = 0xF4;
    static constexpr const uint8_t SET_6_L      = 0xF5;
    static constexpr const uint8_t SET_6_inHL   = 0xF6;
    static constexpr const uint8_t SET_6_A      = 0xF7;
    static constexpr const uint8_t SET_7_B      = 0xF8;
    static constexpr const uint8_t SET_7_C      = 0xF9;
    static constexpr const uint8_t SET_7_D      = 0xFA;
    static constexpr const uint8_t SET_7_E      = 0xFB;
    static constexpr const uint8_t SET_7_H      = 0xFC;
    static constexpr const uint8_t SET_7_L      = 0xFD;
    static constexpr const uint8_t SET_7_inHL   = 0xFE;
    static constexpr const uint8_t SET_7_A      = 0xFF;

}
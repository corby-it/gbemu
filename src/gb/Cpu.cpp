
#include "Cpu.h"
#include "Opcodes.h"
#include "GbCommons.h"
#include <tracy/Tracy.hpp>
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
    flags.fromU8(0);
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

    mImeScheduled = false;

    mIsHalted = false;
    mCheckForHaltBug = false;
    mIsStopped = false;

    regs.reset();
    irqs.reset();
}

CpuStepRes CPU::step()
{
    ZoneScoped;

    // in one step we execute one instruction, the instruction can take 1 or more machine cycles

    bool triggerHaltBug = false;

    // if we're in the stopped state we don't execute instructions, nothing happens until a reset
    // or until one of the joypad lines goes low (not an interrupt, only the signal going low is enough!)
    // see the gameboy developer manual, page 23
    if (mIsStopped) {
        auto joypad = mBus.read8(mmap::regs::joypad);
        
        if ((joypad & 0x0F) != 0x0F) 
            mIsStopped = false;
        else 
            return { true, 1 };
    }

    // check if interrupt occurred
    auto irqRequest = irqs.getCurrentIrq();

    if (mCheckForHaltBug) {
        mCheckForHaltBug = false;

        if (!irqs.ime && irqRequest)
            triggerHaltBug = true;
    }
    
    if (irqRequest) {
        // if an interrupt has been requested we resume from the HALT state, even if the 
        // interrupt will not be handled 
        mIsHalted = false;
        
        // if IME is not set no interrupt will handled
        if (irqs.ime) {
            auto irqType = irqRequest.value();

            // as soon as an interrupt is serviced the IME flags is reset and the corresponding
            // bit in the IF register is reset as well
            irqs.ime = false;
            irqs.writeIF(irqs.readIF() & ~Irqs::mask(irqType));

            // calling an interrupt has the same effect as a CALL instruction
            auto cycles = opCallIrq(irqType);
            mCycles += cycles;

            return { true, cycles };
        }
    }
    
    
    // if the cpu is in the HALT state we don't execute instructions and return now
    if (mIsHalted)
        return { true, 1 };


    // if ime was scheduled to be set, set it now
    if (mImeScheduled) {
        irqs.ime = true;
        mImeScheduled = false;
    }

    // fetch opcode 
    auto opcode = mBus.read8(regs.PC++);

    // HALT bug, see: https://gbdev.io/pandocs/halt.html
    if (triggerHaltBug)
        regs.PC--;

    bool ok = true;
    auto cycles = execute(opcode, ok);
    mCycles += cycles;

    return { ok, cycles };
}


uint8_t CPU::execute(uint8_t opcode, bool& ok)
{
    // opcode reference: 
    // - https://gbdev.io/gb-opcodes/optables/
    // - https://gekkio.fi/files/gb-docs/gbctr.pdf

    // this function returns the number of machine cycles used to execute the instruction

    switch (opcode) {
    // 0x0*
    case op::NOP            : return 1; // nothing to do here
    case op::LD_BC_n16      : return opLdReg16Imm(regs.B, regs.C);
    case op::LD_inBC_A      : return opLdIndReg(regs.BC(), regs.A);
    case op::INC_BC         : return opIncReg16(regs.B, regs.C);
    case op::INC_B          : return opIncReg(regs.B);
    case op::DEC_B          : return opDecReg(regs.B);
    case op::LD_B_n8        : return opLdRegImm(regs.B); // LD B,n8
    case op::RLCA           : return opRlca();
    case op::LD_ina16_SP    : return opLdIndImm16Sp();
    case op::ADD_HL_BC      : return opAddReg16(regs.BC());
    case op::LD_A_inBC      : return opLdRegInd(regs.A, regs.BC());
    case op::DEC_BC         : return opDecReg16(regs.B, regs.C);
    case op::INC_C          : return opIncReg(regs.C);
    case op::DEC_C          : return opDecReg(regs.C);
    case op::LD_C_n8        : return opLdRegImm(regs.C); // LD C,n8
    case op::RRCA           : return opRrca();

    // 0x1*
    case op::STOP           : return opStop();
    case op::LD_DE_n16      : return opLdReg16Imm(regs.D, regs.E);
    case op::LD_inDE_A      : return opLdIndReg(regs.DE(), regs.A);
    case op::INC_DE         : return opIncReg16(regs.D, regs.E);
    case op::INC_D          : return opIncReg(regs.D);
    case op::DEC_D          : return opDecReg(regs.D);
    case op::LD_D_n8        : return opLdRegImm(regs.D); // LD D,n8
    case op::RLA            : return opRla();
    case op::JR_e8          : return opJrImm();
    case op::ADD_HL_DE      : return opAddReg16(regs.DE());
    case op::LD_A_inDE      : return opLdRegInd(regs.A, regs.DE());
    case op::DEC_DE         : return opDecReg16(regs.D, regs.E);
    case op::INC_E          : return opIncReg(regs.E);
    case op::DEC_E          : return opDecReg(regs.E);
    case op::LD_E_n8        : return opLdRegImm(regs.E); // LD E,n8
    case op::RRA            : return opRra();

    // 0x2*
    case op::JR_NZ_e8       : return opJrCond(!regs.flags.Z);
    case op::LD_HL_n16      : return opLdReg16Imm(regs.H, regs.L);
    case op::LD_inHLp_A     : return opLdIndIncA();
    case op::INC_HL         : return opIncReg16(regs.H, regs.L);
    case op::INC_H          : return opIncReg(regs.H);
    case op::DEC_H          : return opDecReg(regs.H);
    case op::LD_H_n8        : return opLdRegImm(regs.H); // LD H,n8
    case op::DAA            : return opDaa();
    case op::JR_Z_e8        : return opJrCond(regs.flags.Z);
    case op::ADD_HL_HL      : return opAddReg16(regs.HL());
    case op::LD_A_inHLp     : return opLdAIndInc();
    case op::DEC_HL         : return opDecReg16(regs.H, regs.L);
    case op::INC_L          : return opIncReg(regs.L);
    case op::DEC_L          : return opDecReg(regs.L);
    case op::LD_L_n8        : return opLdRegImm(regs.L); // LD L,n8
    case op::CPL            : return opCpl();

    // 0x3*
    case op::JR_NC_e8       : return opJrCond(!regs.flags.C);
    case op::LD_SP_n16      : return opLdReg16Imm(regs.SP);
    case op::LD_inHLm_A     : return opLdIndDecA();
    case op::INC_SP         : return opIncSp();
    case op::INC_inHL       : return opIncInd();
    case op::DEC_inHL       : return opDecInd();
    case op::LD_inHL_n8     : return opLdIndImm();
    case op::SCF            : return opScf();
    case op::JR_C_e8        : return opJrCond(regs.flags.C);
    case op::ADD_HL_SP      : return opAddReg16(regs.SP);
    case op::LD_A_inHLm     : return opLdAIndDec();
    case op::DEC_SP         : return opDecSp();
    case op::INC_A          : return opIncReg(regs.A);
    case op::DEC_A          : return opDecReg(regs.A);
    case op::LD_A_n8        : return opLdRegImm(regs.A); // LD A,n8
    case op::CCF            : return opCcf();

    // 0x4*
    case op::LD_B_B         : return opLdRegReg(regs.B, regs.B); // LD B,B
    case op::LD_B_C         : return opLdRegReg(regs.B, regs.C); // LD B,C
    case op::LD_B_D         : return opLdRegReg(regs.B, regs.D); // LD B,D
    case op::LD_B_E         : return opLdRegReg(regs.B, regs.E); // LD B,E
    case op::LD_B_H         : return opLdRegReg(regs.B, regs.H); // LD B,H
    case op::LD_B_L         : return opLdRegReg(regs.B, regs.L); // LD B,L
    case op::LD_B_inHL      : return opLdRegInd(regs.B, regs.HL()); // LD B,[HL]
    case op::LD_B_A         : return opLdRegReg(regs.B, regs.A); // LD B,A
    case op::LD_C_B         : return opLdRegReg(regs.C, regs.B); // LD C,B
    case op::LD_C_C         : return opLdRegReg(regs.C, regs.C); // LD C,C
    case op::LD_C_D         : return opLdRegReg(regs.C, regs.D); // LD C,D
    case op::LD_C_E         : return opLdRegReg(regs.C, regs.E); // LD C,E
    case op::LD_C_H         : return opLdRegReg(regs.C, regs.H); // LD C,H
    case op::LD_C_L         : return opLdRegReg(regs.C, regs.L); // LD C,L
    case op::LD_C_inHL      : return opLdRegInd(regs.C, regs.HL()); // LD C,[HL]
    case op::LD_C_A         : return opLdRegReg(regs.C, regs.A); // LD C,A

    // 0x5*
    case op::LD_D_B         : return opLdRegReg(regs.D, regs.B); // LD D,B
    case op::LD_D_C         : return opLdRegReg(regs.D, regs.C); // LD D,C
    case op::LD_D_D         : return opLdRegReg(regs.D, regs.D); // LD D,D
    case op::LD_D_E         : return opLdRegReg(regs.D, regs.E); // LD D,E
    case op::LD_D_H         : return opLdRegReg(regs.D, regs.H); // LD D,H
    case op::LD_D_L         : return opLdRegReg(regs.D, regs.L); // LD D,L
    case op::LD_D_inHL      : return opLdRegInd(regs.D, regs.HL()); // LD A,[HL]
    case op::LD_D_A         : return opLdRegReg(regs.D, regs.A); // LD D,A
    case op::LD_E_B         : return opLdRegReg(regs.E, regs.B); // LD E,B
    case op::LD_E_C         : return opLdRegReg(regs.E, regs.C); // LD E,C
    case op::LD_E_D         : return opLdRegReg(regs.E, regs.D); // LD E,D
    case op::LD_E_E         : return opLdRegReg(regs.E, regs.E); // LD E,E
    case op::LD_E_H         : return opLdRegReg(regs.E, regs.H); // LD E,H
    case op::LD_E_L         : return opLdRegReg(regs.E, regs.L); // LD E,L
    case op::LD_E_inHL      : return opLdRegInd(regs.E, regs.HL()); // LD E,[HL]
    case op::LD_E_A         : return opLdRegReg(regs.E, regs.A); // LD E,A

    // 0x6*
    case op::LD_H_B         : return opLdRegReg(regs.H, regs.B); // LD H,B
    case op::LD_H_C         : return opLdRegReg(regs.H, regs.C); // LD H,C
    case op::LD_H_D         : return opLdRegReg(regs.H, regs.D); // LD H,D
    case op::LD_H_E         : return opLdRegReg(regs.H, regs.E); // LD H,E
    case op::LD_H_H         : return opLdRegReg(regs.H, regs.H); // LD H,H
    case op::LD_H_L         : return opLdRegReg(regs.H, regs.L); // LD H,L
    case op::LD_H_inHL      : return opLdRegInd(regs.H, regs.HL()); // LD H,[HL]
    case op::LD_H_A         : return opLdRegReg(regs.H, regs.A); // LD H,A
    case op::LD_L_B         : return opLdRegReg(regs.L, regs.B); // LD L,B
    case op::LD_L_C         : return opLdRegReg(regs.L, regs.C); // LD L,C
    case op::LD_L_D         : return opLdRegReg(regs.L, regs.D); // LD L,D
    case op::LD_L_E         : return opLdRegReg(regs.L, regs.E); // LD L,E
    case op::LD_L_H         : return opLdRegReg(regs.L, regs.H); // LD L,H
    case op::LD_L_L         : return opLdRegReg(regs.L, regs.L); // LD L,L
    case op::LD_L_inHL      : return opLdRegInd(regs.L, regs.HL()); // LD L,[HL]
    case op::LD_L_A         : return opLdRegReg(regs.L, regs.A); // LD L,A

    // 0x7*
    case op::LD_inHl_B      : return opLdIndReg(regs.HL(), regs.B); // LD [HL],B
    case op::LD_inHl_C      : return opLdIndReg(regs.HL(), regs.C); // LD [HL],C
    case op::LD_inHl_D      : return opLdIndReg(regs.HL(), regs.D); // LD [HL],D
    case op::LD_inHl_E      : return opLdIndReg(regs.HL(), regs.E); // LD [HL],E
    case op::LD_inHl_H      : return opLdIndReg(regs.HL(), regs.H); // LD [HL],H
    case op::LD_inHl_L      : return opLdIndReg(regs.HL(), regs.L); // LD [HL],L
    case op::HALT           : return opHalt();
    case op::LD_inHl_A      : return opLdIndReg(regs.HL(), regs.A); // LD [HL],A
    case op::LD_A_B         : return opLdRegReg(regs.A, regs.B); // LD A,B
    case op::LD_A_C         : return opLdRegReg(regs.A, regs.C); // LD A,B
    case op::LD_A_D         : return opLdRegReg(regs.A, regs.D); // LD A,B
    case op::LD_A_E         : return opLdRegReg(regs.A, regs.E); // LD A,B
    case op::LD_A_H         : return opLdRegReg(regs.A, regs.H); // LD A,B
    case op::LD_A_L         : return opLdRegReg(regs.A, regs.L); // LD A,B
    case op::LD_A_inHL      : return opLdRegInd(regs.A, regs.HL()); // LD A,[HL]
    case op::LD_A_A         : return opLdRegReg(regs.A, regs.A); // LD A,A

    // 0x8*
    case op::ADD_A_B        : return opAddReg(regs.B); // ADD A,B
    case op::ADD_A_C        : return opAddReg(regs.C); // ADD A,C
    case op::ADD_A_D        : return opAddReg(regs.D); // ADD A,D
    case op::ADD_A_E        : return opAddReg(regs.E); // ADD A,E
    case op::ADD_A_H        : return opAddReg(regs.H); // ADD A,H
    case op::ADD_A_L        : return opAddReg(regs.L); // ADD A,L
    case op::ADD_A_inHL     : return opAddInd(); // ADD A,[HL]
    case op::ADD_A_A        : return opAddReg(regs.A); // ADD A,A
    case op::ADC_A_B        : return opAdcReg(regs.B);
    case op::ADC_A_C        : return opAdcReg(regs.C);
    case op::ADC_A_D        : return opAdcReg(regs.D);
    case op::ADC_A_E        : return opAdcReg(regs.E);
    case op::ADC_A_H        : return opAdcReg(regs.H);
    case op::ADC_A_L        : return opAdcReg(regs.L);
    case op::ADC_A_inHL     : return opAdcInd();
    case op::ADC_A_A        : return opAdcReg(regs.A);

    // 0x9*
    case op::SUB_A_B        : return opSubReg(regs.B);
    case op::SUB_A_C        : return opSubReg(regs.C);
    case op::SUB_A_D        : return opSubReg(regs.D);
    case op::SUB_A_E        : return opSubReg(regs.E);
    case op::SUB_A_H        : return opSubReg(regs.H);
    case op::SUB_A_L        : return opSubReg(regs.L);
    case op::SUB_A_inHL     : return opSubInd();
    case op::SUB_A_A        : return opSubReg(regs.A);
    case op::SBC_A_B        : return opSbcReg(regs.B);
    case op::SBC_A_C        : return opSbcReg(regs.C);
    case op::SBC_A_D        : return opSbcReg(regs.D);
    case op::SBC_A_E        : return opSbcReg(regs.E);
    case op::SBC_A_H        : return opSbcReg(regs.H);
    case op::SBC_A_L        : return opSbcReg(regs.L);
    case op::SBC_A_inHL     : return opSbcInd();
    case op::SBC_A_A        : return opSbcReg(regs.A);

    // 0xA*
    case op::AND_A_B        : return opAndReg(regs.B);
    case op::AND_A_C        : return opAndReg(regs.C);
    case op::AND_A_D        : return opAndReg(regs.D);
    case op::AND_A_E        : return opAndReg(regs.E);
    case op::AND_A_H        : return opAndReg(regs.H);
    case op::AND_A_L        : return opAndReg(regs.L);
    case op::AND_A_inHL     : return opAndInd();
    case op::AND_A_A        : return opAndReg(regs.A);
    case op::XOR_A_B        : return opXorReg(regs.B);
    case op::XOR_A_C        : return opXorReg(regs.C);
    case op::XOR_A_D        : return opXorReg(regs.D);
    case op::XOR_A_E        : return opXorReg(regs.E);
    case op::XOR_A_H        : return opXorReg(regs.H);
    case op::XOR_A_L        : return opXorReg(regs.L);
    case op::XOR_A_inHL     : return opXorInd();
    case op::XOR_A_A        : return opXorReg(regs.A);

    // 0xB*
    case op::OR_A_B         : return opOrReg(regs.B);
    case op::OR_A_C         : return opOrReg(regs.C);
    case op::OR_A_D         : return opOrReg(regs.D);
    case op::OR_A_E         : return opOrReg(regs.E);
    case op::OR_A_H         : return opOrReg(regs.H);
    case op::OR_A_L         : return opOrReg(regs.L);
    case op::OR_A_inHL      : return opOrInd();
    case op::OR_A_A         : return opOrReg(regs.A);
    case op::CP_A_B         : return opCpReg(regs.B);
    case op::CP_A_C         : return opCpReg(regs.C);
    case op::CP_A_D         : return opCpReg(regs.D);
    case op::CP_A_E         : return opCpReg(regs.E);
    case op::CP_A_H         : return opCpReg(regs.H);
    case op::CP_A_L         : return opCpReg(regs.L);
    case op::CP_A_inHL      : return opCpInd(); // CP A,[HL]
    case op::CP_A_A         : return opCpReg(regs.A);

    // 0xC*
    case op::RET_NZ         : return opRetCond(!regs.flags.Z);
    case op::POP_BC         : return opPopReg16(regs.B, regs.C);
    case op::JP_NZ_a16      : return opJpCond(!regs.flags.Z); // JP NZ,a16
    case op::JP_a16         : return opJpImm(); // JP a16
    case op::CALL_NZ_a16    : return opCallCond(!regs.flags.Z);
    case op::PUSH_BC        : return opPushReg16(regs.BC());
    case op::ADD_A_n8       : return opAddImm();
    case op::RST_00         : return opRst(0x00);
    case op::RET_Z          : return opRetCond(regs.flags.Z);
    case op::RET            : return opRet();
    case op::JP_Z_a16       : return opJpCond(regs.flags.Z); // JP Z,a16
    case op::CB_PREFIX      : return executeCb(ok);
    case op::CALL_Z_a16     : return opCallCond(regs.flags.Z);
    case op::CALL_a16       : return opCallImm();
    case op::ADC_A_n8       : return opAdcImm();
    case op::RST_08         : return opRst(0x08);

    // 0xD*
    case op::RET_NC         : return opRetCond(!regs.flags.C);
    case op::POP_DE         : return opPopReg16(regs.D, regs.E);
    case op::JP_NC_a16      : return opJpCond(!regs.flags.C); // JP NC,a16
    // case op:: 0xD3 not implemented
    case op::CALL_NC_a16    : return opCallCond(!regs.flags.C);
    case op::PUSH_DE        : return opPushReg16(regs.DE());
    case op::SUB_A_n8       : return opSubImm();
    case op::RST_10         : return opRst(0x10);
    case op::RET_C          : return opRetCond(regs.flags.C);
    case op::RETI           : return opReti();
    case op::JP_C_a16       : return opJpCond(regs.flags.C); // JP C,a16
    // case op:: 0xDB not implemented
    case op::CALL_C_a16     : return opCallCond(regs.flags.C);
    // case op:: 0xDD not implemented
    case op::SBC_A_n8       : return opSbcImm();
    case op::RST_18         : return opRst(0x18);

    // 0xE*
    case op::LDH_ina8_A     : return opLdIndImm8Reg();
    case op::POP_HL         : return opPopReg16(regs.H, regs.L);
    case op::LDH_inC_A      : return opLdIndReg(0xFF00 + regs.C, regs.A); // LD [C],A  (address is 0xff00 + C)
    // case op:: 0xE3 not implemented
    // case op:: 0xE4 not implemented
    case op::PUSH_HL        : return opPushReg16(regs.HL());
    case op::AND_A_n8       : return opAndImm();
    case op::RST_20         : return opRst(0x20);
    case op::ADD_SP_e8      : return opAddSpImm();
    case op::JP_HL          : return opJpHL(); // JP HL
    case op::LD_ina16_A     : return opLdIndImm16Reg();
    // case op:: 0xEB not implemented
    // case op:: 0xEC not implemented
    // case op:: 0xED not implemented
    case op::XOR_A_n8       : return opXorImm();
    case op::RST_28         : return opRst(0x28);

    // 0xF*
    case op::LDH_A_ina8     : return opLdRegIndImm8();
    case op::POP_AF         : return opPopReg16(regs.A, regs.flags);
    case op::LDH_A_inC      : return opLdRegInd(regs.A, 0xFF00 + regs.C);  // LD A,[C]  (actual address is 0xff00 + C)
    case op::DI             : return opDi();
    // case op:: 0xF4 not implemented
    case op::PUSH_AF        : return opPushReg16(regs.AF());
    case op::OR_A_n8        : return opOrImm();
    case op::RST_30         : return opRst(0x30);
    case op::LD_HL_SPpe8    : return opLdHlSpOffset();
    case op::LD_SP_HL       : return opLdSpHl();
    case op::LD_A_ina16     : return opLdRegIndImm16();
    case op::EI             : return opEi();
    // case op:: 0xFB not implemented
    // case op:: 0xFC not implemented
    case op::CP_A_n8        : return opCpImm();
    case op::RST_38         : return opRst(0x38);

    default:
        // unrecognized opcode
        ok = false;
        return 1;
    }
}

uint8_t CPU::executeCb(bool& ok)
{
    // to correctly execute one of the instructions prefixed with CB we 
    // have to read another byte from PC to get the actual opcode
    uint8_t cbOpcode = mBus.read8(regs.PC++);

    switch (cbOpcode) {
        // 0x0*
        case op_cb::RLC_B        : return opCbRlcReg(regs.B);
        case op_cb::RLC_C        : return opCbRlcReg(regs.C);
        case op_cb::RLC_D        : return opCbRlcReg(regs.D);
        case op_cb::RLC_E        : return opCbRlcReg(regs.E);
        case op_cb::RLC_H        : return opCbRlcReg(regs.H);
        case op_cb::RLC_L        : return opCbRlcReg(regs.L);
        case op_cb::RLC_inHL     : return opCbRlcInd();
        case op_cb::RLC_A        : return opCbRlcReg(regs.A);
        case op_cb::RRC_B        : return opCbRrcReg(regs.B);
        case op_cb::RRC_C        : return opCbRrcReg(regs.C);
        case op_cb::RRC_D        : return opCbRrcReg(regs.D);
        case op_cb::RRC_E        : return opCbRrcReg(regs.E);
        case op_cb::RRC_H        : return opCbRrcReg(regs.H);
        case op_cb::RRC_L        : return opCbRrcReg(regs.L);
        case op_cb::RRC_inHL     : return opCbRrcInd();
        case op_cb::RRC_A        : return opCbRrcReg(regs.A);

        // 0x1*
        case op_cb::RL_B         : return opCbRlReg(regs.B);
        case op_cb::RL_C         : return opCbRlReg(regs.C);
        case op_cb::RL_D         : return opCbRlReg(regs.D);
        case op_cb::RL_E         : return opCbRlReg(regs.E);
        case op_cb::RL_H         : return opCbRlReg(regs.H);
        case op_cb::RL_L         : return opCbRlReg(regs.L);
        case op_cb::RL_inHL      : return opCbRlInd();
        case op_cb::RL_A         : return opCbRlReg(regs.A);
        case op_cb::RR_B         : return opCbRrReg(regs.B);
        case op_cb::RR_C         : return opCbRrReg(regs.C);
        case op_cb::RR_D         : return opCbRrReg(regs.D);
        case op_cb::RR_E         : return opCbRrReg(regs.E);
        case op_cb::RR_H         : return opCbRrReg(regs.H);
        case op_cb::RR_L         : return opCbRrReg(regs.L);
        case op_cb::RR_inHL      : return opCbRrInd();
        case op_cb::RR_A         : return opCbRrReg(regs.A);

        // 0x2*
        case op_cb::SLA_B        : return opCbSlaReg(regs.B);
        case op_cb::SLA_C        : return opCbSlaReg(regs.C);
        case op_cb::SLA_D        : return opCbSlaReg(regs.D);
        case op_cb::SLA_E        : return opCbSlaReg(regs.E);
        case op_cb::SLA_H        : return opCbSlaReg(regs.H);
        case op_cb::SLA_L        : return opCbSlaReg(regs.L);
        case op_cb::SLA_inHL     : return opCbSlaInd();
        case op_cb::SLA_A        : return opCbSlaReg(regs.A);
        case op_cb::SRA_B        : return opCbSraReg(regs.B);
        case op_cb::SRA_C        : return opCbSraReg(regs.C);
        case op_cb::SRA_D        : return opCbSraReg(regs.D);
        case op_cb::SRA_E        : return opCbSraReg(regs.E);
        case op_cb::SRA_H        : return opCbSraReg(regs.H);
        case op_cb::SRA_L        : return opCbSraReg(regs.L);
        case op_cb::SRA_inHL     : return opCbSraInd();
        case op_cb::SRA_A        : return opCbSraReg(regs.A);

        // 0x3*
        case op_cb::SWAP_B       : return opCbSwapReg(regs.B);
        case op_cb::SWAP_C       : return opCbSwapReg(regs.C);
        case op_cb::SWAP_D       : return opCbSwapReg(regs.D);
        case op_cb::SWAP_E       : return opCbSwapReg(regs.E);
        case op_cb::SWAP_H       : return opCbSwapReg(regs.H);
        case op_cb::SWAP_L       : return opCbSwapReg(regs.L);
        case op_cb::SWAP_inHL    : return opCbSwapInd();
        case op_cb::SWAP_A       : return opCbSwapReg(regs.A);
        case op_cb::SRL_B        : return opCbSrlReg(regs.B);
        case op_cb::SRL_C        : return opCbSrlReg(regs.C);
        case op_cb::SRL_D        : return opCbSrlReg(regs.D);
        case op_cb::SRL_E        : return opCbSrlReg(regs.E);
        case op_cb::SRL_H        : return opCbSrlReg(regs.H);
        case op_cb::SRL_L        : return opCbSrlReg(regs.L);
        case op_cb::SRL_inHL     : return opCbSrlInd();
        case op_cb::SRL_A        : return opCbSrlReg(regs.A);

        // 0x4*
        case op_cb::BIT_0_B      : return opCbBitReg(0, regs.B);
        case op_cb::BIT_0_C      : return opCbBitReg(0, regs.C);
        case op_cb::BIT_0_D      : return opCbBitReg(0, regs.D);
        case op_cb::BIT_0_E      : return opCbBitReg(0, regs.E);
        case op_cb::BIT_0_H      : return opCbBitReg(0, regs.H);
        case op_cb::BIT_0_L      : return opCbBitReg(0, regs.L);
        case op_cb::BIT_0_inHL   : return opCbBitInd(0);
        case op_cb::BIT_0_A      : return opCbBitReg(0, regs.A);
        case op_cb::BIT_1_B      : return opCbBitReg(1, regs.B);
        case op_cb::BIT_1_C      : return opCbBitReg(1, regs.C);
        case op_cb::BIT_1_D      : return opCbBitReg(1, regs.D);
        case op_cb::BIT_1_E      : return opCbBitReg(1, regs.E);
        case op_cb::BIT_1_H      : return opCbBitReg(1, regs.H);
        case op_cb::BIT_1_L      : return opCbBitReg(1, regs.L);
        case op_cb::BIT_1_inHL   : return opCbBitInd(1);
        case op_cb::BIT_1_A      : return opCbBitReg(1, regs.A);

        // 0x5*
        case op_cb::BIT_2_B      : return opCbBitReg(2, regs.B);
        case op_cb::BIT_2_C      : return opCbBitReg(2, regs.C);
        case op_cb::BIT_2_D      : return opCbBitReg(2, regs.D);
        case op_cb::BIT_2_E      : return opCbBitReg(2, regs.E);
        case op_cb::BIT_2_H      : return opCbBitReg(2, regs.H);
        case op_cb::BIT_2_L      : return opCbBitReg(2, regs.L);
        case op_cb::BIT_2_inHL   : return opCbBitInd(2);
        case op_cb::BIT_2_A      : return opCbBitReg(2, regs.A);
        case op_cb::BIT_3_B      : return opCbBitReg(3, regs.B);
        case op_cb::BIT_3_C      : return opCbBitReg(3, regs.C);
        case op_cb::BIT_3_D      : return opCbBitReg(3, regs.D);
        case op_cb::BIT_3_E      : return opCbBitReg(3, regs.E);
        case op_cb::BIT_3_H      : return opCbBitReg(3, regs.H);
        case op_cb::BIT_3_L      : return opCbBitReg(3, regs.L);
        case op_cb::BIT_3_inHL   : return opCbBitInd(3);
        case op_cb::BIT_3_A      : return opCbBitReg(3, regs.A);

        // 0x6*
        case op_cb::BIT_4_B      : return opCbBitReg(4, regs.B);
        case op_cb::BIT_4_C      : return opCbBitReg(4, regs.C);
        case op_cb::BIT_4_D      : return opCbBitReg(4, regs.D);
        case op_cb::BIT_4_E      : return opCbBitReg(4, regs.E);
        case op_cb::BIT_4_H      : return opCbBitReg(4, regs.H);
        case op_cb::BIT_4_L      : return opCbBitReg(4, regs.L);
        case op_cb::BIT_4_inHL   : return opCbBitInd(4);
        case op_cb::BIT_4_A      : return opCbBitReg(4, regs.A);
        case op_cb::BIT_5_B      : return opCbBitReg(5, regs.B);
        case op_cb::BIT_5_C      : return opCbBitReg(5, regs.C);
        case op_cb::BIT_5_D      : return opCbBitReg(5, regs.D);
        case op_cb::BIT_5_E      : return opCbBitReg(5, regs.E);
        case op_cb::BIT_5_H      : return opCbBitReg(5, regs.H);
        case op_cb::BIT_5_L      : return opCbBitReg(5, regs.L);
        case op_cb::BIT_5_inHL   : return opCbBitInd(5);
        case op_cb::BIT_5_A      : return opCbBitReg(5, regs.A);

        // 0x7*
        case op_cb::BIT_6_B      : return opCbBitReg(6, regs.B);
        case op_cb::BIT_6_C      : return opCbBitReg(6, regs.C);
        case op_cb::BIT_6_D      : return opCbBitReg(6, regs.D);
        case op_cb::BIT_6_E      : return opCbBitReg(6, regs.E);
        case op_cb::BIT_6_H      : return opCbBitReg(6, regs.H);
        case op_cb::BIT_6_L      : return opCbBitReg(6, regs.L);
        case op_cb::BIT_6_inHL   : return opCbBitInd(6);
        case op_cb::BIT_6_A      : return opCbBitReg(6, regs.A);
        case op_cb::BIT_7_B      : return opCbBitReg(7, regs.B);
        case op_cb::BIT_7_C      : return opCbBitReg(7, regs.C);
        case op_cb::BIT_7_D      : return opCbBitReg(7, regs.D);
        case op_cb::BIT_7_E      : return opCbBitReg(7, regs.E);
        case op_cb::BIT_7_H      : return opCbBitReg(7, regs.H);
        case op_cb::BIT_7_L      : return opCbBitReg(7, regs.L);
        case op_cb::BIT_7_inHL   : return opCbBitInd(7);
        case op_cb::BIT_7_A      : return opCbBitReg(7, regs.A);

        // 0x8*
        case op_cb::RES_0_B      : return opCbResReg(0, regs.B);
        case op_cb::RES_0_C      : return opCbResReg(0, regs.C);
        case op_cb::RES_0_D      : return opCbResReg(0, regs.D);
        case op_cb::RES_0_E      : return opCbResReg(0, regs.E);
        case op_cb::RES_0_H      : return opCbResReg(0, regs.H);
        case op_cb::RES_0_L      : return opCbResReg(0, regs.L);
        case op_cb::RES_0_inHL   : return opCbResInd(0);
        case op_cb::RES_0_A      : return opCbResReg(0, regs.A);
        case op_cb::RES_1_B      : return opCbResReg(1, regs.B);
        case op_cb::RES_1_C      : return opCbResReg(1, regs.C);
        case op_cb::RES_1_D      : return opCbResReg(1, regs.D);
        case op_cb::RES_1_E      : return opCbResReg(1, regs.E);
        case op_cb::RES_1_H      : return opCbResReg(1, regs.H);
        case op_cb::RES_1_L      : return opCbResReg(1, regs.L);
        case op_cb::RES_1_inHL   : return opCbResInd(1);
        case op_cb::RES_1_A      : return opCbResReg(1, regs.A);

        // 0x9*
        case op_cb::RES_2_B      : return opCbResReg(2, regs.B);
        case op_cb::RES_2_C      : return opCbResReg(2, regs.C);
        case op_cb::RES_2_D      : return opCbResReg(2, regs.D);
        case op_cb::RES_2_E      : return opCbResReg(2, regs.E);
        case op_cb::RES_2_H      : return opCbResReg(2, regs.H);
        case op_cb::RES_2_L      : return opCbResReg(2, regs.L);
        case op_cb::RES_2_inHL   : return opCbResInd(2);
        case op_cb::RES_2_A      : return opCbResReg(2, regs.A);
        case op_cb::RES_3_B      : return opCbResReg(3, regs.B);
        case op_cb::RES_3_C      : return opCbResReg(3, regs.C);
        case op_cb::RES_3_D      : return opCbResReg(3, regs.D);
        case op_cb::RES_3_E      : return opCbResReg(3, regs.E);
        case op_cb::RES_3_H      : return opCbResReg(3, regs.H);
        case op_cb::RES_3_L      : return opCbResReg(3, regs.L);
        case op_cb::RES_3_inHL   : return opCbResInd(3);
        case op_cb::RES_3_A      : return opCbResReg(3, regs.A);

        // 0xA*
        case op_cb::RES_4_B      : return opCbResReg(4, regs.B);
        case op_cb::RES_4_C      : return opCbResReg(4, regs.C);
        case op_cb::RES_4_D      : return opCbResReg(4, regs.D);
        case op_cb::RES_4_E      : return opCbResReg(4, regs.E);
        case op_cb::RES_4_H      : return opCbResReg(4, regs.H);
        case op_cb::RES_4_L      : return opCbResReg(4, regs.L);
        case op_cb::RES_4_inHL   : return opCbResInd(4);
        case op_cb::RES_4_A      : return opCbResReg(4, regs.A);
        case op_cb::RES_5_B      : return opCbResReg(5, regs.B);
        case op_cb::RES_5_C      : return opCbResReg(5, regs.C);
        case op_cb::RES_5_D      : return opCbResReg(5, regs.D);
        case op_cb::RES_5_E      : return opCbResReg(5, regs.E);
        case op_cb::RES_5_H      : return opCbResReg(5, regs.H);
        case op_cb::RES_5_L      : return opCbResReg(5, regs.L);
        case op_cb::RES_5_inHL   : return opCbResInd(5);
        case op_cb::RES_5_A      : return opCbResReg(5, regs.A);

        // 0xB*
        case op_cb::RES_6_B      : return opCbResReg(6, regs.B);
        case op_cb::RES_6_C      : return opCbResReg(6, regs.C);
        case op_cb::RES_6_D      : return opCbResReg(6, regs.D);
        case op_cb::RES_6_E      : return opCbResReg(6, regs.E);
        case op_cb::RES_6_H      : return opCbResReg(6, regs.H);
        case op_cb::RES_6_L      : return opCbResReg(6, regs.L);
        case op_cb::RES_6_inHL   : return opCbResInd(6);
        case op_cb::RES_6_A      : return opCbResReg(6, regs.A);
        case op_cb::RES_7_B      : return opCbResReg(7, regs.B);
        case op_cb::RES_7_C      : return opCbResReg(7, regs.C);
        case op_cb::RES_7_D      : return opCbResReg(7, regs.D);
        case op_cb::RES_7_E      : return opCbResReg(7, regs.E);
        case op_cb::RES_7_H      : return opCbResReg(7, regs.H);
        case op_cb::RES_7_L      : return opCbResReg(7, regs.L);
        case op_cb::RES_7_inHL   : return opCbResInd(7);
        case op_cb::RES_7_A      : return opCbResReg(7, regs.A);

        // 0xC*
        case op_cb::SET_0_B      : return opCbSetReg(0, regs.B);
        case op_cb::SET_0_C      : return opCbSetReg(0, regs.C);
        case op_cb::SET_0_D      : return opCbSetReg(0, regs.D);
        case op_cb::SET_0_E      : return opCbSetReg(0, regs.E);
        case op_cb::SET_0_H      : return opCbSetReg(0, regs.H);
        case op_cb::SET_0_L      : return opCbSetReg(0, regs.L);
        case op_cb::SET_0_inHL   : return opCbSetInd(0);
        case op_cb::SET_0_A      : return opCbSetReg(0, regs.A);
        case op_cb::SET_1_B      : return opCbSetReg(1, regs.B);
        case op_cb::SET_1_C      : return opCbSetReg(1, regs.C);
        case op_cb::SET_1_D      : return opCbSetReg(1, regs.D);
        case op_cb::SET_1_E      : return opCbSetReg(1, regs.E);
        case op_cb::SET_1_H      : return opCbSetReg(1, regs.H);
        case op_cb::SET_1_L      : return opCbSetReg(1, regs.L);
        case op_cb::SET_1_inHL   : return opCbSetInd(1);
        case op_cb::SET_1_A      : return opCbSetReg(1, regs.A);

        // 0xD*
        case op_cb::SET_2_B      : return opCbSetReg(2, regs.B);
        case op_cb::SET_2_C      : return opCbSetReg(2, regs.C);
        case op_cb::SET_2_D      : return opCbSetReg(2, regs.D);
        case op_cb::SET_2_E      : return opCbSetReg(2, regs.E);
        case op_cb::SET_2_H      : return opCbSetReg(2, regs.H);
        case op_cb::SET_2_L      : return opCbSetReg(2, regs.L);
        case op_cb::SET_2_inHL   : return opCbSetInd(2);
        case op_cb::SET_2_A      : return opCbSetReg(2, regs.A);
        case op_cb::SET_3_B      : return opCbSetReg(3, regs.B);
        case op_cb::SET_3_C      : return opCbSetReg(3, regs.C);
        case op_cb::SET_3_D      : return opCbSetReg(3, regs.D);
        case op_cb::SET_3_E      : return opCbSetReg(3, regs.E);
        case op_cb::SET_3_H      : return opCbSetReg(3, regs.H);
        case op_cb::SET_3_L      : return opCbSetReg(3, regs.L);
        case op_cb::SET_3_inHL   : return opCbSetInd(3);
        case op_cb::SET_3_A      : return opCbSetReg(3, regs.A);

        // 0xE*
        case op_cb::SET_4_B      : return opCbSetReg(4, regs.B);
        case op_cb::SET_4_C      : return opCbSetReg(4, regs.C);
        case op_cb::SET_4_D      : return opCbSetReg(4, regs.D);
        case op_cb::SET_4_E      : return opCbSetReg(4, regs.E);
        case op_cb::SET_4_H      : return opCbSetReg(4, regs.H);
        case op_cb::SET_4_L      : return opCbSetReg(4, regs.L);
        case op_cb::SET_4_inHL   : return opCbSetInd(4);
        case op_cb::SET_4_A      : return opCbSetReg(4, regs.A);
        case op_cb::SET_5_B      : return opCbSetReg(5, regs.B);
        case op_cb::SET_5_C      : return opCbSetReg(5, regs.C);
        case op_cb::SET_5_D      : return opCbSetReg(5, regs.D);
        case op_cb::SET_5_E      : return opCbSetReg(5, regs.E);
        case op_cb::SET_5_H      : return opCbSetReg(5, regs.H);
        case op_cb::SET_5_L      : return opCbSetReg(5, regs.L);
        case op_cb::SET_5_inHL   : return opCbSetInd(5);
        case op_cb::SET_5_A      : return opCbSetReg(5, regs.A);

        // 0xF*
        case op_cb::SET_6_B      : return opCbSetReg(6, regs.B);
        case op_cb::SET_6_C      : return opCbSetReg(6, regs.C);
        case op_cb::SET_6_D      : return opCbSetReg(6, regs.D);
        case op_cb::SET_6_E      : return opCbSetReg(6, regs.E);
        case op_cb::SET_6_H      : return opCbSetReg(6, regs.H);
        case op_cb::SET_6_L      : return opCbSetReg(6, regs.L);
        case op_cb::SET_6_inHL   : return opCbSetInd(6);
        case op_cb::SET_6_A      : return opCbSetReg(6, regs.A);
        case op_cb::SET_7_B      : return opCbSetReg(7, regs.B);
        case op_cb::SET_7_C      : return opCbSetReg(7, regs.C);
        case op_cb::SET_7_D      : return opCbSetReg(7, regs.D);
        case op_cb::SET_7_E      : return opCbSetReg(7, regs.E);
        case op_cb::SET_7_H      : return opCbSetReg(7, regs.H);
        case op_cb::SET_7_L      : return opCbSetReg(7, regs.L);
        case op_cb::SET_7_inHL   : return opCbSetInd(7);
        case op_cb::SET_7_A      : return opCbSetReg(7, regs.A);
        default:
            // unrecognized opcode
            // shouldn't happen with CB prefixed instructions
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

uint8_t CPU::opLdHlSpOffset()
{
    // LD HL,SP+e8
    // loads in HL the stack pointer value plus an immediate 1-byte signed offset
    // Z and N are always zero
    // H is 1 if there is a carry from bit 3
    // C is 1 if there is a carry from bit 7
    // 3 cycles

    // NOTE: the gameboy programming manual states that the H and C flags are checked 
    // in bits 11 and 15 respectively BUT, many other emulators check those flags 
    // in bits 3 and 7 respectively, as if this wasn't a 16 bit operation but an 8 bit one

    int16_t val = (int8_t)mBus.read8(regs.PC++);

    regs.setHL(regs.SP + val);

    regs.flags.C = checkCarry(regs.SP, (uint16_t)val);
    regs.flags.H = checkHalfCarry((uint8_t)regs.SP, (uint8_t)val);
    regs.flags.Z = false;
    regs.flags.N = false;

    return 3;
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
    regs.flags.C = checkCarry(regs.A, rhs);

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
    regs.flags.C = checkCarry(regs.A, rhs, regs.flags.C);

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

    bool prevC = regs.flags.C;
    int16_t res = regs.A - (rhs + prevC);

    // set N because a subtraction just happened
    regs.flags.Z = (uint8_t)res == 0;
    regs.flags.H = checkHalfBorrow(regs.A, rhs, prevC);
    regs.flags.N = true;
    regs.flags.C = checkBorrow(regs.A, rhs, prevC);

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


uint8_t CPU::opAndCommon(uint8_t rhs, uint8_t cycles)
{
    // perform a bitwise AND between A and another operand, the 
    // result is stored in A
    // e.g.: A = A & rhs

    regs.A &= rhs;

    // H is always 1, C and N are always 0, Z depends on the result
    regs.flags.Z = regs.A == 0;
    regs.flags.H = true;
    regs.flags.N = false;
    regs.flags.C = false;

    return cycles;
}

uint8_t CPU::opAndReg(uint8_t reg)
{
    // AND A,reg
    // 1 cycle
    return opAndCommon(reg, 1);
}

uint8_t CPU::opAndInd()
{
    // AND A,[HL]
    // 2 cycles
    return opAndCommon(mBus.read8(regs.HL()), 2);
}

uint8_t CPU::opAndImm()
{
    // AND A,n8
    // 2 cycles
    return opAndCommon(mBus.read8(regs.PC++), 2);
}

uint8_t CPU::opOrCommon(uint8_t rhs, uint8_t cycles)
{
    // perform a bitwise OR between A and another operand, the 
    // result is stored in A
    // e.g.: A = A | rhs

    regs.A |= rhs;

    // H, C and N are always 0, Z depends on the result
    regs.flags.Z = regs.A == 0;
    regs.flags.H = false;
    regs.flags.N = false;
    regs.flags.C = false;

    return cycles;
}

uint8_t CPU::opOrReg(uint8_t reg)
{
    // OR A,reg
    // 1 cycle
    return opOrCommon(reg, 1);
}

uint8_t CPU::opOrInd()
{
    // OR A,[HL]
    // 2 cycles
    return opOrCommon(mBus.read8(regs.HL()), 2);
}

uint8_t CPU::opOrImm()
{
    // OR A,n8
    // 2 cycles
    return opOrCommon(mBus.read8(regs.PC++), 2);
}

uint8_t CPU::opXorCommon(uint8_t rhs, uint8_t cycles)
{
    // perform a bitwise XOR between A and another operand, the 
    // result is stored in A
    // e.g.: A = A ^ rhs

    regs.A ^= rhs;

    // H, C and N are always 0, Z depends on the result
    regs.flags.Z = regs.A == 0;
    regs.flags.H = false;
    regs.flags.N = false;
    regs.flags.C = false;

    return cycles;
}

uint8_t CPU::opXorReg(uint8_t reg)
{
    // XOR A,reg
    // 1 cycle
    return opXorCommon(reg, 1);
}

uint8_t CPU::opXorInd()
{
    // XOR A,[HL]
    // 2 cycles
    return opXorCommon(mBus.read8(regs.HL()), 2);
}

uint8_t CPU::opXorImm()
{
    // XOR A,n8
    // 2 cycles
    return opXorCommon(mBus.read8(regs.PC++), 2);
}

uint8_t CPU::opCpCommon(uint8_t rhs, uint8_t cycles)
{
    // CP A,val
    // compares A with another value, it performs A - value
    // but the result is not stored, all flags are updated accordingly

    // set N because a subtraction just happened
    regs.flags.Z = regs.A == rhs;
    regs.flags.H = checkHalfBorrow(regs.A, rhs);
    regs.flags.N = true;
    regs.flags.C = checkBorrow(regs.A, rhs);

    return cycles;
}

uint8_t CPU::opCpReg(uint8_t reg)
{
    // CP A,reg
    // 1 cycle
    return opCpCommon(reg, 1);
}

uint8_t CPU::opCpInd()
{
    // CP A,[HL]
    // 2 cycles
    return opCpCommon(mBus.read8(regs.HL()), 2);
}

uint8_t CPU::opCpImm()
{
    // CP A,n8
    // 2 cycles
    return opCpCommon(mBus.read8(regs.PC++), 2);
}

uint8_t CPU::opIncReg(uint8_t& reg)
{
    // INC reg
    // increment the content of reg by 1, set flags accordingly apart from C
    // which is never set in this case

    regs.flags.Z = reg == 0xff;
    regs.flags.H = checkHalfCarry(reg, 1);
    regs.flags.N = false;

    reg++;

    return 1;
}

uint8_t CPU::opIncInd()
{
    // INC [HL]
    // increment the content of mem[HL] by 1, set flags accordingly apart from C
    // which is never set in this case

    uint8_t val = mBus.read8(regs.HL());

    regs.flags.Z = val == 0xff;
    regs.flags.H = checkHalfCarry(val, 1);
    regs.flags.N = false;

    mBus.write8(regs.HL(), val + 1);

    return 3;
}

uint8_t CPU::opDecReg(uint8_t& reg)
{
    // DEC reg
    // decrement the content of reg by 1, set flags accordingly apart from C
    // which is never set in this case

    regs.flags.Z = reg == 0x01;
    regs.flags.H = checkHalfBorrow(reg, 1);
    regs.flags.N = true;

    reg--;

    return 1;
}

uint8_t CPU::opDecInd()
{
    // DEC [HL]
    // decrement the content of mem[HL] by 1, set flags accordingly apart from C
    // which is never set in this case

    uint8_t val = mBus.read8(regs.HL());

    regs.flags.Z = val == 0x01;
    regs.flags.H = checkHalfBorrow(val, 1);
    regs.flags.N = true;

    mBus.write8(regs.HL(), val - 1);

    return 3;
}

uint8_t CPU::opCcf()
{
    // CCF
    // flip carry flag and clear H and N, Z unchanged

    regs.flags.C = !regs.flags.C;
    regs.flags.H = false;
    regs.flags.N = false;

    return 1;
}

uint8_t CPU::opScf()
{
    // SCF 
    // set carry flag and clear H and N, Z unchanged

    regs.flags.C = true;
    regs.flags.H = false;
    regs.flags.N = false;

    return 1;
}

uint8_t CPU::opCpl()
{
    // CPL
    // flips all bits of reg A and sets H and N

    regs.A = ~regs.A;

    regs.flags.H = true;
    regs.flags.N = true;

    return 1;
}

uint8_t CPU::opDaa()
{
    // DAA stands for "Decimal Adjust Accumulator", it's used to turn the accumulator A
    // into a valid BCD representation of its value.

    // instead of working with BCD values directly, additions and subtractions can be performed 
    // in binary as usual and the result can be converted to BCD afterward, for example:
    // 1001 + 1000  = 10001
    // 9    + 8     = 17
    // to convert 17 in binary to 1 and 7 (10001 -> 0001 0111) in BCD one can add 6.
    // since we are working with base-16 values, adding 6 is another way of saying that we are 
    // doing a mod-10 operation on the digit.

    // see https://en.wikipedia.org/wiki/Binary-coded_decimal "Operations with BCD" for 
    // a more detailed explanation.


    // The game boy programming manual is not clear on what happens if the DAA instruction
    // is executed after another instruction that is not ADD, ADC, SUB, SBC, INC or DEC so
    // in the following the example values are assumed to be the result of valid BCD operations
    // in the other cases we don't really know

    // this table is from the gameboy programming manual v1.1 (page 122)

    // For addition (when N == false)
    //  C   H   hi  lo      val     C (after)
    //  0   0   0-9 0-9     00      0
    //          0-8 A-F     06      0
    //          A-F 0-9     60      1
    //          9-F A-F     66      1
    //  0   1   0-9 0-3     06      0
    //          A-F 0-3     66      1
    //  1   0   0-2 0-9     60      1
    //          0-2 A-F     66      1
    //  1   1   0-3 0-3     66      1
    
    // For subtraction (when N == true)
    //  C   H   hi  lo      val     C (after)
    //  0   0   0-9 0-9     00      0
    //  0   1   0-8 6-F     FA      0
    //  1   0   7-F 0-9     A0      1
    //  1   1   6-F 6-F     9A      1

    // after DAA, C is updated according to the table, Z is updated as usual, H is always 0 and N is unchanged

    uint8_t valToAdd = 0;
    bool nextC = false;

    if (!regs.flags.N) {
        // previous instruction was an addition
        if (regs.flags.C || regs.A > 0x99) {
            valToAdd = 0x60;
            nextC = true;
        }
        if (regs.flags.H || lnib(regs.A) > 0x09) {
            valToAdd += 0x06;
        }
    }
    else {
        // previous instruction was a subtraction
        if (regs.flags.C) {
            valToAdd = regs.flags.H ? 0x9A : 0xA0;
            nextC = true;
        }
        else {
            valToAdd = regs.flags.H ? 0xFA : 0x00;
        }
    }

    uint16_t res = regs.A + valToAdd;

    regs.flags.Z = (uint8_t)res == 0;
    regs.flags.C = nextC;
    regs.flags.H = false;

    regs.A = (uint8_t)res;

    return 1;
}

uint8_t CPU::opAddReg16(uint16_t rhs)
{
    // ADD HL,reg16
    // add the content of reg16 (could be BC, DE, HL or SP) to HL and update the flags

    uint32_t res = regs.HL() + rhs;

    // the Z flag is not touched by this
    regs.flags.C = checkCarry16(regs.HL(), rhs);
    regs.flags.H = checkHalfCarry16(regs.HL(), rhs);
    regs.flags.N = false;

    regs.setHL((uint16_t)res);

    return 2;
}

uint8_t CPU::opAddSpImm()
{
    // ADD SP,e8
    // add the *signed* immediate value to the stack pointer and update the flags

    int16_t val = (int8_t)mBus.read8(regs.PC++);

    uint16_t res = regs.SP + val;

    // Z and N are always false, C and H are evaluated as their 8-bit version
    regs.flags.Z = false;
    regs.flags.C = checkCarry(regs.SP, val);
    regs.flags.H = checkHalfCarry((uint8_t)regs.SP, (uint8_t)val);
    regs.flags.N = false;

    regs.SP = res;

    return 4;
}

uint8_t CPU::opIncReg16(uint8_t& msb, uint8_t& lsb)
{
    // INC reg16
    // 2 cycles
    
    uint16_t val = (lsb | (msb << 8)) + 1;

    lsb = val & 0xff;
    msb = val >> 8;

    return 2;
}

uint8_t CPU::opDecReg16(uint8_t& msb, uint8_t& lsb)
{
    // DEC reg16
    // 2 cycles

    uint16_t val = (lsb | (msb << 8)) - 1;

    lsb = val & 0xff;
    msb = val >> 8;

    return 2;
}

uint8_t CPU::opIncSp()
{
    // INC SP
    // 2 cycles
    regs.SP++;
    return 2;
}

uint8_t CPU::opDecSp()
{
    // DEC SP
    // 2 cycles
    regs.SP--;
    return 2;
}

uint8_t CPU::opRlca()
{
    // RLCA 
    // Rotate A left, bit 7 of A goes into the carry flag as well 
    // as coming back into bit 0 of A
    // the other flags are all reset
    // 1 cycle

    bool bit7 = regs.A & 0x80;

    regs.A = (regs.A << 1) | (uint8_t)bit7;
    
    regs.flags.C = bit7;
    regs.flags.Z = false;
    regs.flags.H = false;
    regs.flags.N = false;

    return 1;
}

uint8_t CPU::opRla()
{
    // RLA
    // Rotate A left, bit 7 of A goes into the carry flag and the previous
    // value of C comes back into bit 0 of A
    // the other flags are all reset

    bool bit7 = regs.A & 0x80;

    regs.A = (regs.A << 1) | (uint8_t)regs.flags.C;

    regs.flags.C = bit7;
    regs.flags.Z = false;
    regs.flags.H = false;
    regs.flags.N = false;

    return 1;
}

uint8_t CPU::opRrca()
{
    // RRCA
    // Rotate A right, bit 0 of A goes into the C flag as well 
    // as coming back into A from the left
    // the other flags are all reset

    bool bit0 = regs.A & 0x01;

    regs.A = (regs.A >> 1) | (bit0 << 7);

    regs.flags.C = bit0;
    regs.flags.Z = false;
    regs.flags.H = false;
    regs.flags.N = false;

    return 1;
}

uint8_t CPU::opRra()
{
    // RRA
    // Rotate A right, bit 0 of A goes into the C flag and the previous
    // value of C comes back into bit 7 of A
    // the other flags are all reset

    bool bit0 = regs.A & 0x01;

    regs.A = (regs.A >> 1) | (regs.flags.C << 7);

    regs.flags.C = bit0;
    regs.flags.Z = false;
    regs.flags.H = false;
    regs.flags.N = false;

    return 1;
}

uint8_t CPU::opCbRlcReg(uint8_t& reg)
{
    // RLC reg
    // Rotate a register to the left, the content of bit 7 goes into flag C 
    // and into bit 0, flag Z is updated depending on the value of the register
    // N and H are reset
    // 2 cycles

    bool bit7 = reg & 0x80;

    reg = (reg << 1) | (uint8_t)bit7;

    regs.flags.C = bit7;
    regs.flags.Z = reg == 0;
    regs.flags.H = false;
    regs.flags.N = false;

    return 2;
}

uint8_t CPU::opCbRlcInd()
{
    // RLC [HL]
    // Rotate a byte in memory to the left, the content of bit 7 goes into flag C 
    // and into bit 0, flag Z is updated depending on the value of the byte
    // N and H are reset
    // 4 cycles

    auto val = mBus.read8(regs.HL());
    opCbRlcReg(val);
    mBus.write8(regs.HL(), val);

    return 4;
}

uint8_t CPU::opCbRlReg(uint8_t& reg)
{
    // RL reg
    // Rotate a register left, bit 7 of the register goes into the carry flag and the previous
    // value of C comes back into bit 0 of the register
    // flag Z is updated depending on the value of the register, N and H are reset
    // 2 cycles

    bool bit7 = reg & 0x80;

    reg = (reg << 1) | (uint8_t)regs.flags.C;

    regs.flags.C = bit7;
    regs.flags.Z = reg == 0;
    regs.flags.H = false;
    regs.flags.N = false;

    return 2;
}

uint8_t CPU::opCbRlInd()
{
    // RL [HL]
    // Rotate a byte in memory left, bit 7 of the byte goes into the carry flag and the previous
    // value of C comes back into bit 0 of the byte
    // flag Z is updated depending on the value of the byte, N and H are reset
    // 4 cycles

    auto val = mBus.read8(regs.HL());
    opCbRlReg(val);
    mBus.write8(regs.HL(), val);

    return 4;
}

uint8_t CPU::opCbRrcReg(uint8_t& reg)
{
    // RRC reg
    // Rotate a register right, bit 0 of A goes into the C flag as well 
    // as coming back into the register in bit 7
    // the Z flag depends on the value of the register, H and N are reset
    // 2 cycles

    bool bit0 = reg & 0x01;

    reg = (reg >> 1) | (bit0 << 7);

    regs.flags.C = bit0;
    regs.flags.Z = reg == 0;
    regs.flags.H = false;
    regs.flags.N = false;

    return 2;
}

uint8_t CPU::opCbRrcInd()
{
    // RRC [HL]
    // Rotate a byte in memory right, bit 0 of A goes into the C flag as well 
    // as coming back into the byte in bit 7
    // the Z flag depends on the value of the byte, H and N are reset
    // 4 cycles

    auto val = mBus.read8(regs.HL());
    opCbRrcReg(val);
    mBus.write8(regs.HL(), val);

    return 4;
}

uint8_t CPU::opCbRrReg(uint8_t& reg)
{
    // RR reg
    // Rotate a register right, bit 0 of the register goes into the C flag and the previous
    // value of C comes back into bit 7 of the register
    // the Z flag depends on the value of the byte, H and N are reset

    bool bit0 = reg & 0x01;

    reg = (reg >> 1) | (regs.flags.C << 7);

    regs.flags.C = bit0;
    regs.flags.Z = reg == 0;
    regs.flags.H = false;
    regs.flags.N = false;

    return 2;
}

uint8_t CPU::opCbRrInd()
{
    // RR [HL]
    // Rotate a register right, bit 0 of the register goes into the C flag and the previous
    // value of C comes back into bit 7 of the register
    // the Z flag depends on the value of the byte, H and N are reset
    // 4 cycles

    auto val = mBus.read8(regs.HL());
    opCbRrReg(val);
    mBus.write8(regs.HL(), val);

    return 4;
}

uint8_t CPU::opCbSlaReg(uint8_t& reg)
{
    // SLA reg
    // shifts the register left, bit 7 of the register goes into the C flag and
    // 0 enters in bit 0
    // the Z flag depends on the value of the byte, H and N are reset
    // 2 cycles

    bool bit7 = reg & 0x80;

    reg <<= 1;

    regs.flags.C = bit7;
    regs.flags.Z = reg == 0;
    regs.flags.H = false;
    regs.flags.N = false;

    return 2;
}

uint8_t CPU::opCbSlaInd()
{
    // SLA [HL]
    // shifts the byte in memory left, bit 7 of the byte goes into the C flag and
    // 0 enters in bit 0
    // the Z flag depends on the value of the byte, H and N are reset
    // 4 cycles

    auto val = mBus.read8(regs.HL());
    opCbSlaReg(val);
    mBus.write8(regs.HL(), val);

    return 4;
}

uint8_t CPU::opCbSraReg(uint8_t& reg)
{
    // SRA reg
    // shifts the register right, bit 0 of the register goes into the C flag,
    // bit 7 is shifted to the right but its value doesn't change
    // the Z flag depends on the value of the byte, H and N are reset
    // 2 cycles

    bool bit0 = reg & 0x01;
    bool bit7 = reg & 0x80;

    reg = (reg >> 1) | (bit7 << 7);

    regs.flags.C = bit0;
    regs.flags.Z = reg == 0;
    regs.flags.H = false;
    regs.flags.N = false;

    return 2;
}

uint8_t CPU::opCbSraInd()
{
    // SRA [HL]
    // shifts the byte in memory right, bit 0 of the byte goes into the C flag,
    // bit 7 is shifted to the right but its value doesn't change
    // the Z flag depends on the value of the byte, H and N are reset
    // 4 cycles

    auto val = mBus.read8(regs.HL());
    opCbSraReg(val);
    mBus.write8(regs.HL(), val);

    return 4;
}

uint8_t CPU::opCbSrlReg(uint8_t& reg)
{
    // SRL reg
    // shifts the register right, bit 0 of the register goes into the C flag,
    // 0 enters in bit 7 of the register
    // the Z flag depends on the value of the byte, H and N are reset
    // 2 cycles
    
    bool bit0 = reg & 0x01;
    
    reg >>= 1;

    regs.flags.C = bit0;
    regs.flags.Z = reg == 0;
    regs.flags.H = false;
    regs.flags.N = false;

    return 2;
}

uint8_t CPU::opCbSrlInd()
{
    // SRL [HL]
    // shifts the byte in memory right, bit 0 of the byte goes into the C flag,
    // 0 enters in bit 7 of the byte
    // the Z flag depends on the value of the byte, H and N are reset
    // 4 cycles

    auto val = mBus.read8(regs.HL());
    opCbSrlReg(val);
    mBus.write8(regs.HL(), val);

    return 4;
}

uint8_t CPU::opCbSwapReg(uint8_t& reg)
{
    // SWAP reg
    // swaps the lower 4 bits of the register with the upper 4 bits
    // the Z flag depends on the value of the byte, C, H and N are reset
    // 2 cycles

    reg = (reg << 4) | (reg >> 4);

    regs.flags.C = false;
    regs.flags.Z = reg == 0;
    regs.flags.H = false;
    regs.flags.N = false;

    return 2;
}

uint8_t CPU::opCbSwapInd()
{
    // SWAP [HL]
    // swaps the lower 4 bits of the byte in memory with the upper 4 bits
    // the Z flag depends on the value of the byte, C, H and N are reset
    // 4 cycles

    auto val = mBus.read8(regs.HL());
    opCbSwapReg(val);
    mBus.write8(regs.HL(), val);

    return 4;
}

uint8_t CPU::opCbBitReg(uint8_t b, uint8_t& reg)
{
    // BIT 2,A
    // copies the complement of the specified bit of the register into Z
    // e.g.: BIT 2,A with A = 0x22, bit 2 == 0 --> Z = 1
    // H is always 1, N is always 0, C is unchanged
    // 2 cycles
    assert(b < 8);

    bool bit = reg & (1 << b);

    regs.flags.Z = !bit;
    regs.flags.H = true;
    regs.flags.N = false;

    return 2;
}

uint8_t CPU::opCbBitInd(uint8_t b)
{
    // BIT 3,[HL]
    // copies the complement of the specified bit of the of the byte in memory into Z
    // H is always 1, N is always 0, C is unchanged
    // 3 cycles

    auto val = mBus.read8(regs.HL());
    opCbBitReg(b, val);

    return 3;
}

uint8_t CPU::opCbSetReg(uint8_t b, uint8_t& reg)
{
    // SET 2,reg
    // sets to 1 the specified bit in register reg
    // flags are unchanged
    // 2 cycles
    assert(b < 8);

    reg |= (1 << b);

    return 2;
}

uint8_t CPU::opCbSetInd(uint8_t b)
{
    // SET 3,[HL]
    // sets to 1 the specified bit in the byte in memory
    // flags are unchanged
    // 4 cycles
    assert(b < 8);

    auto val = mBus.read8(regs.HL());
    val |= (1 << b);
    mBus.write8(regs.HL(), val);

    return 4;
}

uint8_t CPU::opCbResReg(uint8_t b, uint8_t& reg)
{
    // RES 2,reg
    // resets to 0 the specified bit in register reg
    // flags are unchanged
    // 2 cycles 
    assert(b < 8);

    reg &= ~(1 << b);

    return 2;
}

uint8_t CPU::opCbResInd(uint8_t b)
{
    // RES 3,[HL]
    // resets to 0 the specified bit in the byte in memory
    // flags are unchanged
    // 4 cycles
    assert(b < 8);

    auto val = mBus.read8(regs.HL());
    val &= ~(1 << b);
    mBus.write8(regs.HL(), val);

    return 4;
}



uint8_t CPU::opJpImm()
{
    // JP a16
    // immediate unconditional jump, jump to address specified in the instruction 
    // len: 3 bytes
    // cycles: 4
    uint16_t addr = mBus.read16(regs.PC);
    
    // the PC is incremented by two to read the address but then it's immediately 
    // replaced by the new address so we can sksip this
    // regs.PC += 2;

    regs.PC = addr;
    return 4;
}

uint8_t CPU::opJpHL()
{
    // JP HL
    // unconditional jump, jump to address specified in HL (NOT IN mem[HL]!!)
    regs.PC = regs.HL();
    return 1;
}

uint8_t CPU::opJpCond(bool cond)
{
    // JP Z,a16
    // immediate conditional jump, jump to address specified in the instruction if condition is true (aka flag is set)
    // len: 3 bytes
    // cycles: 3 if branch not taken, 4 if taken

    if (cond) {
        return opJpImm();
    }
    else {
        regs.PC += 2; // must read the absolute address anyway
        return 3;
    }
}

uint8_t CPU::opJrImm()
{
    // JR e8
    // immediate relative jump, modifies the value of PC using the immediate *signed* value 
    // 3 cycles 

    int16_t val = (int8_t)mBus.read8(regs.PC++);
    regs.PC += val;

    return 3;
}

uint8_t CPU::opJrCond(bool cond)
{
    // JR C/Z/NC/NZ,e8
    // conditional relative jump, modifies the value of PC using the immediate *signed* value 
    // if the condition is verified
    // 2 cycles if branch not taken, 3 cycles if taken

    if (cond) {
        return opJrImm();
    }
    else {
        regs.PC++; // must read the relative address anyway
        return 2;
    }
}

uint8_t CPU::opCallImm()
{
    // CALL a16
    // CALL pushes the current PC to the stack then sets the 
    // PC to the address read from the instruction
    // 6 cycles

    // read the new PC address
    uint16_t newPC = mBus.read16(regs.PC);
    regs.PC += 2;

    // push the old PC to the stack
    regs.SP -= 2;
    mBus.write16(regs.SP, regs.PC);

    // also push the old PC to the nesting stack
    mCallNesting.push(regs.PC);

    // update the current PC
    regs.PC = newPC;

    return 6;
}

uint8_t CPU::opCallCond(bool cond)
{
    // CALL cond,a16
    // same as CALL but branching happens only if cond is met
    // CALL pushes the current PC to the stack then sets the 
    // PC to the address read from the instruction
    // 3 cycles if branch not taken, 6 cycles if branch taken

    if (cond) {
        return opCallImm();
    }
    else {
        regs.PC += 2; // must read the absolute address anyway
        return 3;
    }
}

uint8_t CPU::opRst(uint8_t offset)
{
    // RST HH
    // call to an address in zero-page depending on an offset
    // acceptabel values for the offset are 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30 and 0x38
    // 4 cycles
    assert(offset % 8 == 0);
    assert(offset <= 0x38);
    
    // read the new PC address
    uint16_t newPC = offset;
    
    // push the old PC to the stack
    regs.SP -= 2;
    mBus.write16(regs.SP, regs.PC);

    // update the current PC
    regs.PC = newPC;

    return 4;
}

uint8_t CPU::opRet()
{
    // RET
    // RET pops the old PC value from the stack and copies it into PC
    // 4 cycles

    auto newPC = mBus.read16(regs.SP);
    regs.SP += 2;

    regs.PC = newPC;

    // if the PC that is currently being restored is also at the top of the irq nesting
    // stack we know that we have completed the current interrupt routine
    if (!mIrqNesting.empty() && mIrqNesting.top() == newPC)
        mIrqNesting.pop();

    // check the same things also for the call nesting stack
    if (!mCallNesting.empty() && mCallNesting.top() == newPC)
        mCallNesting.pop();

    return 4;
}

uint8_t CPU::opRetCond(bool cond)
{
    // RET C,Z,NC,NZ
    // like RET but only if the condition is met
    // 2 cycles if branch not taken, 5 cycles if branch taken

    if (cond) {
        opRet();
        return 5;
    }
    else {
        return 2;
    }
}

uint8_t CPU::opReti()
{
    // RETI
    // this is supposed to be used to return from an interrupt routine,
    // just like RET but also sets ime to 1 to re-enable interrupts
    // 4 cycles
    
    opRet();
    irqs.ime = true;

    return 4;
}

uint8_t CPU::opEi()
{
    // EI
    // enable interrupts globally AFTER the next instruction has been executed, here we only have to
    // schedule the ime flag to be set
    // 1 cycle

    mImeScheduled = true;
    return 1;
}

uint8_t CPU::opDi()
{
    // DI
    // disable interrupts globally by turning the ime flag off, also, if the ime flag
    // wash scheduled to be turned on, it won't happen
    // 1 cycle

    irqs.ime = false;
    mImeScheduled = false;
    return 1;
}

uint8_t CPU::opHalt()
{
    // HALT
    // enter halt mode, instructions are not executed anymore, only an interrupt will 
    // put the cpu back to it's regular state

    mIsHalted = true;

    // A hardware bug is triggered if the HALT instruction is executed when the IME flag
    // is NOT set, see: https://gbdev.io/pandocs/halt.html
    // by setting this flag the step() function will check if the bug must be triggered
    // on the next call
    mCheckForHaltBug = true;

    return 1;
}

uint8_t CPU::opStop()
{
    // STOP
    // enter stop mode, basically turns everything off, in this state nothing is executed and
    // the only way to go back to normal is to reset the system or to receive a low signal
    // on one of the joypad lines (not an interrupt, only the signal going low is enough!)
    // see the gameboy developer manual, page 23
    
    // technically STOP is 2-bytes long, even if the second byte is ignored
    regs.PC++;

    mIsStopped = true;

    return 1;
}

uint8_t CPU::opCallIrq(Irqs::Type type)
{
    // call irq routine
    // this is the same as a regular CALL, the current PC is pushed to the stack 
    // and it's replaced with the corresponding irq handler address (very similar to the RST instruction)
    // 
    // Anyway, this takes 5 cycles instead of 4, in the actual hardware the next opcode is fetched, only then
    // the interrupts are checked so, before pushing the PC to the stack, it must be decremented,
    // hence the additional cycle compared to RST
    // 
    // Here we don't actually fetch the next opcode before checking for interrutps, so we only have 
    // to account for the difference in cycles

    // push the old PC to the stack
    regs.SP -= 2;
    mBus.write16(regs.SP, regs.PC);

    // save the old PC to the nesting stack for debug purposes
    mIrqNesting.push(regs.PC);

    // update the current PC
    regs.PC = Irqs::addr(type);

    return 5;
}




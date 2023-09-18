#include "gb/Cpu.h"
#include "gb/Opcodes.h"
#include "doctest/doctest.h"


TEST_CASE("CPU test NOP") 
{
    TestBus bus;
    CPU cpu(bus);

    auto oldRegs = cpu.regs;

    bus.write8(Registers::PCinitialValue, op::NOP);
    cpu.step();
    CHECK(cpu.regs.equalSkipPC(oldRegs));
    CHECK(cpu.elapsedCycles() == 1);
}


TEST_CASE("CPU test LD immediate") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc + 1, val);

    SUBCASE("Test LD A,n8") {
        bus.write8(pc, op::LD_A_n8);
        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD B,n8") {
        bus.write8(pc, op::LD_B_n8);
        cpu.step();
        CHECK(cpu.regs.B == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD C,n8") {
        bus.write8(pc, op::LD_C_n8);
        cpu.step();
        CHECK(cpu.regs.C == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD D,n8") {
        bus.write8(pc, op::LD_D_n8);
        cpu.step();
        CHECK(cpu.regs.D == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD E,n8") {
        bus.write8(pc, op::LD_E_n8);
        cpu.step();
        CHECK(cpu.regs.E == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD H,n8") {
        bus.write8(pc, op::LD_H_n8);
        cpu.step();
        CHECK(cpu.regs.H == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD L,n8") {
        bus.write8(pc, op::LD_L_n8);
        cpu.step();
        CHECK(cpu.regs.L == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
}



TEST_CASE("CPU test LD A,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t pc = Registers::PCinitialValue;

    SUBCASE("Test LD A,A") {
        bus.write8(pc, op::LD_A_A);
        cpu.regs.A = val;
        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD A,B") {
        bus.write8(pc, op::LD_A_B);
        cpu.regs.B = val;
        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD A,C") {
        bus.write8(pc, op::LD_A_C);
        cpu.regs.C = val;
        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD A,D") {
        bus.write8(pc, op::LD_A_D);
        cpu.regs.D = val;
        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD A,E") {
        bus.write8(pc, op::LD_A_E);
        cpu.regs.E = val;
        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD A,H") {
        bus.write8(pc, op::LD_A_H);
        cpu.regs.H = val;
        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD A,L") {
        bus.write8(pc, op::LD_A_L);
        cpu.regs.L = val;
        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
}



TEST_CASE("CPU test LD B,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t pc = Registers::PCinitialValue;

    SUBCASE("Test LD B,A") {
        bus.write8(pc, op::LD_B_A);
        cpu.regs.A = val;
        cpu.step();
        CHECK(cpu.regs.B == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD B,B") {
        bus.write8(pc, op::LD_B_B);
        cpu.regs.B = val;
        cpu.step();
        CHECK(cpu.regs.B == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD B,C") {
        bus.write8(pc, op::LD_B_C);
        cpu.regs.C = val;
        cpu.step();
        CHECK(cpu.regs.B == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD B,D") {
        bus.write8(pc, op::LD_B_D);
        cpu.regs.D = val;
        cpu.step();
        CHECK(cpu.regs.B == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD B,E") {
        bus.write8(pc, op::LD_B_E);
        cpu.regs.E = val;
        cpu.step();
        CHECK(cpu.regs.B == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD B,H") {
        bus.write8(pc, op::LD_B_H);
        cpu.regs.H = val;
        cpu.step();
        CHECK(cpu.regs.B == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD B,L") {
        bus.write8(pc, op::LD_B_L);
        cpu.regs.L = val;
        cpu.step();
        CHECK(cpu.regs.B == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
}


TEST_CASE("CPU test LD C,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t pc = Registers::PCinitialValue;

    SUBCASE("Test LD C,A") {
        bus.write8(pc, op::LD_C_A);
        cpu.regs.A = val;
        cpu.step();
        CHECK(cpu.regs.C == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD C,B") {
        bus.write8(pc, op::LD_C_B);
        cpu.regs.B = val;
        cpu.step();
        CHECK(cpu.regs.C == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD C,C") {
        bus.write8(pc, op::LD_C_C);
        cpu.regs.C = val;
        cpu.step();
        CHECK(cpu.regs.C == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD C,D") {
        bus.write8(pc, op::LD_C_D);
        cpu.regs.D = val;
        cpu.step();
        CHECK(cpu.regs.C == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD C,E") {
        bus.write8(pc, op::LD_C_E);
        cpu.regs.E = val;
        cpu.step();
        CHECK(cpu.regs.C == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD C,H") {
        bus.write8(pc, op::LD_C_H);
        cpu.regs.H = val;
        cpu.step();
        CHECK(cpu.regs.C == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD C,L") {
        bus.write8(pc, op::LD_C_L);
        cpu.regs.L = val;
        cpu.step();
        CHECK(cpu.regs.C == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
}


TEST_CASE("CPU test LD D,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t pc = Registers::PCinitialValue;

    SUBCASE("Test LD D,A") {
        bus.write8(pc, op::LD_D_A);
        cpu.regs.A = val;
        cpu.step();
        CHECK(cpu.regs.D == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD D,B") {
        bus.write8(pc, op::LD_D_B);
        cpu.regs.B = val;
        cpu.step();
        CHECK(cpu.regs.D == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD D,C") {
        bus.write8(pc, op::LD_D_C);
        cpu.regs.C = val;
        cpu.step();
        CHECK(cpu.regs.D == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD D,D") {
        bus.write8(pc, op::LD_D_D);
        cpu.regs.D = val;
        cpu.step();
        CHECK(cpu.regs.D == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD D,E") {
        bus.write8(pc, op::LD_D_E);
        cpu.regs.E = val;
        cpu.step();
        CHECK(cpu.regs.D == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD D,H") {
        bus.write8(pc, op::LD_D_H);
        cpu.regs.H = val;
        cpu.step();
        CHECK(cpu.regs.D == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD D,L") {
        bus.write8(pc, op::LD_D_L);
        cpu.regs.L = val;
        cpu.step();
        CHECK(cpu.regs.D == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
}


TEST_CASE("CPU test LD E,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t pc = Registers::PCinitialValue;

    SUBCASE("Test LD E,A") {
        bus.write8(pc, op::LD_E_A);
        cpu.regs.A = val;
        cpu.step();
        CHECK(cpu.regs.E == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD E,B") {
        bus.write8(pc, op::LD_E_B);
        cpu.regs.B = val;
        cpu.step();
        CHECK(cpu.regs.E == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD E,C") {
        bus.write8(pc, op::LD_E_C);
        cpu.regs.C = val;
        cpu.step();
        CHECK(cpu.regs.E == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD E,D") {
        bus.write8(pc, op::LD_E_D);
        cpu.regs.D = val;
        cpu.step();
        CHECK(cpu.regs.E == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD E,E") {
        bus.write8(pc, op::LD_E_E);
        cpu.regs.E = val;
        cpu.step();
        CHECK(cpu.regs.E == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD E,H") {
        bus.write8(pc, op::LD_E_H);
        cpu.regs.H = val;
        cpu.step();
        CHECK(cpu.regs.E == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD E,L") {
        bus.write8(pc, op::LD_E_L);
        cpu.regs.L = val;
        cpu.step();
        CHECK(cpu.regs.E == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
}


TEST_CASE("CPU test LD H,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t pc = Registers::PCinitialValue;

    SUBCASE("Test LD H,A") {
        bus.write8(pc, op::LD_H_A);
        cpu.regs.A = val;
        cpu.step();
        CHECK(cpu.regs.H == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD H,B") {
        bus.write8(pc, op::LD_H_B);
        cpu.regs.B = val;
        cpu.step();
        CHECK(cpu.regs.H == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD H,C") {
        bus.write8(pc, op::LD_H_C);
        cpu.regs.C = val;
        cpu.step();
        CHECK(cpu.regs.H == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD H,D") {
        bus.write8(pc, op::LD_H_D);
        cpu.regs.D = val;
        cpu.step();
        CHECK(cpu.regs.H == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD H,E") {
        bus.write8(pc, op::LD_H_E);
        cpu.regs.E = val;
        cpu.step();
        CHECK(cpu.regs.H == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD H,H") {
        bus.write8(pc, op::LD_H_H);
        cpu.regs.H = val;
        cpu.step();
        CHECK(cpu.regs.H == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD H,L") {
        bus.write8(pc, op::LD_H_L);
        cpu.regs.L = val;
        cpu.step();
        CHECK(cpu.regs.H == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
}


TEST_CASE("CPU test LD L,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t pc = Registers::PCinitialValue;

    SUBCASE("Test LD L,A") {
        bus.write8(pc, op::LD_L_A);
        cpu.regs.A = val;
        cpu.step();
        CHECK(cpu.regs.L == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD L,B") {
        bus.write8(pc, op::LD_L_B);
        cpu.regs.B = val;
        cpu.step();
        CHECK(cpu.regs.L == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD L,C") {
        bus.write8(pc, op::LD_L_C);
        cpu.regs.C = val;
        cpu.step();
        CHECK(cpu.regs.L == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD L,D") {
        bus.write8(pc, op::LD_L_D);
        cpu.regs.D = val;
        cpu.step();
        CHECK(cpu.regs.L == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD L,E") {
        bus.write8(pc, op::LD_L_E);
        cpu.regs.E = val;
        cpu.step();
        CHECK(cpu.regs.L == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD L,H") {
        bus.write8(pc, op::LD_L_H);
        cpu.regs.H = val;
        cpu.step();
        CHECK(cpu.regs.L == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test LD L,L") {
        bus.write8(pc, op::LD_L_L);
        cpu.regs.L = val;
        cpu.step();
        CHECK(cpu.regs.L == val);
        CHECK(cpu.elapsedCycles() == 1);
    }
}



TEST_CASE("CPU test LD reg,[HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    cpu.regs.setHL(addr);

    bus.write8(addr, val);

    SUBCASE("Test LD A,[HL]") {
        bus.write8(pc, op::LD_A_inHL);
        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD B,[HL]") {
        bus.write8(pc, op::LD_B_inHL);
        cpu.step();
        CHECK(cpu.regs.B == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD C,[HL]") {
        bus.write8(pc, op::LD_C_inHL);
        cpu.step();
        CHECK(cpu.regs.C == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD D,[HL]") {
        bus.write8(pc, op::LD_D_inHL);
        cpu.step();
        CHECK(cpu.regs.D == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD E,[HL]") {
        bus.write8(pc, op::LD_E_inHL);
        cpu.step();
        CHECK(cpu.regs.E == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD H,[HL]") {
        bus.write8(pc, op::LD_H_inHL);
        cpu.step();
        CHECK(cpu.regs.H == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD L,[HL]") {
        bus.write8(pc, op::LD_L_inHL);
        cpu.step();
        CHECK(cpu.regs.L == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
}



TEST_CASE("CPU test LD [HL],reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    cpu.regs.setHL(addr);

    SUBCASE("Test LD [HL],A") {
        cpu.regs.A = val;
        bus.write8(pc, op::LD_inHl_A);
        cpu.step();
        CHECK(bus.read8(addr) == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD [HL],B") {
        cpu.regs.B = val;
        bus.write8(pc, op::LD_inHl_B);
        cpu.step();
        CHECK(bus.read8(addr) == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD [HL],C") {
        cpu.regs.C = val;
        bus.write8(pc, op::LD_inHl_C);
        cpu.step();
        CHECK(bus.read8(addr) == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD [HL],D") {
        cpu.regs.D = val;
        bus.write8(pc, op::LD_inHl_D);
        cpu.step();
        CHECK(bus.read8(addr) == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD [HL],E") {
        cpu.regs.E = val;
        bus.write8(pc, op::LD_inHl_E);
        cpu.step();
        CHECK(bus.read8(addr) == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD [HL],H") {
        bus.write8(pc, op::LD_inHl_H);
        cpu.step();
        CHECK(bus.read8(addr) == addr >> 8);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD [HL],L") {
        bus.write8(pc, op::LD_inHl_L);
        cpu.step();
        CHECK(bus.read8(addr) == (addr & 0xff));
        CHECK(cpu.elapsedCycles() == 2);
    }
}


TEST_CASE("CPU test LD [HL],n8") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    cpu.regs.setHL(addr);

    bus.write8(pc, op::LD_inHL_n8);
    bus.write8(pc + 1, val);

    cpu.step();
    CHECK(bus.read8(addr) == val);
    CHECK(cpu.elapsedCycles() == 3);
}


TEST_CASE("CPU test LD A,[BC]/[DE]") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(addr, val);

    SUBCASE("test LD A,[BC]") {
        cpu.regs.setBC(addr);
        bus.write8(pc, op::LD_A_inBC);

        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("test LD A,[DE]") {
        cpu.regs.setDE(addr);
        bus.write8(pc, op::LD_A_inDE);

        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
}


TEST_CASE("CPU test LD [BC]/[DE],A") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    cpu.regs.A = val;

    SUBCASE("test LD [BC],A") {
        cpu.regs.setBC(addr);
        bus.write8(pc, op::LD_inBC_A);

        cpu.step();
        CHECK(bus.read8(addr) == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("test LD [DE],A") {
        cpu.regs.setDE(addr);
        bus.write8(pc, op::LD_inDE_A);

        cpu.step();
        CHECK(bus.read8(addr) == val);
        CHECK(cpu.elapsedCycles() == 2);
    }
}


TEST_CASE("CPU test LD A,[a16]") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::LD_A_ina16);
    bus.write8(pc + 1, addr & 0xff);
    bus.write8(pc + 2, addr >> 8);
    bus.write8(addr, val);
    
    cpu.step();
    CHECK(cpu.regs.A == val);
    CHECK(cpu.elapsedCycles() == 4);
}


TEST_CASE("CPU test LD [a16],A") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::LD_ina16_A);
    bus.write8(pc + 1, addr & 0xff);
    bus.write8(pc + 2, addr >> 8);
    cpu.regs.A = val;
    
    cpu.step();
    CHECK(bus.read8(addr) == val);
    CHECK(cpu.elapsedCycles() == 4);
}


TEST_CASE("CPU test LDH A,[C]") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0xff25;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::LDH_A_inC);
    bus.write8(addr, val);
    cpu.regs.C = 0x25;
    
    cpu.step();
    CHECK(cpu.regs.A == val);
    CHECK(cpu.elapsedCycles() == 2);
}


TEST_CASE("CPU test LDH [C],A") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0xff25;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::LDH_inC_A);
    cpu.regs.A = val;
    cpu.regs.C = 0x25;
    
    cpu.step();
    CHECK(bus.read8(addr) == val);
    CHECK(cpu.elapsedCycles() == 2);
}


TEST_CASE("CPU test LDH A,[a8]") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0xff25;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::LDH_A_ina8);
    bus.write8(pc + 1, 0x25);
    bus.write8(addr, val);
    
    cpu.step();
    CHECK(cpu.regs.A == val);
    CHECK(cpu.elapsedCycles() == 3);
}


TEST_CASE("CPU test LDH [a8],A") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0xff25;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::LDH_ina8_A);
    bus.write8(pc + 1, 0x25);
    cpu.regs.A = val;
    
    cpu.step();
    CHECK(bus.read8(addr) == val);
    CHECK(cpu.elapsedCycles() == 3);
}


TEST_CASE("CPU test LD indirect from HL with increment and decrement") {
    TestBus bus;
    CPU cpu(bus);

    const uint8_t val = 0x32;
    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    cpu.regs.setHL(addr);


    SUBCASE("Test LD A,[HL-]") {
        bus.write8(pc, op::LD_A_inHLm);
        bus.write8(addr, val);

        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.regs.HL() == addr - 1);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD [HL-],A") {
        bus.write8(pc, op::LD_inHLm_A);
        cpu.regs.A = val;

        cpu.step();
        CHECK(bus.read8(addr) == val);
        CHECK(cpu.regs.HL() == addr - 1);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD A,[HL+]") {
        bus.write8(pc, op::LD_A_inHLp);
        bus.write8(addr, val);

        cpu.step();
        CHECK(cpu.regs.A == val);
        CHECK(cpu.regs.HL() == addr + 1);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test LD [HL+],A") {
        bus.write8(pc, op::LD_inHLp_A);
        cpu.regs.A = val;

        cpu.step();
        CHECK(bus.read8(addr) == val);
        CHECK(cpu.regs.HL() == addr + 1);
        CHECK(cpu.elapsedCycles() == 2);
    }
}



TEST_CASE("CPU test LD immediate 16-bit in register") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t val = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write16(pc + 1, val);

    SUBCASE("Test LD BC,n16") {
        bus.write8(pc, op::LD_BC_n16);
        cpu.step();
        
        CHECK(cpu.regs.BC() == val);
        CHECK(cpu.elapsedCycles() == 3);
    }
    SUBCASE("Test LD DE,n16") {
        bus.write8(pc, op::LD_DE_n16);
        cpu.step();
        
        CHECK(cpu.regs.DE() == val);
        CHECK(cpu.elapsedCycles() == 3);
    }
    SUBCASE("Test LD HL,n16") {
        bus.write8(pc, op::LD_HL_n16);
        cpu.step();
        
        CHECK(cpu.regs.HL() == val);
        CHECK(cpu.elapsedCycles() == 3);
    }
    SUBCASE("Test LD SP,n16") {
        bus.write8(pc, op::LD_SP_n16);
        cpu.step();
        
        CHECK(cpu.regs.SP == val);
        CHECK(cpu.elapsedCycles() == 3);
    }
}


TEST_CASE("CPU test LD [a16],SP") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t addr = 0x1234;
    const uint16_t val = 0x4321;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::LD_ina16_SP);
    bus.write16(pc + 1, addr);

    cpu.regs.SP = val;
    cpu.step();

    CHECK(bus.read16(addr) == val);
    CHECK(cpu.elapsedCycles() == 5);
}

TEST_CASE("CPU test LD SP,HL") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t val = 0x4321;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::LD_SP_HL);
    cpu.regs.setHL(val);

    cpu.step();

    CHECK(cpu.regs.SP == val);
    CHECK(cpu.elapsedCycles() == 2);
}

TEST_CASE("CPU test PUSH reg16") {
    TestBus bus;
    CPU cpu(bus);

    // the last nibble of val is zero because when used to set the value of 
    // the F register the least significant 4 bits are not used and are always 0
    const uint16_t addr = 0x1234;
    const uint16_t val = 0x4320;
    const uint16_t pc = Registers::PCinitialValue;

    cpu.regs.SP = addr;

    SUBCASE("Test PUSH BC") {
        bus.write8(pc, op::PUSH_BC);
        cpu.regs.setBC(val);
        cpu.step();

        CHECK(bus.read16(addr - 2) == val);
        CHECK(cpu.regs.SP == addr - 2);
        CHECK(cpu.elapsedCycles() == 4);
    }
    SUBCASE("Test PUSH DE") {
        bus.write8(pc, op::PUSH_DE);
        cpu.regs.setDE(val);
        cpu.step();

        CHECK(bus.read16(addr - 2) == val);
        CHECK(cpu.regs.SP == addr - 2);
        CHECK(cpu.elapsedCycles() == 4);
    }
    SUBCASE("Test PUSH HL") {
        bus.write8(pc, op::PUSH_HL);
        cpu.regs.setHL(val);
        cpu.step();

        CHECK(bus.read16(addr - 2) == val);
        CHECK(cpu.regs.SP == addr - 2);
        CHECK(cpu.elapsedCycles() == 4);
    }
    SUBCASE("Test PUSH AF") {
        bus.write8(pc, op::PUSH_AF);
        cpu.regs.setAF(val);
        cpu.step();

        CHECK(bus.read16(addr - 2) == val);
        CHECK(cpu.regs.SP == addr - 2);
        CHECK(cpu.elapsedCycles() == 4);
    }
}

TEST_CASE("CPU test POP reg16") {
    TestBus bus;
    CPU cpu(bus);

    // the last nibble of val is zero because when used to set the value of 
    // the F register the least significant 4 bits are not used and are always 0
    const uint16_t addr = 0x1234;
    const uint16_t val = 0x4320;
    const uint16_t pc = Registers::PCinitialValue;

    cpu.regs.SP = addr;
    bus.write16(addr, val);

    SUBCASE("Test POP BC") {
        bus.write8(pc, op::POP_BC);
        cpu.step();

        CHECK(cpu.regs.BC() == val);
        CHECK(cpu.regs.SP == addr + 2);
        CHECK(cpu.elapsedCycles() == 3);
    }
    SUBCASE("Test POP DE") {
        bus.write8(pc, op::POP_DE);
        cpu.step();

        CHECK(cpu.regs.DE() == val);
        CHECK(cpu.regs.SP == addr + 2);
        CHECK(cpu.elapsedCycles() == 3);
    }
    SUBCASE("Test POP HL") {
        bus.write8(pc, op::POP_HL);
        cpu.step();

        CHECK(cpu.regs.HL() == val);
        CHECK(cpu.regs.SP == addr + 2);
        CHECK(cpu.elapsedCycles() == 3);
    }
    SUBCASE("Test POP AF") {
        bus.write8(pc, op::POP_AF);
        cpu.step();

        CHECK(cpu.regs.AF() == val);
        CHECK(cpu.regs.SP == addr + 2);
        CHECK(cpu.elapsedCycles() == 3);

        // if the value written to AF is 0x4321 the flags will be replaced with 0x21
        // this means that 0x2 will end up in the most significant nibble of the flags
        // so the flags will be z=0, n=0, h=1, c=0
        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.N);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.C);
    }
}



TEST_CASE("CPU test ADD A,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    // we use register B for the test, all the other registers use the same code
    bus.write8(pc, op::ADD_A_B);

    SUBCASE("Test ADD A,B no flags") {
        cpu.regs.A = 1;
        cpu.regs.B = 2;
        cpu.step();
        CHECK(cpu.regs.A == 3);
        
        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.N);
        
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADD A,B zero flag") {
        cpu.regs.A = 0;
        cpu.regs.B = 0;
        cpu.step();
        CHECK(cpu.regs.A == 0);
        
        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.N);
        
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADD A,B carry and half-carry flags") {
        cpu.regs.A = 255;
        cpu.regs.B = 2;
        cpu.step();
        CHECK(cpu.regs.A == 1);
        
        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADD A,B zero, carry and half-carry flags") {
        cpu.regs.A = 255;
        cpu.regs.B = 1;
        cpu.step();
        CHECK(cpu.regs.A == 0);

        CHECK(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.N);
        
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADD A,B half-carry flag") {
        cpu.regs.A = 0x0f;
        cpu.regs.B = 1;
        cpu.step();
        CHECK(cpu.regs.A == 0x10);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}



TEST_CASE("CPU test ADD A,[HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    // here we only check if the correct value pointed by HL in memory is 
    // actually used, flags are already checked in the ADD A,reg and the code is the same

    bus.write8(pc, op::ADD_A_inHL);
    bus.write8(addr, 2);
    cpu.regs.setHL(addr);

    cpu.regs.A = 1;
    cpu.step();

    CHECK(cpu.regs.A == 3);

    CHECK_FALSE(cpu.regs.flags.Z);
    CHECK_FALSE(cpu.regs.flags.H);
    CHECK_FALSE(cpu.regs.flags.C);
    CHECK_FALSE(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 2);
}


TEST_CASE("CPU test ADD A,n8") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;
    
    // here we only check if the correct immediate value is actually used, flags are 
    // already checked in the ADD A,reg and the code is the same

    bus.write8(pc, op::ADD_A_n8);
    bus.write8(pc + 1, 2);
    cpu.regs.A = 1;
    
    cpu.step();
    
    CHECK(cpu.regs.A == 3);

    CHECK_FALSE(cpu.regs.flags.Z);
    CHECK_FALSE(cpu.regs.flags.H);
    CHECK_FALSE(cpu.regs.flags.C);
    CHECK_FALSE(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 2);
}


TEST_CASE("CPU test ADC A,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    // we use register B for the test, all the other registers use the same code
    bus.write8(pc, op::ADC_A_B);

    SUBCASE("Test ADC A,B no flags") {
        cpu.regs.A = 1;
        cpu.regs.B = 2;
        cpu.regs.flags.C = 0;
        cpu.step();
        CHECK(cpu.regs.A == 3);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADC A,B +carry, no flags") {
        cpu.regs.A = 1;
        cpu.regs.B = 2;
        cpu.regs.flags.C = 1;
        cpu.step();
        CHECK(cpu.regs.A == 4);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADC A,B zero flag") {
        cpu.regs.A = 0;
        cpu.regs.B = 0;
        cpu.regs.flags.C = 0;
        cpu.step();
        CHECK(cpu.regs.A == 0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADC A,B +carry, carry and half-carry flags") {
        cpu.regs.A = 255;
        cpu.regs.B = 1;
        cpu.regs.flags.C = 1;
        cpu.step();
        CHECK(cpu.regs.A == 1);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADC A,B +carry, zero, carry and half-carry flags") {
        cpu.regs.A = 254;
        cpu.regs.B = 1;
        cpu.regs.flags.C = 1;
        cpu.step();
        CHECK(cpu.regs.A == 0);

        CHECK(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADC A,B half-carry flag") {
        cpu.regs.A = 0x0e;
        cpu.regs.B = 1;
        cpu.regs.flags.C = 1;
        cpu.step();
        CHECK(cpu.regs.A == 0x10);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}



TEST_CASE("CPU test ADC A,[HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    // here we only check if the correct value pointed by HL in memory is 
    // actually used, flags are already checked in the ADD A,reg and the code is the same

    cpu.regs.setHL(addr);

    bus.write8(pc, op::ADC_A_inHL);
    bus.write8(addr, 2);
    cpu.regs.A = 1;
    cpu.regs.flags.C = 0;
    
    cpu.step();
    
    CHECK(cpu.regs.A == 3);

    CHECK_FALSE(cpu.regs.flags.Z);
    CHECK_FALSE(cpu.regs.flags.H);
    CHECK_FALSE(cpu.regs.flags.C);
    CHECK_FALSE(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 2);
}


TEST_CASE("CPU test ADC A,n8") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    // here we only check if the correct immediate value is actually used, flags are 
    // already checked in the ADD A,reg and the code is the same

    bus.write8(pc, op::ADC_A_n8);
    bus.write8(pc + 1, 2);
    cpu.regs.A = 1;
    cpu.regs.flags.C = 0;
    
    cpu.step();
    
    CHECK(cpu.regs.A == 3);

    CHECK_FALSE(cpu.regs.flags.Z);
    CHECK_FALSE(cpu.regs.flags.H);
    CHECK_FALSE(cpu.regs.flags.C);
    CHECK_FALSE(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 2);
}


TEST_CASE("CPU test SUB A,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::SUB_A_B);
    cpu.regs.B = 0x23;

    // To perform the subtraction SUB subtracts the other operand from A, the result is stored in A and
    // the flags are set as follows:
    // - flag N is always set because a subtraction has been performed
    // - flag Z is set only if the result is zero
    // - flag C is set only if we need to borrow from bit 7 (that is, if A < val)
    // - flag N is set only if we need to borrow from bit 3 (that is, if the lower nibble of A is lower than
    //      the lower nibble of the value)

    // we use register B, the code is the same for the other registers

    SUBCASE("Test SUB A,B no flags") {
        cpu.regs.A = 0x25;
        cpu.step();
        CHECK(cpu.regs.A == 0x02);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test SUB A,B half-carry flag") {
        cpu.regs.A = 0x31;
        cpu.step();
        CHECK(cpu.regs.A == 0x0E);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test SUB A,B carry and half-carry flag") {
        cpu.regs.A = 0x20;
        cpu.step();
        CHECK(cpu.regs.A == 0xFD);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test SUB A,B zero flag") {
        cpu.regs.A = 0x23;
        cpu.step();
        CHECK(cpu.regs.A == 0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}


TEST_CASE("CPU test SUB A,[HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    // see the SUB A,reg tests for an explanation of the expected results,
    // here we only check that the correct value pointed by HL in memory is used
    // flags are already checked in SUB A,reg and the code for the actual subtraction is the same

    bus.write8(pc, op::SUB_A_inHL);
    bus.write8(addr, 0x23);
    cpu.regs.setHL(addr);
    cpu.regs.A = 0x25;

    cpu.step();

    CHECK(cpu.regs.A == 0x02);

    CHECK_FALSE(cpu.regs.flags.Z);
    CHECK_FALSE(cpu.regs.flags.C);
    CHECK_FALSE(cpu.regs.flags.H);
    CHECK(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 2);
}


TEST_CASE("CPU test SUB A,n8") {
    TestBus bus;
    CPU cpu(bus);

    // see the SUB A,reg tests for an explanation of the expected results,
    // here we only check that the correct immediate value is used, flags are already checked 
    // in SUB A,reg and the code for the actual subtraction is the same

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::SUB_A_n8);
    bus.write8(pc + 1, 0x23);
    cpu.regs.A = 0x25;

    cpu.step();

    CHECK(cpu.regs.A == 0x02);

    CHECK_FALSE(cpu.regs.flags.Z);
    CHECK_FALSE(cpu.regs.flags.C);
    CHECK_FALSE(cpu.regs.flags.H);
    CHECK(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 2);
}


TEST_CASE("CPU test SBC A,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::SBC_A_B);
    cpu.regs.B = 0x23;


    SUBCASE("Test SBC A,B half-carry flag") {
        cpu.regs.A = 0x31;
        cpu.regs.flags.C = 1;
        cpu.step();
        CHECK(cpu.regs.A == 0x0D);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test SBC A,B no flags") {
        cpu.regs.A = 0x25;
        cpu.regs.flags.C = 1;
        cpu.step();
        CHECK(cpu.regs.A == 0x01);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test SBC A,B carry and half-carry flags") {
        cpu.regs.A = 0x20;
        cpu.regs.flags.C = 1;
        cpu.step();
        CHECK(cpu.regs.A == 0xFC);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test SBC A,B zero flag") {
        cpu.regs.A = 0x24;
        cpu.regs.flags.C = 1;
        cpu.step();
        CHECK(cpu.regs.A == 0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}


TEST_CASE("CPU test SBC A,[HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;
    
    // see the SUB A,reg tests for an explanation of the expected results,
    // here we only check that the correct value pointed by HL in memory is used
    // flags are already checked in SBC A,reg and the code for the actual subtraction is the same

    uint16_t addr = 0x1234;
    bus.write8(pc, op::SBC_A_inHL);
    bus.write8(addr, 0x23);
    cpu.regs.setHL(addr);
    cpu.regs.A = 0x31;
    cpu.regs.flags.C = 1;

    cpu.step();

    CHECK(cpu.regs.A == 0x0D);

    CHECK_FALSE(cpu.regs.flags.Z);
    CHECK_FALSE(cpu.regs.flags.C);
    CHECK(cpu.regs.flags.H);
    CHECK(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 2);
}


TEST_CASE("CPU test SBC A,n8") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::SBC_A_n8);
    bus.write8(pc + 1, 0x23);
    cpu.regs.A = 0x31;
    cpu.regs.flags.C = 1;

    cpu.step();

    CHECK(cpu.regs.A == 0x0D);

    CHECK_FALSE(cpu.regs.flags.Z);
    CHECK_FALSE(cpu.regs.flags.C);
    CHECK(cpu.regs.flags.H);
    CHECK(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 2);
}



TEST_CASE("CPU test AND A,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::AND_A_B);
    cpu.regs.B = 0x23;

    // when using the AND instruction the Z flag depend on the result of the 
    // operation while N is always 0, C is always 0 and N is always 1

    SUBCASE("Test AND A,B no flags") {
        cpu.regs.A = 0x31;
        cpu.step();
        CHECK(cpu.regs.A == 0x21);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test AND A,B zero flag") {
        cpu.regs.A = 0x0;
        cpu.step();
        CHECK(cpu.regs.A == 0x0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}

TEST_CASE("CPU test AND A,[HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    uint16_t addr = 0x1234;
    bus.write8(pc, op::AND_A_inHL);
    bus.write8(addr, 0x23);
    cpu.regs.setHL(addr);

    // when using the AND instruction the Z flag depend on the result of the 
    // operation while N is always 0, C is always 0 and N is always 1

    SUBCASE("Test AND A,[HL] no flags") {
        cpu.regs.A = 0x31;
        cpu.step();
        CHECK(cpu.regs.A == 0x21);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test AND A,[HL] zero flag") {
        cpu.regs.A = 0x0;
        cpu.step();
        CHECK(cpu.regs.A == 0x0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
}

TEST_CASE("CPU test AND A,n8") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::AND_A_n8);
    bus.write8(pc + 1, 0x23);

    // when using the AND instruction the Z flag depend on the result of the 
    // operation while N is always 0, C is always 0 and N is always 1

    SUBCASE("Test AND A,n8 no flags") {
        cpu.regs.A = 0x31;
        cpu.step();
        CHECK(cpu.regs.A == 0x21);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test AND A,n8 zero flag") {
        cpu.regs.A = 0x0;
        cpu.step();
        CHECK(cpu.regs.A == 0x0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
}


TEST_CASE("CPU test OR A,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::OR_A_B);

    // when using the OR instruction the Z flag depend on the result of the 
    // operation while all other flags are always 0

    SUBCASE("Test OR A,B no flags") {
        cpu.regs.B = 0x23;
        cpu.regs.A = 0x31;
        cpu.step();
        CHECK(cpu.regs.A == 0x33);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test OR A,B zero flag") {
        cpu.regs.B = 0x0;
        cpu.regs.A = 0x0;
        cpu.step();
        CHECK(cpu.regs.A == 0x0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}

TEST_CASE("CPU test OR A,[HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    uint16_t addr = 0x1234;
    bus.write8(pc, op::OR_A_inHL);
    cpu.regs.setHL(addr);

    // when using the OR instruction the Z flag depend on the result of the 
    // operation while all other flags are always 0

    SUBCASE("Test OR A,[HL] no flags") {
        bus.write8(addr, 0x23);
        cpu.regs.A = 0x31;
        cpu.step();
        CHECK(cpu.regs.A == 0x33);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test OR A,[HL] zero flag") {
        bus.write8(addr, 0x00);
        cpu.regs.A = 0x0;
        cpu.step();
        CHECK(cpu.regs.A == 0x0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
}

TEST_CASE("CPU test OR A,n8") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::OR_A_n8);

    // when using the OR instruction the Z flag depend on the result of the 
    // operation while all other flags are always 0

    SUBCASE("Test OR A,n8 no flags") {
        bus.write8(pc + 1, 0x23);
        cpu.regs.A = 0x31;
        cpu.step();
        CHECK(cpu.regs.A == 0x33);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test OR A,n8 zero flag") {
        bus.write8(pc + 1, 0);
        cpu.regs.A = 0;
        cpu.step();
        CHECK(cpu.regs.A == 0x0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
}



TEST_CASE("CPU test XOR A,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::XOR_A_B);

    // when using the XOR instruction the Z flag depend on the result of the 
    // operation while all other flags are always 0

    SUBCASE("Test XOR A,B no flags") {
        cpu.regs.B = 0x23;
        cpu.regs.A = 0x31;
        cpu.step();
        CHECK(cpu.regs.A == 0x12);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test XOR A,B zero flag") {
        cpu.regs.B = 0x32;
        cpu.regs.A = 0x32;
        cpu.step();
        CHECK(cpu.regs.A == 0x0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}

TEST_CASE("CPU test XOR A,[HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    uint16_t addr = 0x1234;
    bus.write8(pc, op::XOR_A_inHL);
    cpu.regs.setHL(addr);

    // when using the XOR instruction the Z flag depend on the result of the 
    // operation while all other flags are always 0

    SUBCASE("Test XOR A,[HL] no flags") {
        bus.write8(addr, 0x23);
        cpu.regs.A = 0x31;
        cpu.step();
        CHECK(cpu.regs.A == 0x12);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test XOR A,[HL] zero flag") {
        bus.write8(addr, 0x32);
        cpu.regs.A = 0x32;
        cpu.step();
        CHECK(cpu.regs.A == 0x0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
}

TEST_CASE("CPU test XOR A,n8") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::XOR_A_n8);

    // when using the XOR instruction the Z flag depend on the result of the 
    // operation while all other flags are always 0

    SUBCASE("Test XOR A,n8 no flags") {
        bus.write8(pc + 1, 0x23);
        cpu.regs.A = 0x31;
        cpu.step();
        CHECK(cpu.regs.A == 0x12);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test XOR A,n8 zero flag") {
        bus.write8(pc + 1, 0x32);
        cpu.regs.A = 0x32;
        cpu.step();
        CHECK(cpu.regs.A == 0x0);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
}


TEST_CASE("CPU test CP A,reg") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::CP_A_B);
    cpu.regs.B = 0x23;

    // CP simply performs a subtraction and doesn't store the result,
    // see the SUB A,reg tests for an explanation of the expected results

    SUBCASE("Test CP A,B no flags") {
        cpu.regs.A = 0x25;
        cpu.step();
        CHECK(cpu.regs.A == 0x25);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test CP A,B half-carry flag") {
        cpu.regs.A = 0x31;
        cpu.step();
        CHECK(cpu.regs.A == 0x31);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test CP A,B carry and half-carry flag") {
        cpu.regs.A = 0x20;
        cpu.step();
        CHECK(cpu.regs.A == 0x20);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test CP A,B zero flag") {
        cpu.regs.A = 0x23;
        cpu.step();
        CHECK(cpu.regs.A == 0x23);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}

TEST_CASE("CPU test CP A,[HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    cpu.regs.setHL(addr);
    bus.write8(pc, op::CP_A_inHL);
    bus.write8(addr, 0x23);

    // this works just like CP A,reg
    // here we only check if the right value is used

    SUBCASE("Test CP A,[HL] zero flag") {
        cpu.regs.A = 0x23;
        cpu.step();
        CHECK(cpu.regs.A == 0x23);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
}

TEST_CASE("CPU test CP A,n8") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::CP_A_n8);
    bus.write8(pc + 1, 0x23);

    // this works just like CP A,reg
    // here we only check if the right value is used

    SUBCASE("Test CP A,n8 zero flag") {
        cpu.regs.A = 0x23;
        cpu.step();
        CHECK(cpu.regs.A == 0x23);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
}



TEST_CASE("CPU test INC reg 8-bit") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::INC_A);

    // we use reg A, the code is the same for the other registers
    // the carry flag is never set 

    SUBCASE("Test INC A no flags") {
        cpu.regs.A = 0x01;
        cpu.step();
        CHECK(cpu.regs.A == 0x02);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test INC A half carry flag") {
        cpu.regs.A = 0x0f;
        cpu.step();
        CHECK(cpu.regs.A == 0x10);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test INC A carry, half-carry and zero flags") {
        cpu.regs.A = 0xff;
        cpu.step();
        CHECK(cpu.regs.A == 0x00);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}

TEST_CASE("CPU test INC [HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;
    uint16_t addr = 0x1234;

    bus.write8(pc, op::INC_inHL);
    bus.write8(addr, 0x20);
    cpu.regs.setHL(addr);

    cpu.step();

    CHECK(bus.read8(addr) == 0x21);

    CHECK_FALSE(cpu.regs.flags.Z);
    CHECK_FALSE(cpu.regs.flags.C);
    CHECK_FALSE(cpu.regs.flags.H);
    CHECK_FALSE(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 3);
}



TEST_CASE("CPU test DEC reg 8-bit") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::DEC_A);

    // we use reg A, the code is the same for the other registers
    // the carry flag is never set

    SUBCASE("Test DEC A no flags") {
        cpu.regs.A = 0x03;
        cpu.step();
        CHECK(cpu.regs.A == 0x02);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test DEC A half carry flag") {
        cpu.regs.A = 0x10;
        cpu.step();
        CHECK(cpu.regs.A == 0x0f);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test DEC A zero flag") {
        cpu.regs.A = 0x01;
        cpu.step();
        CHECK(cpu.regs.A == 0x00);

        CHECK(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}

TEST_CASE("CPU test DEC [HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;
    uint16_t addr = 0x1234;

    bus.write8(pc, op::DEC_inHL);
    bus.write8(addr, 0x32);
    cpu.regs.setHL(addr);

    cpu.step();

    CHECK(bus.read8(addr) == 0x31);

    CHECK_FALSE(cpu.regs.flags.Z);
    CHECK_FALSE(cpu.regs.flags.C);
    CHECK_FALSE(cpu.regs.flags.H);
    CHECK(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 3);
}



TEST_CASE("CPU test CCF") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    // flip the C flag and clear H and N, Z is unchanged

    bus.write8(pc, op::CCF);
    cpu.regs.flags.C = 1;

    cpu.step();

    CHECK_FALSE(cpu.regs.flags.C);
    CHECK_FALSE(cpu.regs.flags.H);
    CHECK_FALSE(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 1);
}

TEST_CASE("CPU test SCF") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    // set the C flag and clear H and N, Z is unchanged

    bus.write8(pc, op::SCF);
    cpu.regs.flags.C = 0;

    cpu.step();

    CHECK(cpu.regs.flags.C);
    CHECK_FALSE(cpu.regs.flags.H);
    CHECK_FALSE(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 1);
}

TEST_CASE("CPU test CPL") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    // flips all bits of reg A and sets H and N

    bus.write8(pc, op::CPL);
    cpu.regs.A = 0x12;

    cpu.step();

    CHECK(cpu.regs.A == 0xED);

    CHECK(cpu.regs.flags.H);
    CHECK(cpu.regs.flags.N);

    CHECK(cpu.elapsedCycles() == 1);
}


TEST_CASE("CPU test DAA") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

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

    SUBCASE("Test ADD A,B then DAA") {
        // we want to do 45 + 38 = 83 in decimal
        // so we do 0x45 + 0x38 = 0x7D in hex
        // to convert 0x7D to 0x83 we have to add 6
        // 
        // 0x45     0100 0101 +
        // 0x38     0011 1000 =
        //          -----------
        // 0x7D     0111 1101 +
        // 0x06     0000 0110 =
        //          -----------
        // 0x83     1000 0111


        bus.write8(pc, op::ADD_A_B);
        bus.write8(pc + 1, op::DAA);

        cpu.regs.A = 0x45;
        cpu.regs.B = 0x38;

        // perform ADD A,B
        cpu.step();
        CHECK(cpu.regs.A == 0x7D);
        CHECK_FALSE(cpu.regs.flags.N);
        CHECK(cpu.elapsedCycles() == 1);

        // perform DAA
        cpu.step();
        CHECK(cpu.regs.A == 0x83);
        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.H); // H is always false
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.elapsedCycles() == 2);
    }

    SUBCASE("Test SUB A,B then DAA") {
        // we want to do 83 - 38 = 45 in decimal
        // so we do 0x83 - 0x38 = 0x4B in hex
        // to convert 0x4B to 0x45 we have to subtract 6
        // that is, sum the 2 complement of 6 which is 0xFA
        // 
        // 0x83     1000 0011 -
        // 0x38     0011 1000 =
        //          -----------
        // 0x4B     0100 1011 +
        // 0xFA     1111 1010 =
        //          -----------
        // 0x45     0100 0101 (+carry)


        bus.write8(pc, op::SUB_A_B);
        bus.write8(pc + 1, op::DAA);

        cpu.regs.A = 0x83;
        cpu.regs.B = 0x38;

        // perform SUB A,B
        cpu.step();
        CHECK(cpu.regs.A == 0x4B);
        CHECK(cpu.regs.flags.N);
        CHECK(cpu.elapsedCycles() == 1);

        // perform DAA
        cpu.step();
        CHECK(cpu.regs.A == 0x45);
        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.H); // H is always false
        CHECK(cpu.regs.flags.C);
        CHECK(cpu.elapsedCycles() == 2);
    }
}


TEST_CASE("CPU test ADD HL,reg16") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::ADD_HL_BC);
    cpu.regs.setBC(0x0333);

    // we use BC, when using other registers the code is the same
    // Z flag is not used by this instruction, only C and H are updated and N is always false

    SUBCASE("Test ADD HL,BC no flags") {
        cpu.regs.setHL(0x0444);
        cpu.step();
        
        CHECK(cpu.regs.HL() == 0x0777);

        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test ADD HL,BC half-carry flag") {
        cpu.regs.setHL(0x0f44);
        cpu.step();

        CHECK(cpu.regs.HL() == 0x1277);

        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test ADD HL,BC carry and half-carry flags") {
        cpu.regs.setHL(0xff44);
        cpu.step();

        CHECK(cpu.regs.HL() == 0x0277);

        CHECK(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 2);
    }
}


TEST_CASE("CPU test ADD SP,e8") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::ADD_SP_e8);

    // the immediate value is considered to be a signed value
    // only C and H are updated, Z and N is always false

    SUBCASE("Test ADD SP,e8 no flags") {
        cpu.regs.SP = 0x0333;
        bus.write8(pc + 1, 0x22);
        cpu.step();

        CHECK(cpu.regs.SP == 0x0355);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 4);
    }
    SUBCASE("Test ADD SP,e8 half-carry flag") {
        cpu.regs.SP = 0x002E;
        bus.write8(pc + 1, 0x13);
        cpu.step();

        CHECK(cpu.regs.SP == 0x0041);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 4);
    }
    SUBCASE("Test ADD SP,e8 carry flag") {
        cpu.regs.SP = 0x00EE;
        bus.write8(pc + 1, 0x31);
        cpu.step();

        CHECK(cpu.regs.SP == 0x011F);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 4);
    }
    SUBCASE("Test ADD SP,e8 negative value") {
        cpu.regs.SP = 0x00EE;
        bus.write8(pc + 1, (uint8_t)-10);
        cpu.step();

        CHECK(cpu.regs.SP == 0x00E4);

        // in this case both C and H should be set because it's like adding 
        // FFF6 to SP (00EE) so both carry and half-carry are true
        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.C);
        CHECK(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 4);
    }
}


TEST_CASE("CPU test INC/DEC reg 16-bit") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    SUBCASE("Test INC BC") {
        bus.write8(pc, op::INC_BC);
        cpu.regs.setBC(0x0333);
        cpu.step();

        CHECK(cpu.regs.BC() == 0x0334);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test INC DE") {
        bus.write8(pc, op::INC_DE);
        cpu.regs.setDE(0x0333);
        cpu.step();

        CHECK(cpu.regs.DE() == 0x0334);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test INC HL") {
        bus.write8(pc, op::INC_HL);
        cpu.regs.setHL(0x0333);
        cpu.step();

        CHECK(cpu.regs.HL() == 0x0334);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test INC SP") {
        bus.write8(pc, op::INC_SP);
        cpu.regs.SP = 0x0333;
        cpu.step();

        CHECK(cpu.regs.SP == 0x0334);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test DEC BC") {
        bus.write8(pc, op::DEC_BC);
        cpu.regs.setBC(0x0333);
        cpu.step();

        CHECK(cpu.regs.BC() == 0x0332);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test DEC DE") {
        bus.write8(pc, op::DEC_DE);
        cpu.regs.setDE(0x0333);
        cpu.step();

        CHECK(cpu.regs.DE() == 0x0332);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test DEC HL") {
        bus.write8(pc, op::DEC_HL);
        cpu.regs.setHL(0x0333);
        cpu.step();

        CHECK(cpu.regs.HL() == 0x0332);
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test DEC SP") {
        bus.write8(pc, op::DEC_SP);
        cpu.regs.SP = 0x0333;
        cpu.step();

        CHECK(cpu.regs.SP == 0x0332);
        CHECK(cpu.elapsedCycles() == 2);
    }
}



TEST_CASE("CPU test RLCA") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    // Rotate A left, bit 7 of A goes into the carry flag as well 
    // as coming back into bit 0 of A
    // the other flags are all reset

    bus.write8(pc, op::RLCA);

    SUBCASE("Test RLCA with bit 7 == 0") {
        cpu.regs.A = 0x74;
        cpu.step();

        CHECK(cpu.regs.A == 0xE8);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test RLCA with bit 7 == 1") {
        cpu.regs.A = 0xF4;
        cpu.step();

        CHECK(cpu.regs.A == 0xE9);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}


TEST_CASE("CPU test RLA") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    // Rotate A left, bit 7 of A goes into the carry flag and the previous
    // value of C comes back into bit 0 of A
    // the other flags are all reset

    bus.write8(pc, op::RLA);

    SUBCASE("Test RCA with bit 7 == 1 an C == 0") {
        cpu.regs.A = 0xF4;
        cpu.regs.flags.C = false;
        cpu.step();

        CHECK(cpu.regs.A == 0xE8);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test RCA with bit 7 == 0 an C == 1") {
        cpu.regs.A = 0x74;
        cpu.regs.flags.C = true;
        cpu.step();

        CHECK(cpu.regs.A == 0xE9);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}


TEST_CASE("CPU test RRCA") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    // Rotate A right, bit 0 of A goes into the C flag as well 
    // as coming back into A from the left
    // the other flags are all reset

    bus.write8(pc, op::RRCA);

    SUBCASE("Test RRCA with bit 0 == 0") {
        cpu.regs.A = 0x7E;
        cpu.step();

        CHECK(cpu.regs.A == 0x3F);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test RRCA with bit 0 == 1") {
        cpu.regs.A = 0x7B;
        cpu.step();

        CHECK(cpu.regs.A == 0xBD);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}


TEST_CASE("CPU test RRA") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t pc = Registers::PCinitialValue;

    // Rotate A right, bit 0 of A goes into the C flag and the previous
    // value of C comes back into bit 7 of A
    // the other flags are all reset

    bus.write8(pc, op::RRA);

    SUBCASE("Test RRA with bit 0 == 1 an C == 0") {
        cpu.regs.A = 0x23;
        cpu.regs.flags.C = false;
        cpu.step();

        CHECK(cpu.regs.A == 0x11);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test RRA with bit 0 == 0 an C == 1") {
        cpu.regs.A = 0x32;
        cpu.regs.flags.C = true;
        cpu.step();

        CHECK(cpu.regs.A == 0x99);

        CHECK_FALSE(cpu.regs.flags.Z);
        CHECK_FALSE(cpu.regs.flags.C);
        CHECK_FALSE(cpu.regs.flags.H);
        CHECK_FALSE(cpu.regs.flags.N);

        CHECK(cpu.elapsedCycles() == 1);
    }
}




TEST_CASE("CPU test conditional jumps JP C/Z/NC/NZ, a16") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc + 1, addr & 0xff);
    bus.write8(pc + 2, addr >> 8);

    SUBCASE("Test JP C,a16 branch not taken") {
        bus.write8(pc, op::JP_C_a16);
        cpu.step();
        CHECK(cpu.regs.PC == pc + 3);
        CHECK(cpu.elapsedCycles() == 3);
    }
    SUBCASE("Test JP C,a16 branch taken") {
        bus.write8(pc, op::JP_C_a16);
        cpu.regs.flags.C = true;
        cpu.step();
        CHECK(cpu.regs.PC == addr);
        CHECK(cpu.elapsedCycles() == 4);
    }
    SUBCASE("Test JP Z,a16 branch not taken") {
        bus.write8(pc, op::JP_Z_a16);
        cpu.step();
        CHECK(cpu.regs.PC == pc + 3);
        CHECK(cpu.elapsedCycles() == 3);
    }
    SUBCASE("Test JP Z,a16 branch taken") {
        bus.write8(pc, op::JP_Z_a16);
        cpu.regs.flags.Z = true;
        cpu.step();
        CHECK(cpu.regs.PC == addr);
        CHECK(cpu.elapsedCycles() == 4);
    }
    SUBCASE("Test JP NC,a16 branch not taken") {
        bus.write8(pc, op::JP_NC_a16);
        cpu.regs.flags.C = true;
        cpu.step();
        CHECK(cpu.regs.PC == pc + 3);
        CHECK(cpu.elapsedCycles() == 3);
    }
    SUBCASE("Test JP NC,a16 branch taken") {
        bus.write8(pc, op::JP_NC_a16);
        cpu.step();
        CHECK(cpu.regs.PC == addr);
        CHECK(cpu.elapsedCycles() == 4);
    }
    SUBCASE("Test JP NZ,a16 branch not taken") {
        bus.write8(pc, op::JP_NZ_a16);
        cpu.regs.flags.Z = true;
        cpu.step();
        CHECK(cpu.regs.PC == pc + 3);
        CHECK(cpu.elapsedCycles() == 3);
    }
    SUBCASE("Test JP NZ,a16 branch taken") {
        bus.write8(pc, op::JP_NZ_a16);
        cpu.step();
        CHECK(cpu.regs.PC == addr);
        CHECK(cpu.elapsedCycles() == 4);
    }
}



TEST_CASE("CPU test unconditional jump JP a16") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::JP_a16);
    bus.write8(pc + 1, addr & 0xff);
    bus.write8(pc + 2, addr >> 8);

    cpu.step();

    CHECK(cpu.regs.PC == addr);
    CHECK(cpu.elapsedCycles() == 4);
}

TEST_CASE("CPU test unconditional jump JP HL") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    bus.write8(pc, op::JP_HL);
    cpu.regs.setHL(addr);
    cpu.step();

    CHECK(cpu.regs.PC == addr);
    CHECK(cpu.elapsedCycles() == 1);
}
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
        
        CHECK_FALSE(cpu.regs.getZ());
        CHECK_FALSE(cpu.regs.getH());
        CHECK_FALSE(cpu.regs.getC());
        CHECK_FALSE(cpu.regs.getN());
        
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADD A,B zero flag") {
        cpu.regs.A = 0;
        cpu.regs.B = 0;
        cpu.step();
        CHECK(cpu.regs.A == 0);
        
        CHECK(cpu.regs.getZ());
        CHECK_FALSE(cpu.regs.getH());
        CHECK_FALSE(cpu.regs.getC());
        CHECK_FALSE(cpu.regs.getN());
        
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADD A,B carry flag") {
        cpu.regs.A = 255;
        cpu.regs.B = 2;
        cpu.step();
        CHECK(cpu.regs.A == 1);
        
        CHECK_FALSE(cpu.regs.getZ());
        CHECK_FALSE(cpu.regs.getH());
        CHECK(cpu.regs.getC());
        CHECK_FALSE(cpu.regs.getN());

        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADD A,B zero and carry flags") {
        cpu.regs.A = 255;
        cpu.regs.B = 1;
        cpu.step();
        CHECK(cpu.regs.A == 0);

        CHECK(cpu.regs.getZ());
        CHECK_FALSE(cpu.regs.getH());
        CHECK(cpu.regs.getC());
        CHECK_FALSE(cpu.regs.getN());
        
        CHECK(cpu.elapsedCycles() == 1);
    }
    SUBCASE("Test ADD A,B half-carry flag") {
        cpu.regs.A = 0x0f;
        cpu.regs.B = 1;
        cpu.step();
        CHECK(cpu.regs.A == 0x10);

        CHECK_FALSE(cpu.regs.getZ());
        CHECK(cpu.regs.getH());
        CHECK_FALSE(cpu.regs.getC());
        CHECK_FALSE(cpu.regs.getN());

        CHECK(cpu.elapsedCycles() == 1);
    }
}



TEST_CASE("CPU test ADD A,[HL]") {
    TestBus bus;
    CPU cpu(bus);

    const uint16_t addr = 0x1234;
    const uint16_t pc = Registers::PCinitialValue;

    cpu.regs.setHL(addr);

    bus.write8(pc, op::ADD_A_inHL);

    SUBCASE("Test ADD A,[HL] no flags") {
        cpu.regs.A = 1;
        bus.write8(addr, 2);
        cpu.step();
        CHECK(cpu.regs.A == 3);
        
        CHECK_FALSE(cpu.regs.getZ());
        CHECK_FALSE(cpu.regs.getH());
        CHECK_FALSE(cpu.regs.getC());
        CHECK_FALSE(cpu.regs.getN());
        
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test ADD A,[HL] zero flag") {
        cpu.regs.A = 0;
        bus.write8(addr, 0);
        cpu.step();
        CHECK(cpu.regs.A == 0);
        
        CHECK(cpu.regs.getZ());
        CHECK_FALSE(cpu.regs.getH());
        CHECK_FALSE(cpu.regs.getC());
        CHECK_FALSE(cpu.regs.getN());
        
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test ADD A,[HL] carry flag") {
        cpu.regs.A = 255;
        bus.write8(addr, 2);
        cpu.step();
        CHECK(cpu.regs.A == 1);
        
        CHECK_FALSE(cpu.regs.getZ());
        CHECK_FALSE(cpu.regs.getH());
        CHECK(cpu.regs.getC());
        CHECK_FALSE(cpu.regs.getN());

        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test ADD A,[HL] zero and carry flags") {
        cpu.regs.A = 255;
        bus.write8(addr, 1);
        cpu.step();
        CHECK(cpu.regs.A == 0);

        CHECK(cpu.regs.getZ());
        CHECK_FALSE(cpu.regs.getH());
        CHECK(cpu.regs.getC());
        CHECK_FALSE(cpu.regs.getN());
        
        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test ADD A,[HL] half-carry flag") {
        cpu.regs.A = 0x0f;
        bus.write8(addr, 1);
        cpu.step();
        CHECK(cpu.regs.A == 0x10);

        CHECK_FALSE(cpu.regs.getZ());
        CHECK(cpu.regs.getH());
        CHECK_FALSE(cpu.regs.getC());
        CHECK_FALSE(cpu.regs.getN());

        CHECK(cpu.elapsedCycles() == 2);
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

    // To perform the subtraction CP just sums A and the compl2 of mem[HL],
    // this results in a few cases:
    // - flag N is always set because a subtraction has been performed
    // - when A == mem[HL] we will have Z and C set but not H
    // - when A > mem[HL] we will have C set but not Z, H depends on the numbers
    // - when A < mem[HL] both C and Z won't be set

    SUBCASE("Test CP A,[HL] carry flag") {
        cpu.regs.A = 0x25;
        cpu.step();
        CHECK(cpu.regs.A == 0x25);

        CHECK_FALSE(cpu.regs.getZ());
        CHECK(cpu.regs.getC());
        CHECK(cpu.regs.getN());
        CHECK_FALSE(cpu.regs.getH());

        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test CP A,[HL] half-carry flag") {
        cpu.regs.A = 0x20;
        cpu.step();
        CHECK(cpu.regs.A == 0x20);

        CHECK_FALSE(cpu.regs.getZ());
        CHECK_FALSE(cpu.regs.getC());
        CHECK(cpu.regs.getN());
        CHECK(cpu.regs.getH());

        CHECK(cpu.elapsedCycles() == 2);
    }
    SUBCASE("Test CP A,[HL] zero and carry flag") {
        cpu.regs.A = 0x23;
        cpu.step();
        CHECK(cpu.regs.A == 0x23);

        CHECK(cpu.regs.getZ());
        CHECK(cpu.regs.getC());
        CHECK(cpu.regs.getN());
        CHECK_FALSE(cpu.regs.getH());

        CHECK(cpu.elapsedCycles() == 2);
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
        cpu.regs.setC();
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
        cpu.regs.setZ();
        cpu.step();
        CHECK(cpu.regs.PC == addr);
        CHECK(cpu.elapsedCycles() == 4);
    }
    SUBCASE("Test JP NC,a16 branch not taken") {
        bus.write8(pc, op::JP_NC_a16);
        cpu.regs.setC();
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
        cpu.regs.setZ();
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
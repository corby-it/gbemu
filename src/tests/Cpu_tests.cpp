#include "gb/Cpu.h"
#include "doctest/doctest.h"


TEST_CASE("PROVA") {
    Bus bus;
    CPU cpu(bus);

    CHECK(cpu.regs().A == 0);
    CHECK(cpu.regs().PC == Registers::PCinitialValue);
}
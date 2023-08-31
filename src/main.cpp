
#include "gb/Bus.h"
#include "gb/Cpu.h"
#include "gb/Opcodes.h"

// this is required to implement a test runner for doctest, otherwise it won't compile
// in release mode everything will be stripped away
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"


// Main code
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    Bus bus;
    CPU cpu(bus);

    bus.write8(0x00, op::LD_A_n8); // LD A,0
    bus.write8(0x01, 0);
    bus.write8(0x02, op::LD_B_n8); // LD B,3
    bus.write8(0x03, 3);
    bus.write8(0x04, op::LD_H_n8); // LD H,ff
    bus.write8(0x05, 0xff);
    bus.write8(0x06, op::LD_L_n8); // LD L,0
    bus.write8(0x07, 0);
    bus.write8(0x08, op::CP_A_inHL); // CP A,[HL]
    bus.write8(0x09, op::JP_Z_a16); // JP Z,end
    bus.write8(0x0A, 0x10); // jp addrL
    bus.write8(0x0B, 0x00); // jp addrH
    bus.write8(0x0C, op::ADD_A_B); // ADD B
    bus.write8(0x0D, op::JP_a16); // JP to CP A
    bus.write8(0x0E, 0x08); // jp addrL
    bus.write8(0x0F, 0x00); // jp addrH
    bus.write8(0x10, 0x01); // wrong opcode to stop the cpu

    bus.write8(0xff00, 30); // compare val
    

    while(cpu.step()) {}

    return 0;
}

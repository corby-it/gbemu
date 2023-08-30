
#include "gb/Bus.h"
#include "gb/Cpu.h"

// Main code
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    Bus bus;
    CPU cpu(bus);

    bus.write8(0x00, 0x3E); // LD A,0
    bus.write8(0x01, 0);
    bus.write8(0x02, 0x06); // LD B,3
    bus.write8(0x03, 3);
    bus.write8(0x04, 0x26); // LD H,ff
    bus.write8(0x05, 0xff);
    bus.write8(0x06, 0x2E); // LD L,0
    bus.write8(0x07, 0);
    bus.write8(0x08, 0xBE); // CP A,[HL]
    bus.write8(0x09, 0xCA); // JP Z,end
    bus.write8(0x0A, 0x10); // jp addrL
    bus.write8(0x0B, 0x00); // jp addrH
    bus.write8(0x0C, 0x80); // ADD B
    bus.write8(0x0D, 0xC3); // JP to compare
    bus.write8(0x0E, 0x08); // jp addrL
    bus.write8(0x0F, 0x00); // jp addrH
    bus.write8(0x10, 0x01); // wrong opcode to stop the cpu

    bus.write8(0xff00, 30); // compare val
    

    while(cpu.step()) {}

    return 0;
}

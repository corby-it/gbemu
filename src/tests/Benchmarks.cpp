
#include "gb/GameBoyCore.h"
#include <array>
#include <nanobench.h>
#include <doctest/doctest.h>


class RWMock : public ReadWriteIf {
public:
    uint8_t read8(uint16_t addr) const override
    {
        (void)addr;
        return 0;
    }

    void write8(uint16_t addr, uint8_t val) override
    {
        (void)addr;
        (void)val;
    }
};



class GameBoyMap : public Bus {
public:
    GameBoyMap() : mAddrMap(initAddressMap()) {}

    uint8_t read8(uint16_t addr) const override
    {
        auto it = mAddrMap.lower_bound(addr);
        if (it == mAddrMap.end())
            return 0xff;

        return it->second ? it->second->read8(addr) : 0xff;
    }

    void write8(uint16_t addr, uint8_t val) override
    {
        auto it = mAddrMap.lower_bound(addr);
        if (it == mAddrMap.end())
            return;

        if (auto* writeObj = it->second; writeObj) {
            writeObj->write8(addr, val);
        }
    }

private:

    RWMock cpu;
    RWMock wram;
    RWMock ppu;
    RWMock dma;
    RWMock cartridge;
    RWMock timer;
    RWMock joypad;
    RWMock apu;
    RWMock serial;
    RWMock hiRam;
    RWMock infrared;
    RWMock undocRegs;

    AddressMap initAddressMap() {

        AddressMap map = {
            // memory -------------------------------------------------------------
            { mmap::rom::start, &cartridge },
            { mmap::rom::end, &cartridge },
            { mmap::vram::start, &ppu },
            { mmap::vram::end, &ppu },
            { mmap::external_ram::start, &cartridge },
            { mmap::external_ram::end, &cartridge },
            { mmap::wram::start, &wram },
            { mmap::wram::end, &wram },
            { mmap::echoram::start, &wram },
            { mmap::echoram::end, &wram },
            { mmap::oam::start, &ppu },
            { mmap::oam::end, &ppu },
            { mmap::prohibited::start, nullptr },
            { mmap::prohibited::end, nullptr },

            // control registers --------------------------------------------------
            { mmap::regs::joypad, &joypad },
            { mmap::regs::serial_data, &serial },
            { mmap::regs::serial_ctrl, &serial },
            { mmap::regs::timer::start - 1, nullptr },
            { mmap::regs::timer::start, &timer },
            { mmap::regs::timer::end, &timer },
            { mmap::regs::IF - 1, nullptr },
            { mmap::regs::IF, &cpu },
            { mmap::regs::audio::start, &apu },
            { mmap::regs::audio::end, &apu },
            { mmap::regs::lcd::start, &ppu },
            { mmap::regs::lcd::lyc, &ppu },
            { mmap::regs::lcd::dma, &dma },
            { mmap::regs::lcd::bgp, &ppu },
            { mmap::regs::lcd::end, &ppu },
            { mmap::regs::key0, nullptr },      // KEY0
            { mmap::regs::key1, &cpu },         // KEY1
            { mmap::regs::vbk - 1, nullptr },   // VBK
            { mmap::regs::vbk, &ppu },     // VBK
            { mmap::regs::boot, nullptr },      // BOOT
            { mmap::regs::hdma::start, &ppu },   // HDMA
            { mmap::regs::hdma::end, &ppu },     // HDMA
            { mmap::regs::infrared, &infrared },    // Infrared
            { mmap::regs::col_palette::start - 1, nullptr },     // Color palette
            { mmap::regs::col_palette::start, &ppu },    // Color palette
            { mmap::regs::col_palette::end, &ppu },      // Color palette
            { mmap::regs::opri, nullptr },      // OPRI
            { mmap::regs::svbk - 1, nullptr },  // SVBK
            { mmap::regs::svbk, &wram },        // SVBK
            { mmap::regs::undocumented::start - 1, nullptr },   // undoc regs
            { mmap::regs::undocumented::start, &undocRegs },    // undoc regs
            { mmap::regs::undocumented::end, &undocRegs },      // undoc regs
            { mmap::regs::pcm12, &apu },  // PCM regs
            { mmap::regs::pcm34, &apu },  // PCM regs
            { mmap::hiram::start - 1, nullptr },
            { mmap::hiram::start, &hiRam },
            { mmap::hiram::end, &hiRam },
            { mmap::IE, &cpu },
        };

        return map;
    }

    const AddressMap mAddrMap;
};


class GameBoyDirect : public Bus {
public:
    GameBoyDirect()
    {
        initAddrMap();
    }

    uint8_t read8(uint16_t addr) const override
    {
        uint8_t lo = (uint8_t)addr;
        uint8_t hi = addr >> 8;

        switch (hi) {
        case 0xFE: return oamSpace[lo >> 4]->read8(addr);
        case 0xFF: return regsSpace[lo]->read8(addr);
        default:
            return mainSpace[hi]->read8(addr);
        }
    }

    void write8(uint16_t addr, uint8_t val) override
    {
        uint8_t lo = (uint8_t)addr;
        uint8_t hi = addr >> 8;

        switch (hi) {
        case 0xFE: oamSpace[lo >> 4]->write8(addr, val); break;
        case 0xFF: regsSpace[lo]->write8(addr, val); break;
        default:
            mainSpace[hi]->write8(addr, val);
        }
    }

private:
    void initAddrMap()
    {
        // fill all address with the empty address space handler so that there 
        // are no nullptrs in the arrays
        mainSpace.fill(&emptySpace);
        oamSpace.fill(&emptySpace);
        regsSpace.fill(&emptySpace);

        // the mainSpace map handles addresses from 00xx to FDxx using the upper byte of the address:
        // - ROM (0000 - 7FFF)
        // - VRAM (8000 - 9FFF)
        // - external RAM (A000 - BFFF)
        // - work RAM (C000 - DFFF)
        // - echo RAM (E000 - FDFF)

        for (uint32_t addr = mmap::rom::start >> 8; addr <= mmap::rom::end >> 8; addr++)
            mainSpace[addr] = &cartridge;
        for (uint32_t addr = mmap::vram::start >> 8; addr <= mmap::vram::end >> 8; addr++)
            mainSpace[addr] = &ppu;
        for (uint32_t addr = mmap::external_ram::start >> 8; addr <= mmap::external_ram::end >> 8; addr++)
            mainSpace[addr] = &cartridge;
        for (uint32_t addr = mmap::wram::start >> 8; addr <= mmap::wram::end >> 8; addr++)
            mainSpace[addr] = &wram;
        for (uint32_t addr = mmap::echoram::start >> 8; addr <= mmap::echoram::end >> 8; addr++)
            mainSpace[addr] = &wram;

        // the oamSpace map handles addresses from FE00 - FEFF using the top nibble of the lower byte:
        // - OAM RAM (FE000 - FE9F)
        // - forbidden space (FEA0 - FEFF), same as empty space

        for (uint32_t addr = 0; addr < 0x0A; addr++)
            oamSpace[addr] = &ppu;

        // the regsSpace map handles addresses from FF00 - FFFF using the lower byte of the address:
        // - registers (FF00 - FF7F)
        // - high RAM (FF80 - FFFE)
        // - interrupt enable reg (FFFF)

        regsSpace[mmap::regs::joypad & 0xFF] = &joypad;
        regsSpace[mmap::regs::serial_data & 0xFF] = &serial;
        regsSpace[mmap::regs::serial_ctrl & 0xFF] = &serial;

        for (uint32_t addr = mmap::regs::timer::start & 0xFF; addr <= (mmap::regs::timer::end & 0xFF); addr++)
            regsSpace[addr] = &timer;

        regsSpace[mmap::regs::IF & 0xFF] = &cpu;

        for (uint32_t addr = mmap::regs::audio::start & 0xFF; addr <= (mmap::regs::audio::end & 0xFF); addr++)
            regsSpace[addr] = &apu;

        for (uint32_t addr = mmap::regs::lcd::start & 0xFF; addr <= (mmap::regs::lcd::end & 0xFF); addr++)
            regsSpace[addr] = &ppu;

        regsSpace[mmap::regs::lcd::dma & 0xFF] = &dma;
        regsSpace[mmap::regs::key1 & 0xFF] = &cpu;
        regsSpace[mmap::regs::vbk & 0xFF] = &ppu;

        for (uint32_t addr = mmap::regs::hdma::start & 0xFF; addr <= (mmap::regs::hdma::end & 0xFF); addr++)
            regsSpace[addr] = &ppu;

        regsSpace[mmap::regs::infrared & 0xFF] = &infrared;

        for (uint32_t addr = mmap::regs::col_palette::start & 0xFF; addr <= (mmap::regs::col_palette::end & 0xFF); addr++)
            regsSpace[addr] = &ppu;

        regsSpace[mmap::regs::svbk & 0xFF] = &wram;

        for (uint32_t addr = mmap::regs::undocumented::start & 0xFF; addr <= (mmap::regs::undocumented::end & 0xFF); addr++)
            regsSpace[addr] = &undocRegs;

        regsSpace[mmap::regs::pcm12 & 0xFF] = &apu;
        regsSpace[mmap::regs::pcm34 & 0xFF] = &apu;

        for (uint32_t addr = mmap::hiram::start & 0xFF; addr <= (mmap::hiram::end & 0xFF); addr++)
            regsSpace[addr] = &hiRam;

        regsSpace[mmap::IE & 0xFF] = &cpu;
    }

    RWMock cpu;
    RWMock wram;
    RWMock ppu;
    RWMock dma;
    RWMock cartridge;
    RWMock timer;
    RWMock joypad;
    RWMock apu;
    RWMock serial;
    RWMock hiRam;
    RWMock infrared;
    RWMock undocRegs;

    EmptyAddrSpace emptySpace;


    std::array<ReadWriteIf*, 256> mainSpace;
    std::array<ReadWriteIf*, 16> oamSpace;
    std::array<ReadWriteIf*, 256> regsSpace;
};





//TEST_CASE("Benchmark read/write") {
//
//    GameBoyMap gbMap;
//    GameBoyDirect gbDirect;
//
//    uint16_t lowAddr = 0xC00A;
//    uint16_t highAddr = 0xFFE0;
//
//    ankerl::nanobench::Bench benchLow;
//    benchLow.title("Read/write access - Low addr")
//        .unit("uint8_t")
//        .warmup(100)
//        .relative(true)
//        .performanceCounters(true);
//
//    benchLow.run("std::map", [&]() {
//        auto val = gbMap.read8(lowAddr);
//        ankerl::nanobench::doNotOptimizeAway(val);
//    });
//    benchLow.run("direct access", [&]() {
//        auto val = gbDirect.read8(lowAddr);
//        ankerl::nanobench::doNotOptimizeAway(val);
//    });
//
//
//    ankerl::nanobench::Bench benchHigh;
//    benchHigh.title("Read/write access - High addr")
//        .unit("uint8_t")
//        .warmup(100)
//        .relative(true)
//        .performanceCounters(true);
//
//    benchHigh.run("std::map", [&]() {
//        auto val = gbMap.read8(highAddr);
//        ankerl::nanobench::doNotOptimizeAway(val);
//    });
//    benchHigh.run("direct access", [&]() {
//        auto val = gbDirect.read8(highAddr);
//        ankerl::nanobench::doNotOptimizeAway(val);
//    });
//
//}
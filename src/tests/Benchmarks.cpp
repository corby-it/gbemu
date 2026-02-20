
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
        : bins{}
        , subBins{}
        , regsBins{}
    {
        for (auto& obj : bins) {
            obj = &cpu;
        }
        for (auto& obj : subBins) {
            obj = &ppu;
        }
        for (auto& obj : regsBins) {
            obj = &apu;
        }
    }

    uint8_t read8(uint16_t addr) const override
    {
        uint8_t lo = (uint8_t)addr;
        uint8_t hi = addr >> 8;

        switch (hi) {
        case 0xFE: return subBins[lo]->read8(addr);
        case 0xFF: return regsBins[lo]->read8(addr);
        default: return bins[hi]->read8(addr);
        }
    }

    void write8(uint16_t addr, uint8_t val) override
    {
        uint8_t lo = (uint8_t)addr;
        uint8_t hi = addr >> 8;

        switch (hi) {
        case 0xFE: subBins[lo]->write8(addr, val); break;
        case 0xFF: regsBins[lo]->write8(addr, val); break;
        default: bins[hi]->write8(addr, val); break;
        }
    }

private:

    std::array<RWMock*, 256> bins;
    std::array<RWMock*, 256> subBins;
    std::array<RWMock*, 256> regsBins;

    RWMock cpu;
    RWMock ppu;
    RWMock apu;
};







TEST_CASE("Benchmark read/write") {

    GameBoyMap gbMap;
    GameBoyDirect gbDirect;

    uint16_t lowAddr = 0xC00A;
    uint16_t highAddr = 0xFFE0;

    ankerl::nanobench::Bench benchLow;
    benchLow.title("Read/write access - Low addr")
        .unit("uint8_t")
        .warmup(100)
        .relative(true)
        .performanceCounters(true);

    benchLow.run("std::map", [&]() {
        auto val = gbMap.read8(lowAddr);
        ankerl::nanobench::doNotOptimizeAway(val);
    });
    benchLow.run("direct access", [&]() {
        auto val = gbDirect.read8(lowAddr);
        ankerl::nanobench::doNotOptimizeAway(val);
    });


    ankerl::nanobench::Bench benchHigh;
    benchHigh.title("Read/write access - High addr")
        .unit("uint8_t")
        .warmup(100)
        .relative(true)
        .performanceCounters(true);

    benchHigh.run("std::map", [&]() {
        auto val = gbMap.read8(highAddr);
        ankerl::nanobench::doNotOptimizeAway(val);
    });
    benchHigh.run("direct access", [&]() {
        auto val = gbDirect.read8(highAddr);
        ankerl::nanobench::doNotOptimizeAway(val);
    });

}
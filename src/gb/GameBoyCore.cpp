

#include "GameBoyCore.h"
#include "GbCommons.h"
#include "Opcodes.h"
#include "gbdebug/Debug.h"



using hr_clock = std::chrono::high_resolution_clock;
using namespace std::chrono;
using namespace std::chrono_literals;

const char* GameBoyClassic::statusToStr(Status st)
{
    switch (st) {
    default:
    case Status::Stopped: return "Stopped";
    case Status::Paused: return "Paused";
    case Status::Playing: return "Playing";
    case Status::Stepping: return "Stepping";
    }
}



GameBoyClassic::GameBoyClassic()
    : bus()
    , cpu(bus)
    , wram(mmap::wram::start)
    , ppu(bus)
    , dma(bus)
    , cartridge()
    , timer(bus)
    , joypad(bus)
    , audio()
    , serial()
    , hiRam(mmap::hiram::start)
    , status(Status::Stopped)
    , mStepInstruction(false)
    , breakOnLdbb(false)
    , mMustStepTime(hr_clock::now())
    , mStepAvgTimeAccumulator(500us)
    , mStepTimeCounter(1)
{
    bus.connect(cpu);
    bus.connect(wram);
    bus.connect(ppu);
    bus.connect(dma);
    bus.connect(cartridge);
    bus.connect(timer);
    bus.connect(joypad);
    bus.connect(audio);
    bus.connect(serial);
    bus.connect(hiRam);

    gbReset();

    currInstruction = instructionToStr(bus, cpu.regs.PC);
}

void GameBoyClassic::gbReset()
{
    cpu.reset();
    wram.reset();
    ppu.reset();
    dma.reset();
    cartridge.reset();
    timer.reset();
    joypad.reset();
    audio.reset();
    serial.reset();
    hiRam.reset();

    currInstruction = instructionToStr(bus, cpu.regs.PC);
}

uint32_t GameBoyClassic::gbStep()
{
    auto [ok, cycles] = cpu.step();

    dma.step(cycles);
    ppu.step(cycles);
    timer.step(cycles);
    joypad.step(cycles);

    currInstruction = instructionToStr(bus, cpu.regs.PC);

    return cycles;
}



bool GameBoyClassic::emulate()
{
    // return true if there is more emulation work to be done (that is, if
    // the emulation is not stopped or paused)

    bool ret = false;

    switch (status) {
    default:
    case GameBoyClassic::Status::Stopped:
    case GameBoyClassic::Status::Paused: {
        // if the emulation is stopped or paused there is nothing to do
        break;
    }

    case GameBoyClassic::Status::Playing:
    {
        if (breakOnLdbb && bus.read8(cpu.regs.PC) == op::LD_B_B) {
            status = Status::Paused;
            ret = false;

        }
        else {
            // emulate the gameboy, full speed
            auto before = hr_clock::now();
            gbStep();
            auto after = hr_clock::now();

            mStepTimeCounter++;
            mStepAvgTimeAccumulator += duration_cast<nanoseconds>(after - before);
            ret = true;
        }
        break;
    }

    case GameBoyClassic::Status::Stepping: {
        // execute 1 instruction only if the user wants to
        if (mStepInstruction) {
            gbStep();
            mStepInstruction = false;
        }
        ret = false;
        break;
    }
    }

    return ret;
}

void GameBoyClassic::play()
{
    status = Status::Playing;
}

void GameBoyClassic::pause()
{
    status = Status::Paused;
}

void GameBoyClassic::stop()
{
    // on stop we also reset the whole gameboy
    status = Status::Stopped;
    gbReset();
}

void GameBoyClassic::step()
{
    status = Status::Stepping;
    mStepInstruction = true;
}

CartridgeLoadingRes GameBoyClassic::loadCartridge(const std::filesystem::path& path)
{
    auto res = cartridge.loadRomFile(path);

    if (res == CartridgeLoadingRes::Ok)
        gbReset();

    return res;
}

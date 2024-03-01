

#include "GameBoyCore.h"
#include "GbCommons.h"

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
}

uint32_t GameBoyClassic::gbStep()
{
    auto res = cpu.step();

    dma.step(res.cycles);
    ppu.step(res.cycles);
    timer.step(res.cycles);
    joypad.step(res.cycles);

    return res.cycles;
}



void GameBoyClassic::emulate()
{
    // if the emulation is stopped or paused there is nothing to do
    if (status == Status::Stopped || status == Status::Paused) {
        return;
    }
    else if (status == Status::Playing) {
        // emulate the gameboy, full speed
        auto tp = hr_clock::now();
        gbStep();
        auto elapsed = hr_clock::now() - tp;

        mStepTimeCounter++;
        mStepAvgTimeAccumulator += duration_cast<microseconds>(elapsed);
    }
    else if (status == Status::Stepping) {
        // execute 1 instruction only if the user wants to
        if (mStepInstruction) {
            gbStep();
            mStepInstruction = false;
        }
    }
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

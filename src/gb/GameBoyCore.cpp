

#include "GameBoyCore.h"
#include "GbCommons.h"

using hr_clock = std::chrono::high_resolution_clock;


const char* GameBoyClassic::statusToStr(Status st)
{
    switch (st) {
    default:
    case Status::Stopped: return "Stopped";
    case Status::Paused: return "Paused";
    case Status::Playing: return "Playing";
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
    , mMustStepTime(hr_clock::now())
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


void GameBoyClassic::emulate()
{
    // if the emulation is stopped or paused there is nothing to do
    if (status == Status::Stopped || status == Status::Paused)
        return;

    // emulate the gameboy
    auto now = hr_clock::now();

    if (now >= mMustStepTime) {
        // time to execute an instruction
        auto res = cpu.step();
        
        dma.step(res.cycles);
        ppu.step(res.cycles);
        timer.step(res.cycles);
        joypad.step(res.cycles);

        // the last instruction took N machine cycles, assuming our pc is much faster than the 
        // gameboy cpu we can compute a time in the future when we'll have to execute the next instruction
        mMustStepTime = now + machinePeriod * res.cycles;
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

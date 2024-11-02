

#include "GameBoyCore.h"
#include "GbCommons.h"
#include "Opcodes.h"
#include "gbdebug/Debug.h"
#include <tracy/Tracy.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <fstream>



using hr_clock = std::chrono::high_resolution_clock;
using namespace std::chrono;
using namespace std::chrono_literals;

namespace fs = std::filesystem;



const char* saveStateErrorToStr(SaveStateError err)
{
    switch (err) {
    case SaveStateError::NoError: return "No error";
    case SaveStateError::OpenFileError: return "Can't open file";
    case SaveStateError::CartridgeMismatch: return "The currently loaded cartridge header doesn't match the save state cartridge header";
    case SaveStateError::LoadingError: return "Loading error";
    default:
        assert(false);
        return "Unknown error";
    }
}





const char* GameBoyClassic::statusToStr(Status st)
{
    switch (st) {
    default:
    case Status::Stopped: return "Stopped";
    case Status::Paused: return "Paused";
    case Status::Running: return "Running";
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

    dbg.updateInstructionToStr(*this);
}

GbStepRes GameBoyClassic::gbStep()
{
    ZoneScoped;

    auto cpuRes = cpu.step();

    dma.step(cpuRes.cycles);
    bool frameReady = ppu.step(cpuRes.cycles);
    timer.step(cpuRes.cycles, cpu.isStopped());
    joypad.step(cpuRes.cycles);

    if (status != Status::Running) {
        ZoneScoped;
        dbg.updateInstructionToStr(*this);
    }

    GbStepRes res;
    res.cpuRes = cpuRes;
    res.frameReady = frameReady;

    return res;
}



EmulateRes GameBoyClassic::emulate()
{
    ZoneScoped;

    // return true if there is more emulation work to be done (that is, if
    // the emulation is not stopped or paused)

    EmulateRes res;
    res.stillGoing = false;

    switch (status) {
    default:
    case GameBoyClassic::Status::Stopped:
    case GameBoyClassic::Status::Paused: {
        // if the emulation is stopped or paused there is nothing to do
        break;
    }

    case GameBoyClassic::Status::Running:
    {
        // emulate the gameboy, full speed
        res.stepRes = gbStep();
        res.stillGoing = true;
        break;
    }

    case GameBoyClassic::Status::Stepping: {
        // execute 1 instruction only if the user wants to
        if (mStepInstruction) {
            res.stepRes = gbStep();
            mStepInstruction = false;
        }
        res.stillGoing = false;
        break;
    }
    }


    // debug stuff
    if (status == Status::Running && dbg.enabled) {
        if (dbg.breakOnLdbb && bus.read8(cpu.regs.PC) == op::LD_B_B) {
            status = Status::Paused;
            res.stillGoing = false;
        }
        else if (dbg.breakOnRet && cpu.callNesting() == dbg.targetCallNesting) {
            dbg.breakOnRet = false;
            status = Status::Paused;
            res.stillGoing = false;
        }
    }

    return res;
}

void GameBoyClassic::play()
{
    status = Status::Running;
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

void GameBoyClassic::stepReturn()
{
    // we can't step to a return instruction if the cpu
    // is not inside a function call
    if (cpu.callNesting() == 0)
        return;

    dbg.targetCallNesting = cpu.callNesting() - 1;
    dbg.breakOnRet = true;
    status = Status::Running;
}


CartridgeLoadingRes GameBoyClassic::loadCartridge(const std::filesystem::path& path)
{
    auto res = cartridge.loadRomFile(path);

    if (res == CartridgeLoadingRes::Ok) {
        romFilePath = path;
        gbReset();
    }

    // also try to load debug symbols (if any)
    dbg.symTable->parseSymbolFile(path);

    return res;
}

SaveStateError GameBoyClassic::saveState(const fs::path& path)
{
    std::ofstream ofs(path, std::ios::binary);
    if(!ofs)
        return SaveStateError::OpenFileError;

    {
        cereal::BinaryOutputArchive oar(ofs);

        // write the content of the cartridge header file first, this will be used to check
        // if a save state is compatible with the currently loaded cartridge
        oar(cartridge.header.asArray());

        oar(cpu);
        oar(wram);
        oar(ppu);
        oar(dma);
        oar(cartridge);
        oar(timer);
        oar(joypad);
        oar(audio);
        oar(serial);
        oar(hiRam);
    }

    return SaveStateError::NoError;
}

SaveStateError GameBoyClassic::loadState(const fs::path& path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
        return SaveStateError::OpenFileError;

    {
        cereal::BinaryInputArchive iar(ifs);

        // first verify if the header of the currently loaded cartridge is compatible with
        // the one contained in the save state
        std::array<uint8_t, CartridgeHeader::headerSize> buf;
        iar(buf);

        if (cartridge.header != buf)
            return SaveStateError::CartridgeMismatch;

        iar(cpu);
        iar(wram);
        iar(ppu);
        iar(dma);
        iar(cartridge);
        iar(timer);
        iar(joypad);
        iar(audio);
        iar(serial);
        iar(hiRam);
    }

    return SaveStateError::NoError;
}




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
    case SaveStateError::OpenFileError: return "Can't open the file";
    case SaveStateError::CartridgeMismatch: return "The currently loaded cartridge header doesn't match the save state cartridge header";
    case SaveStateError::LoadingError: return "Loading error, maybe the save state file is corrupted?";
    case SaveStateError::SavingError: return "Saving error";
    default:
        assert(false);
        return "Unknown error";
    }
}




// ------------------------------------------------------------------------------------------------
// GameBoyIf
// ------------------------------------------------------------------------------------------------

const char* GameBoyIf::statusToStr(Status st)
{
    switch (st) {
    default:
    case Status::Stopped: return "Stopped";
    case Status::Paused: return "Paused";
    case Status::Running: return "Running";
    case Status::Stepping: return "Stepping";
    }
}


GameBoyIf::GameBoyIf()
    : cpu(*this)
    , wram()
    , ppu(*this)
    , dma(*this)
    , cartridge()
    , timer(*this)
    , joypad(*this)
    , apu()
    , serial(*this)
    , hiRam(mmap::hiram::start)
    , status(Status::Stopped)
    , mStepInstruction(false)
{}


EmulateRes GameBoyIf::emulate()
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
        if (dbg.breakOnLdbb && this->read8(cpu.regs.PC) == op::LD_B_B) {
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

void GameBoyIf::play()
{
    status = Status::Running;
}

void GameBoyIf::pause()
{
    status = Status::Paused;
}

void GameBoyIf::stop()
{
    // on stop we also reset the whole gameboy
    status = Status::Stopped;
    gbReset();
}

void GameBoyIf::step()
{
    status = Status::Stepping;
    mStepInstruction = true;
}

void GameBoyIf::stepReturn()
{
    // we can't step to a return instruction if the cpu
    // is not inside a function call
    if (cpu.callNesting() == 0)
        return;

    dbg.targetCallNesting = cpu.callNesting() - 1;
    dbg.breakOnRet = true;
    status = Status::Running;
}


CartridgeLoadingRes GameBoyIf::loadCartridge(const std::filesystem::path& path)
{
    // load cartridge from file

    auto res = cartridge.loadRomFile(path);

    if (res == CartridgeLoadingRes::Ok) {
        romFilePath = path;
        gbReset();
    }

    // also try to load debug symbols (if any)
    dbg.symTable->parseSymbolFile(path);

    return res;
}

CartridgeLoadingRes GameBoyIf::loadCartridge(const uint8_t* data, size_t size)
{
    // load cartridge from memory

    auto res = cartridge.loadRomData(data, size);

    if (res == CartridgeLoadingRes::Ok) {
        // loading from memory, we don't know where the rom file is
        romFilePath.clear();
        gbReset();
    }

    // we don't have debug symbols so clear the sym table
    dbg.symTable->reset();

    return res;
}






GameBoyClassic::GameBoyClassic()
    : mAddrMap(initAddressMap())
{
    wram.setIsCgb(false);

    // make sure to reset everything 
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
    apu.reset();
    serial.reset();
    hiRam.reset();

    dbg.updateInstructionToStr(*this);
}


AddressMap GameBoyClassic::initAddressMap()
{
    AddressMap map = {
        // memory -------------------------------------------------------------
        { mmap::rom::start, &cartridge },
        { mmap::rom::end, &cartridge },
        { mmap::vram::start, &ppu.vram },
        { mmap::vram::end, &ppu.vram },
        { mmap::external_ram::start, &cartridge },
        { mmap::external_ram::end, &cartridge },
        { mmap::wram::start, &wram },
        { mmap::wram::end, &wram },
        { mmap::echoram::start, &wram },
        { mmap::echoram::end, &wram },
        { mmap::oam::start, &ppu.oamRam },
        { mmap::oam::end, &ppu.oamRam },
        { mmap::prohibited::start, nullptr },
        { mmap::prohibited::end, nullptr },

        // control registers --------------------------------------------------
        { mmap::regs::joypad, &joypad },
        { mmap::regs::serial_data, &serial },
        { mmap::regs::serial_ctrl, &serial },
        { mmap::regs::serial_ctrl + 1, nullptr },
        { mmap::regs::timer::start, &timer },
        { mmap::regs::timer::end, &timer },
        { mmap::regs::timer::end + 1, nullptr },
        { mmap::regs::IF - 1, nullptr },
        { mmap::regs::IF, &cpu.irqs },
        { mmap::regs::audio::start, &apu },
        { mmap::regs::audio::end, &apu },
        { mmap::regs::lcd::start, &ppu },
        { mmap::regs::lcd::lyc, &ppu },
        { mmap::regs::lcd::dma, &dma },
        { mmap::regs::lcd::bgp, &ppu },
        { mmap::regs::lcd::end, &ppu },
        { mmap::regs::lcd::end + 1, nullptr },
        { mmap::hiram::start - 1, nullptr },
        { mmap::hiram::start, &hiRam },
        { mmap::hiram::end, &hiRam },
        { mmap::IE, &cpu.irqs },
    };

    return map;
}

uint8_t GameBoyClassic::read8(uint16_t addr) const
{
    auto it = mAddrMap.lower_bound(addr);
    if (it == mAddrMap.end())
        return 0xff;

    return it->second ? it->second->read8(addr) : 0xff;
}

void GameBoyClassic::write8(uint16_t addr, uint8_t val)
{
    auto it = mAddrMap.lower_bound(addr);
    if (it == mAddrMap.end())
        return;

    if (auto* writeObj = it->second; writeObj) {
        writeObj->write8(addr, val);
    }
}



GbStepRes GameBoyClassic::gbStep()
{
    ZoneScoped;

    auto cpuRes = cpu.step();

    dma.step(cpuRes.cycles);
    bool frameReady = ppu.step(cpuRes.cycles);
    timer.step(cpuRes.cycles, cpu.isStopped());
    serial.step(cpuRes.cycles);
    joypad.step(cpuRes.cycles);
    apu.step(cpuRes.cycles);

    if (status != Status::Running) {
        ZoneScoped;
        dbg.updateInstructionToStr(*this);
    }

    GbStepRes res;
    res.cpuRes = cpuRes;
    res.frameReady = frameReady;

    return res;
}




SaveStateError GameBoyClassic::saveState(const fs::path& path)
{
    std::ofstream ofs(path, std::ios::binary);
    if(!ofs)
        return SaveStateError::OpenFileError;

    try {
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
        oar(apu);
        oar(serial);
        oar(hiRam);
    }
    catch (const cereal::Exception& /*ex*/) {
        return SaveStateError::SavingError;
    }

    return SaveStateError::NoError;
}

SaveStateError GameBoyClassic::loadState(const fs::path& path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
        return SaveStateError::OpenFileError;

    try {
        cereal::BinaryInputArchive iar(ifs);

        // first verify if the header of the currently loaded cartridge is compatible with
        // the one contained in the save state
        std::array<uint8_t, CartridgeHeader::headerSize> buf;
        iar(buf);

        if (cartridge.header != buf)
            return SaveStateError::CartridgeMismatch;

        // read everything from the archive into a copy of the current state

        CPU tmpCpu = cpu;
        WorkRam tmpWram = wram;
        PPU tmpPpu = ppu;
        DMA tmpDma = dma;
        Cartridge tmpCart = cartridge;
        Timer tmpTimer = timer;
        Joypad tmpJoypad = joypad;
        APU tmpApu = apu;
        Serial tmpSerial = serial;
        HiRam tmphiRam = hiRam;

        iar(tmpCpu);
        iar(tmpWram);
        iar(tmpPpu);
        iar(tmpDma);
        iar(tmpCart);
        iar(tmpTimer);
        iar(tmpJoypad);
        iar(tmpApu);
        iar(tmpSerial);
        iar(tmphiRam);

        // if no execption got caught while deserializing we can copy back 
        // the new state into the gameboy
        cpu = tmpCpu;
        wram = tmpWram;
        ppu = tmpPpu;
        dma = tmpDma;
        cartridge = tmpCart;
        timer = tmpTimer;
        joypad = tmpJoypad;
        apu = tmpApu;
        serial = tmpSerial;
        hiRam = tmphiRam;
    }
    catch (const cereal::Exception& /*ex*/) {
        return SaveStateError::LoadingError;
    }

    return SaveStateError::NoError;
}




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
    : cpu(*this)
    , wram(mmap::wram::start)
    , ppu(*this)
    , dma(*this)
    , cartridge()
    , timer(*this)
    , joypad(*this)
    , apu()
    , serial()
    , hiRam(mmap::hiram::start)
    , status(Status::Stopped)
    , mStepInstruction(false)
{
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


uint8_t GameBoyClassic::read8(uint16_t addr) const
{
    // Memory -------------------------------------------------------------------------------------

    if (addr >= mmap::rom::start && addr <= mmap::rom::end) {
        return cartridge.read8(addr);
    }
    else if (addr >= mmap::vram::start && addr <= mmap::vram::end) {
        return ppu.vram.read8(addr);
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        return cartridge.read8(addr);
    }
    else if (addr >= mmap::wram::start && addr <= mmap::wram::end) {
        return wram.read8(addr);
    }
    else if (addr >= mmap::echoram::start && addr <= mmap::echoram::end) {
        return wram.read8(addr - (mmap::echoram::start - mmap::wram::start));
    }
    else if (addr >= mmap::oam::start && addr <= mmap::oam::end) {
        return ppu.oamRam.read8(addr);
    }
    // Control registers --------------------------------------------------------------------------
    else if (addr == mmap::regs::joypad) {
        return joypad.read();
    }
    else if (addr == mmap::regs::serial_data) {
        return serial.readData();
    }
    else if (addr == mmap::regs::serial_ctrl) {
        return serial.readCtrl();
    }
    else if (addr >= mmap::regs::timer::start && addr <= mmap::regs::timer::end) {
        if (addr == mmap::regs::timer::DIV) return timer.readDIV();
        else if (addr == mmap::regs::timer::TIMA) return timer.readTIMA();
        else if (addr == mmap::regs::timer::TMA) return timer.readTMA();
        else if (addr == mmap::regs::timer::TAC) return timer.readTAC();
    }
    else if (addr >= mmap::regs::audio::start && addr <= mmap::regs::audio::end) {
        return apu.read(addr);
    }
    else if (addr >= mmap::regs::lcd::start && addr <= mmap::regs::lcd::end) {
        if (addr == mmap::regs::lcd::lcdc) return ppu.readLCDC();
        else if (addr == mmap::regs::lcd::stat) return ppu.readSTAT();
        else if (addr == mmap::regs::lcd::scy) return ppu.readSCY();
        else if (addr == mmap::regs::lcd::scx) return ppu.readSCX();
        else if (addr == mmap::regs::lcd::ly) return ppu.readLY();
        else if (addr == mmap::regs::lcd::lyc) return ppu.readLYC();
        else if (addr == mmap::regs::lcd::dma) return dma.read();
        else if (addr == mmap::regs::lcd::bgp) return ppu.readBGP();
        else if (addr == mmap::regs::lcd::obp0) return ppu.readOBP0();
        else if (addr == mmap::regs::lcd::obp1) return ppu.readOBP1();
        else if (addr == mmap::regs::lcd::wy) return ppu.readWY();
        else if (addr == mmap::regs::lcd::wx) return ppu.readWX();
    }
    else if (addr == mmap::regs::IF) {
        return cpu.irqs.readIF();
    }
    else if (addr >= mmap::hiram::start && addr <= mmap::hiram::end) {
        return hiRam.read8(addr);
    }
    else if (addr == mmap::IE) {
        return cpu.irqs.readIE();
    }
    
    return 0xFF;
}

void GameBoyClassic::write8(uint16_t addr, uint8_t val)
{
    // Memory -------------------------------------------------------------------------------------
    if (addr >= mmap::rom::start && addr <= mmap::rom::end) {
        cartridge.write8(addr, val);
    }
    else if (addr >= mmap::vram::start && addr <= mmap::vram::end) {
        ppu.vram.write8(addr, val);
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        cartridge.write8(addr, val);
    }
    else if (addr >= mmap::wram::start && addr <= mmap::wram::end) {
        wram.write8(addr, val);
    }
    else if (addr >= mmap::echoram::start && addr <= mmap::echoram::end) {
        wram.write8(addr - (mmap::echoram::start - mmap::wram::start), val);
    }
    else if (addr >= mmap::oam::start && addr <= mmap::oam::end) {
        ppu.oamRam.write8(addr, val);
    }
    // Control registers --------------------------------------------------------------------------
    else if (addr == mmap::regs::joypad) {
        joypad.write(val);
    }
    else if (addr == mmap::regs::serial_data) {
        serial.writeData(val);
    }
    else if (addr == mmap::regs::serial_ctrl) {
        serial.writeCtrl(val);
    }
    else if (addr >= mmap::regs::timer::start && addr <= mmap::regs::timer::end) {
        if (addr == mmap::regs::timer::DIV) timer.writeDIV(val);
        else if (addr == mmap::regs::timer::TIMA) timer.writeTIMA(val);
        else if (addr == mmap::regs::timer::TMA) timer.writeTMA(val);
        else if (addr == mmap::regs::timer::TAC) timer.writeTAC(val);
    }
    else if (addr >= mmap::regs::audio::start && addr <= mmap::regs::audio::end) {
        return apu.write(addr, val);
    }
    else if (addr >= mmap::regs::lcd::start && addr <= mmap::regs::lcd::end) {
        if (addr == mmap::regs::lcd::lcdc) ppu.writeLCDC(val);
        else if (addr == mmap::regs::lcd::stat) ppu.writeSTAT(val);
        else if (addr == mmap::regs::lcd::scy) ppu.writeSCY(val);
        else if (addr == mmap::regs::lcd::scx) ppu.writeSCX(val);
        else if (addr == mmap::regs::lcd::ly) ppu.writeLY(val);
        else if (addr == mmap::regs::lcd::lyc) ppu.writeLYC(val);
        else if (addr == mmap::regs::lcd::dma) dma.write(val);
        else if (addr == mmap::regs::lcd::bgp) ppu.writeBGP(val);
        else if (addr == mmap::regs::lcd::obp0) ppu.writeOBP0(val);
        else if (addr == mmap::regs::lcd::obp1) ppu.writeOBP1(val);
        else if (addr == mmap::regs::lcd::wy) ppu.writeWY(val);
        else if (addr == mmap::regs::lcd::wx) ppu.writeWX(val);
    }
    else if (addr == mmap::regs::IF) {
        cpu.irqs.writeIF(val);
    }
    else if (addr >= mmap::hiram::start && addr <= mmap::hiram::end) {
        hiRam.write8(addr, val);
    }
    else if (addr == mmap::IE) {
        cpu.irqs.writeIE(val);
    }
}



GbStepRes GameBoyClassic::gbStep()
{
    ZoneScoped;

    auto cpuRes = cpu.step();

    dma.step(cpuRes.cycles);
    bool frameReady = ppu.step(cpuRes.cycles);
    timer.step(cpuRes.cycles, cpu.isStopped());
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


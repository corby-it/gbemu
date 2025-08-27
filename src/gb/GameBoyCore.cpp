

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



const char* gbTypeToStr(GbType type)
{
    switch (type) {
    case GbType::DMG: return "DMG";
    case GbType::CGB: return "CGB";
    default:
        return "unknown type";
    }
}

const char* saveStateErrorToStr(SaveStateError err)
{
    switch (err) {
    case SaveStateError::NoError: return "No error";
    case SaveStateError::OpenFileError: return "Can't open the file";
    case SaveStateError::HardwareMismatch: return "The save state hardware type doesn't match the currently emulated hardware type";
    case SaveStateError::CartridgeMismatch: return "The currently loaded cartridge header doesn't match the save state cartridge header";
    case SaveStateError::LoadingError: return "Loading error, maybe the save state file is corrupted?";
    case SaveStateError::SavingError: return "Saving error";
    default:
        assert(false);
        return "Unknown error";
    }
}




// ------------------------------------------------------------------------------------------------
// GameBoy
// ------------------------------------------------------------------------------------------------

const char* GameBoy::statusToStr(Status st)
{
    switch (st) {
    default:
    case Status::Stopped: return "Stopped";
    case Status::Paused: return "Paused";
    case Status::Running: return "Running";
    case Status::Stepping: return "Stepping";
    }
}


GameBoy::GameBoy()
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
    , mAddrMap(initAddressMap())
    , mType(GbType::DMG)
    , mStepInstruction(false)
{
    cpu.setIsCgb(false);
    wram.setIsCgb(false);
    ppu.setIsCgb(false);
    apu.setIsCgb(false);
    infrared.setIsCgb(false);
    undocRegs.setIsCgb(false);

    gbReset();
}

AddressMap GameBoy::initAddressMap()
{
    // highlighted by comments are registers specific to the CGB

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
        { mmap::regs::vbk, &ppu.vram },     // VBK
        { mmap::regs::boot, nullptr },      // BOOT
        { mmap::regs::hdma::start, &ppu.hdma },   // HDMA
        { mmap::regs::hdma::end, &ppu.hdma },     // HDMA
        { mmap::regs::infrared, &infrared },    // Infrared
        { mmap::regs::col_palette::start - 1, nullptr },     // Color palette
        { mmap::regs::col_palette::start, &ppu.colors },    // Color palette
        { mmap::regs::col_palette::end, &ppu.colors },      // Color palette
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

void GameBoy::gbReset()
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
    infrared.reset();
    undocRegs.reset();

    dbg.updateInstructionToStr(*this);
}

const GBTimingInfo& GameBoy::getCurrTimingInfo() const
{
    static const GBTimingInfo baseSpeed = {
        clockFreq,
        machineFreq,
        clockPeriod,
        machinePeriod,
    };
    static const GBTimingInfo doubleSpeed = {
        clockFreq * 2,
        machineFreq * 2,
        clockPeriod / 2,
        machinePeriod / 2,
    };

    if (cpu.key1.doubleSpeed)
        return doubleSpeed;
    else
        return baseSpeed;
}


void GameBoy::setType(GbType type)
{
    if (type == mType)
        return;

    mType = type;

    bool isCgb = type == GbType::CGB;

    cpu.setIsCgb(isCgb);
    wram.setIsCgb(isCgb);
    ppu.setIsCgb(isCgb);
    apu.setIsCgb(isCgb);
    infrared.setIsCgb(isCgb);
    undocRegs.setIsCgb(isCgb);
    
    // make sure to reset everything 
    gbReset();
}


EmulateRes GameBoy::emulate()
{
    ZoneScoped;

    // return true if there is more emulation work to be done (that is, if
    // the emulation is not stopped or paused)

    EmulateRes res;
    res.stillGoing = false;

    switch (status) {
    default:
    case Status::Stopped:
    case Status::Paused: {
        // if the emulation is stopped or paused there is nothing to do
        break;
    }

    case Status::Running:
    {
        // emulate the gameboy, full speed
        res.stepRes = gbStep();
        res.stillGoing = true;
        break;
    }

    case Status::Stepping: {
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

void GameBoy::play()
{
    status = Status::Running;
}

void GameBoy::pause()
{
    status = Status::Paused;
}

void GameBoy::stop()
{
    // on stop we also reset the whole gameboy
    status = Status::Stopped;
    gbReset();
}

void GameBoy::step()
{
    status = Status::Stepping;
    mStepInstruction = true;
}

void GameBoy::stepReturn()
{
    // we can't step to a return instruction if the cpu
    // is not inside a function call
    if (cpu.callNesting() == 0)
        return;

    dbg.targetCallNesting = cpu.callNesting() - 1;
    dbg.breakOnRet = true;
    status = Status::Running;
}

uint8_t GameBoy::read8(uint16_t addr) const
{
    auto it = mAddrMap.lower_bound(addr);
    if (it == mAddrMap.end())
        return 0xff;

    return it->second ? it->second->read8(addr) : 0xff;
}

void GameBoy::write8(uint16_t addr, uint8_t val)
{
    auto it = mAddrMap.lower_bound(addr);
    if (it == mAddrMap.end())
        return;

    if (auto* writeObj = it->second; writeObj) {
        writeObj->write8(addr, val);
    }
}


CartridgeLoadingRes GameBoy::loadCartridge(const std::filesystem::path& path)
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

CartridgeLoadingRes GameBoy::loadCartridge(const uint8_t* data, size_t size)
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



SaveStateError GameBoy::saveState(const fs::path& path)
{
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs)
        return SaveStateError::OpenFileError;

    try {
        cereal::BinaryOutputArchive oar(ofs);

        // first write the hardware type
        oar(static_cast<uint8_t>(mType));

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
        oar(infrared);
        oar(undocRegs);
    }
    catch (const cereal::Exception& /*ex*/) {
        return SaveStateError::SavingError;
    }

    return SaveStateError::NoError;
}

SaveStateError GameBoy::loadState(const fs::path& path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
        return SaveStateError::OpenFileError;

    try {
        cereal::BinaryInputArchive iar(ifs);

        uint8_t hwTypeU8;
        iar(hwTypeU8);

        auto hwType = static_cast<GbType>(hwTypeU8);
        if (hwType != mType)
            return SaveStateError::HardwareMismatch;

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
        HiRam tmpHiRam = hiRam;
        Infrared tmpInfrared = infrared;
        UndocumentedRegs tmpUndocRegs = undocRegs;

        iar(tmpCpu);
        iar(tmpWram);
        iar(tmpPpu);
        iar(tmpDma);
        iar(tmpCart);
        iar(tmpTimer);
        iar(tmpJoypad);
        iar(tmpApu);
        iar(tmpSerial);
        iar(tmpHiRam);
        iar(tmpInfrared);
        iar(tmpUndocRegs);

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
        hiRam = tmpHiRam;
        infrared = tmpInfrared;
        undocRegs = tmpUndocRegs;
    }
    catch (const cereal::Exception& /*ex*/) {
        return SaveStateError::LoadingError;
    }

    return SaveStateError::NoError;
}



GbStepRes GameBoy::gbStep()
{
    ZoneScoped;

    auto cpuRes = cpu.step();

    bool frameReady = false;

    if (!cpu.isStopped()) {

        if (cpu.key1.doubleSpeed) {
            // when the cpu is running at double speed, everything is running at
            // double clock speed as well, except for the PPU and the APU
            static bool runPpuApu = false;

            if (runPpuApu) {
                frameReady = ppu.step(cpuRes.cycles);
                apu.step(cpuRes.cycles);
            }

            dma.step(cpuRes.cycles);
            timer.step(cpuRes.cycles, cpu.isStopped());
            serial.step(cpuRes.cycles);
            joypad.step(cpuRes.cycles);

            runPpuApu = !runPpuApu;
        }
        else {
            dma.step(cpuRes.cycles);
            frameReady = ppu.step(cpuRes.cycles);
            timer.step(cpuRes.cycles, cpu.isStopped());
            serial.step(cpuRes.cycles);
            joypad.step(cpuRes.cycles);
            apu.step(cpuRes.cycles);
        }

        // handle bus events (if any)
        for (; !mEvtQueue.empty(); mEvtQueue.pop()) {

            switch (mEvtQueue.front()) {
            // when the cpu executes a HALT instruction, a running hdma hblank 
            // transfer must be paused and will resume only when the cpu resumes
            // from the halt state
            case BusEvent::CpuExecHalt: ppu.hdma.pauseOnCpuHalt(); break;
            case BusEvent::CpuResumesFromHalt: ppu.hdma.resumeOnCpuHalt(); break;

            // when an HDMA transfer starts the cpu is halted and it automatically
            // returns to its normal state when the transfer is over
            case BusEvent::HdmaStarted: cpu.halt(true); break;
            case BusEvent::HdmaStopped: cpu.halt(false); break;
            default:
                break;
            }

        }
    }

    if (status != Status::Running) {
        ZoneScoped;
        dbg.updateInstructionToStr(*this);
    }

    GbStepRes res;
    res.cpuRes = cpuRes;
    res.frameReady = frameReady;

    return res;
}



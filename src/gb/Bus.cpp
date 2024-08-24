

#include "Bus.h"
#include "Cpu.h"
#include "Timer.h"
#include "Joypad.h"
#include "Dma.h"
#include "Ppu.h"
#include "Audio.h"
#include "Serial.h"
#include "Cartridge.h"
#include "GbCommons.h"
#include <cassert>


// ------------------------------------------------------------------------------------------------
// Bus
// ------------------------------------------------------------------------------------------------

uint16_t Bus::read16(uint16_t addr) const
{
    assert(addr <= UINT16_MAX - 1);
    // the GB is little endian so:
    // - the byte at [addr] is the lsb
    // - the byte at [addr+1] is the msb
    return read8(addr) | (read8(addr + 1) << 8);
}


void Bus::write16(uint16_t addr, uint16_t val)
{
    assert(addr <= UINT16_MAX - 1);
    // the GB is little endian so:
    // - the lsb must be written at [addr]
    // - the msb must be written at [addr+1]
    write8(addr, (uint8_t)val);
    write8(addr + 1, val >> 8);
}




// ------------------------------------------------------------------------------------------------
// GBBus
// ------------------------------------------------------------------------------------------------

GBBus::GBBus()
    : mCpu(nullptr)
    , mWram(nullptr)
    , mPpu(nullptr)
    , mDma(nullptr)
    , mCartridge(nullptr)
    , mTimer(nullptr)
    , mJoypad(nullptr)
    , mAudio(nullptr)
    , mSerial(nullptr)
    , mHiRam(nullptr)
    , mReadFnPtr(&GBBus::dummyRead)
    , mWriteFnPtr(&GBBus::dummyWrite)
{}

void GBBus::switchRWFunctions()
{
    if (mCpu && mWram && mPpu && mDma && mCartridge && mTimer
        && mJoypad && mAudio && mSerial && mHiRam) {

        mReadFnPtr = &GBBus::realRead;
        mWriteFnPtr = &GBBus::realWrite;
    }
    else {
        mReadFnPtr = &GBBus::dummyRead;
        mWriteFnPtr = &GBBus::dummyWrite;
    }
}


uint8_t GBBus::realRead(uint16_t addr) const
{
    // Memory -------------------------------------------------------------------------------------

    if (addr >= mmap::rom::start && addr <= mmap::rom::end)
        return mCartridge->read8(addr);

    if (addr >= mmap::vram::start && addr <= mmap::vram::end)
        return mPpu->vram.read8(addr);

    if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end)
        return mCartridge->read8(addr);

    if (addr >= mmap::wram::start && addr <= mmap::wram::end)
        return mWram->read8(addr);

    if (addr >= mmap::echoram::start && addr <= mmap::echoram::end)
        return mWram->read8(addr - (mmap::echoram::start - mmap::wram::start));

    if (addr >= mmap::oam::start && addr <= mmap::oam::end)
        return mPpu->oamRam.read8(addr);

    // Control registers --------------------------------------------------------------------------

    if (addr == mmap::regs::joypad)
        return mJoypad->read();

    if (addr == mmap::regs::serial_data)
        return mSerial->readData();
    if (addr == mmap::regs::serial_ctrl)
        return mSerial->readCtrl();

    if (addr >= mmap::regs::timer::start && addr <= mmap::regs::timer::end) {
        if (addr == mmap::regs::timer::DIV) return mTimer->readDIV();
        if (addr == mmap::regs::timer::TIMA) return mTimer->readTIMA();
        if (addr == mmap::regs::timer::TMA) return mTimer->readTMA();
        if (addr == mmap::regs::timer::TAC) return mTimer->readTAC();
    }

    if (addr >= mmap::regs::audio::start && addr <= mmap::regs::audio::end) {
        return mAudio->read(addr);
    }

    if (addr >= mmap::regs::lcd::start && addr <= mmap::regs::lcd::end) {
        if (addr == mmap::regs::lcd::lcdc) return mPpu->readLCDC();
        if (addr == mmap::regs::lcd::stat) return mPpu->readSTAT();
        if (addr == mmap::regs::lcd::scy) return mPpu->readSCY();
        if (addr == mmap::regs::lcd::scx) return mPpu->readSCX();
        if (addr == mmap::regs::lcd::ly) return mPpu->readLY();
        if (addr == mmap::regs::lcd::lyc) return mPpu->readLYC();
        if (addr == mmap::regs::lcd::dma) return mDma->read();
        if (addr == mmap::regs::lcd::bgp) return mPpu->readBGP();
        if (addr == mmap::regs::lcd::obp0) return mPpu->readOBP0();
        if (addr == mmap::regs::lcd::obp1) return mPpu->readOBP1();
        if (addr == mmap::regs::lcd::wy) return mPpu->readWY();
        if (addr == mmap::regs::lcd::wx) return mPpu->readWX();
    }

    if (addr == mmap::regs::IF)
        return mCpu->irqs.readIF();

    if (addr >= mmap::hiram::start && addr <= mmap::hiram::end)
        return mHiRam->read8(addr);

    if (addr == mmap::IE)
        return mCpu->irqs.readIE();
    
    return 0xFF;
}

void GBBus::realWrite(uint16_t addr, uint8_t val)
{
    // Memory -------------------------------------------------------------------------------------
    if (addr >= mmap::rom::start && addr <= mmap::rom::end) {
        mCartridge->write8(addr, val);
    }
    else if (addr >= mmap::vram::start && addr <= mmap::vram::end) {
        mPpu->vram.write8(addr, val);
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        mCartridge->write8(addr, val);
    }
    else if (addr >= mmap::wram::start && addr <= mmap::wram::end) {
        mWram->write8(addr, val);
    }
    else if (addr >= mmap::echoram::start && addr <= mmap::echoram::end) {
        mWram->write8(addr - (mmap::echoram::start - mmap::wram::start), val);
    }
    else if (addr >= mmap::oam::start && addr <= mmap::oam::end) {
        mPpu->oamRam.write8(addr, val);
    }
    // Control registers --------------------------------------------------------------------------
    else if (addr == mmap::regs::joypad) {
        mJoypad->write(val);
    }
    else if (addr == mmap::regs::serial_data) {
        mSerial->writeData(val);
    }
    else if (addr == mmap::regs::serial_ctrl) {
        mSerial->writeCtrl(val);
    }
    else if (addr >= mmap::regs::timer::start && addr <= mmap::regs::timer::end) {
        if (addr == mmap::regs::timer::DIV) mTimer->writeDIV(val);
        else if (addr == mmap::regs::timer::TIMA) mTimer->writeTIMA(val);
        else if (addr == mmap::regs::timer::TMA) mTimer->writeTMA(val);
        else if (addr == mmap::regs::timer::TAC) mTimer->writeTAC(val);
    }
    else if (addr >= mmap::regs::audio::start && addr <= mmap::regs::audio::end) {
        return mAudio->write(addr, val);
    }
    else if (addr >= mmap::regs::lcd::start && addr <= mmap::regs::lcd::end) {
        if (addr == mmap::regs::lcd::lcdc) mPpu->writeLCDC(val);
        else if (addr == mmap::regs::lcd::stat) mPpu->writeSTAT(val);
        else if (addr == mmap::regs::lcd::scy) mPpu->writeSCY(val);
        else if (addr == mmap::regs::lcd::scx) mPpu->writeSCX(val);
        else if (addr == mmap::regs::lcd::ly) mPpu->writeLY(val);
        else if (addr == mmap::regs::lcd::lyc) mPpu->writeLYC(val);
        else if (addr == mmap::regs::lcd::dma) mDma->write(val);
        else if (addr == mmap::regs::lcd::bgp) mPpu->writeBGP(val);
        else if (addr == mmap::regs::lcd::obp0) mPpu->writeOBP0(val);
        else if (addr == mmap::regs::lcd::obp1) mPpu->writeOBP1(val);
        else if (addr == mmap::regs::lcd::wy) mPpu->writeWY(val);
        else if (addr == mmap::regs::lcd::wx) mPpu->writeWX(val);
    }
    else if (addr == mmap::regs::IF) {
        mCpu->irqs.writeIF(val);
    }
    else if (addr >= mmap::hiram::start && addr <= mmap::hiram::end) {
        mHiRam->write8(addr, val);
    }
    else if (addr == mmap::IE) {
        mCpu->irqs.writeIE(val);
    }
}

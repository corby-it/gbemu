
#include "Mbc.h"
#include "GbCommons.h"




// ------------------------------------------------------------------------------------------------
// MbcInterface
// ------------------------------------------------------------------------------------------------

MbcInterface::MbcInterface(MbcType type, const std::vector<uint8_t>& rom, std::vector<uint8_t>& ram)
    : mType(type)
    , mRom(rom)
    , mRam(ram)
    , mRomBanksCount(uint8_t(rom.size() / romBankSize))
    , mRamBanksCount(uint8_t(ram.size() / ramBankSize))
    , mRomCurrBank(0)
    , mRamCurrBank(0)
{}

void MbcInterface::reset()
{
    mRomCurrBank = 0;
    mRamCurrBank = 0;

    onReset();
}




// ------------------------------------------------------------------------------------------------
// MbcNone
// ------------------------------------------------------------------------------------------------

MbcNone::MbcNone(const std::vector<uint8_t>& rom, std::vector<uint8_t>& ram)
    : MbcInterface(MbcType::None, rom, ram)
{}

uint8_t MbcNone::read8(uint16_t addr) const
{
    if (addr > mmap::rom::end)
        return 0xFF;

    return mRom[addr];
}

void MbcNone::write8(uint16_t /*addr*/, uint8_t /*val*/)
{
    // with no MBC there are no register to write to
    // so writes have no effect
}

void MbcNone::onReset()
{
    // when there is no MBC the current rom bank is always 1
    mRomCurrBank = 1;
}




// ------------------------------------------------------------------------------------------------
// Mbc1
// ------------------------------------------------------------------------------------------------

// see https://gbdev.io/pandocs/MBC1.html for a detailed explanation of how this works

uint8_t Mbc1::computeRomBankLowMask(uint8_t romBanksCount)
{
    if (romBanksCount <= 2)
        return 0x01;
    else if (romBanksCount > 2 && romBanksCount <= 4)
        return 0x03;
    else if (romBanksCount > 4 && romBanksCount <= 8)
        return 0x07;
    else if (romBanksCount > 8 && romBanksCount <= 16)
        return 0x0F;
    else
        return 0x1F;
}

Mbc1::Mbc1(const std::vector<uint8_t>& rom, std::vector<uint8_t>& ram, bool hasRam)
    : MbcInterface(MbcType::Mbc1, rom, ram)
    , mHasRam(hasRam)
    , mRomBankLowMask(computeRomBankLowMask(mRomBanksCount))
{
    onReset();
}

void Mbc1::onReset()
{
    mRamEnabled = false;
    mAddrMode1 = false;
    mRomBankLow = 1;
    mRomBankHigh = 0;

    updateBankConfiguration();
}


uint8_t Mbc1::read8(uint16_t addr) const
{
    if (addr <= mmap::rom::end) {
        // read from rom
        // rom banks are 16KB in size so we must discard the upper 2 bits from the address
        
        uint32_t romAddr = 0;

        if (addr <= mmap::rom::bank0::end) {
            addr &= 0x3FFF;
            romAddr = mRomCurrBankLow * romBankSize + addr;
        }
        else {
            addr &= 0x3FFF;
            romAddr = mRomCurrBank * romBankSize + addr;
        }

        return mRom[romAddr];
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        // read from ram
        if(!mHasRam || !mRamEnabled)
            return 0xFF;

        // ram banks are 8KB in size so from the received address we discard the upper 3 bits
        addr &= 0x1FFF;
        uint32_t ramAddr = mRamCurrBank * ramBankSize + addr;
        
        return mRam[ramAddr];
    }
    else {
        // wrong address
        assert(false);
        return 0xFF;
    }
}

void Mbc1::write8(uint16_t addr, uint8_t val)
{
    if (addr <= 0x1FFF) {
        // ram enable reg - 4 bits
        
        // writing anywhere in this range enables or disables ram, 
        // writing a value with 0xA in the lowest 4 bits enable ram, any other value disables it
        if ((val & 0x0F) == 0x0A)
            mRamEnabled = true;
        else 
            mRamEnabled = false;

        updateBankConfiguration();
    }
    else if (addr >= 0x2000 && addr <= 0x3FFF) {
        // rom bank code low - 5 bits

        // the value written here selects the rom bank number that will be connected to the 4000-7FFF region
        // top 3 bits are discarded
        // with 5 bits it's possible to address 32 banks, if the cartridge has less than 32 banks the top bits
        // will be masked to make it impossible to select an out-of-range bank
        
        // a value of 0 is not allowed in this register and writing a 0 will cause the mbc to increment it by 1
        // this means that rom banks 20, 40, 60 are not accessible
        // they become accessible when in addressing mode 1 through the 0000-3FFF region

        mRomBankLow = val & 0x1F;
        if (mRomBankLow == 0)
            mRomBankLow++;

        updateBankConfiguration();
    }
    else if (addr >= 0x4000 && addr <= 0x5FFF) {
        // rom bank code high or ram bank code - 2 bits

        // depending on the selected addressing mode the value written in this register may
        // either be used as additional 2 bits for the rom bank number (bits 5 and 6) 
        // or as the ram bank number

        // top bits are discarded

        mRomBankHigh = val & 0x03;

        updateBankConfiguration();
    }
    else if (addr >= 0x6000 && addr <= 0x7FFF) {
        // addressing mode
        // writing 1 in the lower bit of this register enables the "advanced addressing" mode

        mAddrMode1 = val & 0x01;

        updateBankConfiguration();
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        // write to ram
        if (!mHasRam || !mRamEnabled)
            return;
        
        // ram banks are 8KB in size so from the received address we discard the upper 3 bits
        addr &= 0x1FFF;
        uint32_t ramAddr = mRamCurrBank * ramBankSize + addr;

        mRam[ramAddr] = val;
    }
}


void Mbc1::updateBankConfiguration()
{
    if (mAddrMode1) {
        // ram bank is selected using the value in the upper 2 bits of the rom bank
        mRamCurrBank = mRomBankHigh;

        // rom bank in the 4000-7FFF range is selected using both rom bank low and rom bank high 
        // the value is then masked depending on the number of available banks (in the actual hardware
        // the upper pins would not be connected in case of smaller roms)
        mRomCurrBank = ((mRomBankLow & mRomBankLowMask) | (mRomBankHigh << 5));

        // rom bank in the 0000-3FFF range is selected using rom bank high shifted by 5
        mRomCurrBankLow = mRomBankHigh << 5;
    }
    else { 
        // ram bank is always 0
        mRamCurrBank = 0;

        // same as in mode 1
        mRomCurrBank = ((mRomBankLow & mRomBankLowMask) | (mRomBankHigh << 5));

        // always maps the first bank
        mRomCurrBankLow = 0;
    }
}

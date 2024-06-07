
#include "Mbc.h"
#include "GbCommons.h"


static constexpr uint8_t bitmask(uint8_t nbits)
{
    return (1 << nbits) - 1;
}

static constexpr uint8_t bankMask(uint8_t maxbits, uint8_t nbanks)
{
    // assume that nbanks is a power of 2
    return bitmask(maxbits) & (nbanks - 1);
}





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


Mbc1::Mbc1(const std::vector<uint8_t>& rom, std::vector<uint8_t>& ram, bool hasRam)
    : MbcInterface(MbcType::Mbc1, rom, ram)
    , mHasRam(hasRam)
    , mRomBankLowMask(bankMask(5, mRomBanksCount))
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
        // writing a value with 0xA in the lowest 4 bits enables ram, any other value disables it
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




// ------------------------------------------------------------------------------------------------
// Mbc3
// ------------------------------------------------------------------------------------------------

Mbc3::Mbc3(const std::vector<uint8_t>& rom, std::vector<uint8_t>& ram, bool hasRam)
    : MbcInterface(MbcType::Mbc3, rom, ram)
    , mHasRam(hasRam)
    , mRomBankMask(bankMask(7, mRomBanksCount))
    , mRamBankMask(bankMask(2, mRamBanksCount))
{
    onReset();
}

void Mbc3::onReset()
{
    // init the rtc latch reg to 1 so that it's necessary to write 0 and then 1
    // to actually latch the rtc
    mRtcLatchReg = 1;
    mRamRtcEnabled = false;
}

uint8_t Mbc3::read8(uint16_t addr) const
{
    if (addr <= mmap::rom::end) {
        // read from rom
        // rom banks are 16KB in size so we must discard the upper 2 bits from the address
        
        uint32_t romAddr = 0;

        if (addr <= mmap::rom::bank0::end) {
            // the first 16K addresses are always mapped to bank 0
            addr &= 0x3FFF;
            romAddr = addr;
        }
        else {
            addr &= 0x3FFF;
            romAddr = mRomCurrBank * romBankSize + addr;
        }

        return mRom[romAddr];
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        // read from ram or rtc
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

void Mbc3::write8(uint16_t addr, uint8_t val)
{
    if (addr <= 0x1FFF) {
        // ram and rtc enable reg - 4 bits
        
        // writing anywhere in this range enables or disables the ram and the rtc at the same time
        // writing a value with 0xA in the lowest 4 bits enables them, any other value disables it
        if ((val & 0x0F) == 0x0A)
            mRamRtcEnabled = true;
        else 
            mRamRtcEnabled = false;
    }
    else if (addr >= 0x2000 && addr <= 0x3FFF) {
        // rom bank number - 7 bits

        // the value written here selects the rom bank number that will be connected to the 4000-7FFF region
        // top bit is discarded
        // with 7 bits it's possible to address 128 banks, if the cartridge has less than 128 banks the top bits
        // will be masked to make it impossible to select an out-of-range bank
        
        // a value of 0 is not allowed in this register and writing a 0 will cause the mbc to increment it by 1
        
        mRomCurrBank = val & mRomBankMask;
        if (mRomCurrBank == 0)
            mRomCurrBank++;
    }
    else if (addr >= 0x4000 && addr <= 0x5FFF) {
        // ram bank number or RTC register

        // writing a value in range for $00-$03 maps the corresponding external RAM Bank (if any) into memory at A000-BFFF.
        // When writing a value of $08-$0C, this will map the corresponding RTC register into memory at A000-BFFF

        // top bits are discarded
        if(val < 0x04)
            val &= mRamBankMask;

        mRamCurrBank = val & 0x0F;
    }
    else if (addr >= 0x6000 && addr <= 0x7FFF) {
        // latch clock data
        // When writing $00, and then $01 to this register, the current time becomes latched into the RTC registers.
        // The latched data will not change until it becomes latched again, by repeating the write $00->$01 procedure.
        // This provides a way to read the RTC registers while the clock keeps ticking.

        if(mRtcLatchReg == 0 && val == 1) {
            mRtcLatchReg = 1;
            mRtc.latch();
        }
        else if(val == 0) {
            mRtcLatchReg = 0;
        }
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        if(!mRamRtcEnabled)
            return;

        switch(mRamCurrBank) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03: {
                // try writing to RAM
                // ram banks are 8KB in size so from the received address we discard the upper 3 bits
                if(mHasRam) {
                    addr &= 0x1FFF;
                    uint32_t ramAddr = mRamCurrBank * ramBankSize + addr;

                    mRam[ramAddr] = val;
                }
                break;
            }
            case 0x08: mRtc.writeSec(val); break;
            case 0x09: mRtc.writeMin(val); break;
            case 0x0A: mRtc.writeHours(val); break;
            case 0x0B: mRtc.writeDaysL(val); break;
            case 0x0C: mRtc.writeDaysH(val); break;
        }
    }
}

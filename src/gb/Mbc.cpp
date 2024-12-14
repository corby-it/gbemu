
#include "Mbc.h"
#include "GbCommons.h"


static constexpr uint16_t bitmask(uint8_t nbits)
{
    return (1 << nbits) - 1;
}

static constexpr uint16_t bankMask(uint8_t maxbits, uint16_t nbanks)
{
    if (nbanks == 0)
        return 0;

    // assume that nbanks is a power of 2
    return bitmask(maxbits) & (nbanks - 1);
}





// ------------------------------------------------------------------------------------------------
// MbcInterface
// ------------------------------------------------------------------------------------------------

MbcInterface::MbcInterface(MbcType type, size_t romSize, size_t ramSize)
    : rom(romSize, 0)
    , ram(ramSize, 0)
    , mType(type)
    , mRomBanksCount(uint16_t(rom.size() / romBankSize))
    , mRamBanksCount(uint16_t(ram.size() / ramBankSize))
    , mRomCurrBank(0)
    , mRamCurrBank(0)
{}

void MbcInterface::reset()
{
    mRomCurrBank = 0;
    mRamCurrBank = 0;

    std::fill(ram.begin(), ram.end(), 0);

    onReset();
}




// ------------------------------------------------------------------------------------------------
// MbcNone
// ------------------------------------------------------------------------------------------------

MbcNone::MbcNone(size_t romSize, size_t ramSize)
    : MbcInterface(MbcType::None, romSize, ramSize)
{}

uint8_t MbcNone::read8(uint16_t addr) const
{
    if (addr > mmap::rom::end)
        return 0xFF;

    return rom[addr];
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


Mbc1::Mbc1(size_t romSize, size_t ramSize)
    : MbcInterface(MbcType::Mbc1, romSize, ramSize)
    , mRamEnabled(false)
    , mAddrMode1(false)
    , mRomMask(bankMask(7, mRomBanksCount))
    , mRamMask(bankMask(2, mRamBanksCount))
    , mRomBankLow(1)
    , mRomBankHigh(0)
{
    updateBankConfiguration();
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

        return rom[romAddr];
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        // read from ram
        if(ram.size() == 0 || !mRamEnabled)
            return 0xFF;

        // ram banks are 8KB in size so from the received address we discard the upper 3 bits
        addr &= 0x1FFF;
        uint32_t ramAddr = mRamCurrBank * ramBankSize + addr;
        
        return ram[ramAddr];
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
        if (ram.size() == 0 || !mRamEnabled)
            return;
        
        // ram banks are 8KB in size so from the received address we discard the upper 3 bits
        addr &= 0x1FFF;
        uint32_t ramAddr = mRamCurrBank * ramBankSize + addr;

        ram[ramAddr] = val;
    }
}


void Mbc1::updateBankConfiguration()
{
    if (mAddrMode1) {
        // ram bank is selected using the value in the upper 2 bits of the rom bank
        mRamCurrBank = mRomBankHigh & mRamMask;

        // rom bank in the 4000-7FFF range is selected using both rom bank low and rom bank high 
        // the value is then masked depending on the number of available banks (in the actual hardware
        // the upper pins would not be connected in case of smaller roms)
        mRomCurrBank = (mRomBankHigh << 5) + mRomBankLow;
        mRomCurrBank &= mRomMask;

        // rom bank in the 0000-3FFF range is selected using rom bank high shifted by 5
        mRomCurrBankLow = (mRomBankHigh << 5) & mRomMask;
    }
    else { 
        // ram bank is always 0
        mRamCurrBank = 0;

        // same as in mode 1
        mRomCurrBank = (mRomBankHigh << 5) + mRomBankLow;
        mRomCurrBank &= mRomMask;

        // always maps the first bank
        mRomCurrBankLow = 0;
    }
}




// ------------------------------------------------------------------------------------------------
// Mbc2
// ------------------------------------------------------------------------------------------------

Mbc2::Mbc2(size_t romSize, size_t ramSize)
    : MbcInterface(MbcType::Mbc2, romSize, ramSize)
    , mRamEnabled(false)
    , mRomMask(bankMask(4, mRomBanksCount))
{
    // mbc2 always has 512 half-bytes of ram
    ram.resize(512);

    mRomCurrBank = 1;
}

void Mbc2::onReset()
{
    mRamEnabled = false;
    mRomCurrBank = 1;
}

uint8_t Mbc2::read8(uint16_t addr) const
{
    if (addr <= mmap::rom::bank0::end) {
        // always read from bank 0
        return rom[addr];
    }
    else if (addr >= mmap::rom::bankN::start && addr <= mmap::rom::bankN::end) {
        // read from the selected rom bank
        // discard top 2 bits from the address

        uint32_t romAddr = mRomCurrBank * romBankSize + (addr & 0x3FFF);
        return rom[romAddr];
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        // read from ram 
        if (!mRamEnabled)
            return 0xFF;

        // only the bottom 9 bits of the address are used to index into the internal ram,
        // so ram access repeats after the first 512 half bytes
        // the upper 4 bits are undefined, in this case we set them to F.

        uint8_t val = ram[addr & 0x1FF];
        return val | 0xF0;
    }

    // should never get to this point
    assert(false);
    return 0xFF;
}

void Mbc2::write8(uint16_t addr, uint8_t val)
{
    if (addr <= mmap::rom::bank0::end) {
        // both mbc2 registers are accessible through the same address range
        // as the first rom bank, it's possible to choose between them using bit 8 of the address

        if (addr & 0x0100) {
            // bit 8 is set, set rom bank register value for address range 0x4000 - 0x7FFF
            // only lower 4 bits are used to determine the rom bank (if the value is 0 it becomes 1)

            // it's not possible to write the value 0 directly to this register, if it happens, 
            // the value turns into a 1. 
            // Anyway, it's possible to access bank 0 from the 0x4000 - 0x7FFF range, it's possible when 
            // the value written is not exactly 0 but the upper bits are ignored
            // for example, if the chip has only 8 banks of rom, bit 3 is ignored 
            val &= 0x0F;
            if (val == 0)
                ++val;
                
            mRomCurrBank = val & mRomMask;
        }
        else {
            // bit 8 is not set, enable or disable ram
            // ram is enabled by writing a value with 0xA in the lower nibble, any other value disables ram
            mRamEnabled = (val & 0x0F) == 0x0A;
        }
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        if (!mRamEnabled)
            return;

        // as with reads, only the bottom 9 bits of the address are used to index into the internal ram,
        // so ram access repeats after the first 512 half bytes
        
        ram[addr & 0x1FF] = val;
    }
}





// ------------------------------------------------------------------------------------------------
// Mbc3
// ------------------------------------------------------------------------------------------------

Mbc3::Mbc3(size_t romSize, size_t ramSize)
    : MbcInterface(MbcType::Mbc3, romSize, ramSize)
    , mRomMask(bankMask(7, mRomBanksCount))
    , mRamMask(bankMask(2, mRamBanksCount))
    , mRtcLatchReg(1)
    , mRamRtcEnabled(false)
{}

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

        return rom[romAddr];
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        // read from ram or rtc
        
        switch (mRamCurrBank) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03: {
            // try reading to RAM
            // ram banks are 8KB in size so from the received address we discard the upper 3 bits
            if (ram.size() == 0 || !mRamRtcEnabled)
                return 0xFF;

            addr &= 0x1FFF;
            uint32_t ramAddr = mRamCurrBank * ramBankSize + addr;

            return ram[ramAddr];
        }
        case 0x08: return mRamRtcEnabled ? rtc.readSec() : 0xFF; break;
        case 0x09: return mRamRtcEnabled ? rtc.readMin() : 0xFF; break;
        case 0x0A: return mRamRtcEnabled ? rtc.readHours() : 0xFF; break;
        case 0x0B: return mRamRtcEnabled ? rtc.readDaysL() : 0xFF; break;
        case 0x0C: return mRamRtcEnabled ? rtc.readDaysH() : 0xFF; break;
        default:
            // selecting other "banks" returns FF
            return 0xFF;
        }
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
        
        mRomCurrBank = val & mRomMask;
        if (mRomCurrBank == 0)
            mRomCurrBank++;
    }
    else if (addr >= 0x4000 && addr <= 0x5FFF) {
        // ram bank number or RTC register

        // writing a value in range for $00-$03 maps the corresponding external RAM Bank (if any) into memory at A000-BFFF.
        // When writing a value of $08-$0C, this will map the corresponding RTC register into memory at A000-BFFF

        // top bits are discarded
        if(val < 0x04)
            val &= mRamMask;

        mRamCurrBank = val & 0x0F;
    }
    else if (addr >= 0x6000 && addr <= 0x7FFF) {
        // latch clock data
        // When writing $00, and then $01 to this register, the current time becomes latched into the RTC registers.
        // The latched data will not change until it becomes latched again, by repeating the write $00->$01 procedure.
        // This provides a way to read the RTC registers while the clock keeps ticking.

        if(mRtcLatchReg == 0 && val == 1) {
            mRtcLatchReg = 1;
            rtc.latch();
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
                if(ram.size() != 0) {
                    addr &= 0x1FFF;
                    uint32_t ramAddr = mRamCurrBank * ramBankSize + addr;

                    ram[ramAddr] = val;
                }
                break;
            }
            case 0x08: rtc.writeSec(val); break;
            case 0x09: rtc.writeMin(val); break;
            case 0x0A: rtc.writeHours(val); break;
            case 0x0B: rtc.writeDaysL(val); break;
            case 0x0C: rtc.writeDaysH(val); break;
        }
    }
}


// ------------------------------------------------------------------------------------------------
// Mbc5
// ------------------------------------------------------------------------------------------------

Mbc5::Mbc5(size_t romSize, size_t ramSize)
    : MbcInterface(MbcType::Mbc5, romSize, ramSize)
    , mRamEnabled(false)
    , mRomMask(bankMask(9, mRomBanksCount))
    , mRamMask(bankMask(4, mRamBanksCount))
    , mRomB0(1)
    , mRomB1(0)
{
    mRomCurrBank = 1;
}

void Mbc5::onReset()
{
    mRamEnabled = false;

    mRomCurrBank = 1;
}

uint8_t Mbc5::read8(uint16_t addr) const
{
    if (addr <= mmap::rom::bank0::end) {
        // always read from bank 0
        return rom[addr];
    }
    else if (addr >= mmap::rom::bankN::start && addr <= mmap::rom::bankN::end) {
        // read from the selected rom bank
        // discard top 2 bits from the address

        uint32_t romAddr = mRomCurrBank * romBankSize + (addr & 0x3FFF);
        return rom[romAddr];
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        // read from ram 
        if (!mRamEnabled || ram.size() == 0)
            return 0xFF;

        // ram banks are 8KB in size so from the received address we discard the upper 3 bits
        addr &= 0x1FFF;
        uint32_t ramAddr = mRamCurrBank * ramBankSize + addr;

        return ram[ramAddr];
    }

    // should never get to this point
    assert(false);
    return 0xFF;
}

void Mbc5::write8(uint16_t addr, uint8_t val)
{
    if (addr <= 0x1FFF) {
        // ram enable
        // writing 0x0A enables ram, all other values disable it
        mRamEnabled = val == 0x0A;
    }
    else if (addr >= 0x2000 && addr <= 0x2FFF) {
        // bits 0 to 7 for rom bank number
        mRomB0 = val;
        mRomCurrBank = (mRomB0 | (mRomB1 << 8)) & mRomMask;
    }
    else if (addr >= 0x3000 && addr <= 0x3FFF) {
        // bit 8 for rom bank number
        mRomB1 = val & 0x01;
        mRomCurrBank = (mRomB0 | (mRomB1 << 8)) & mRomMask;
    }
    else if (addr >= 0x4000 && addr <= 0x5FFF) {
        // bits 0 to 3 for ram bank number
        mRamCurrBank = val & mRamMask;
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        // write to ram
        if (!mRamEnabled || ram.size() == 0)
            return;

        // ram banks are 8KB in size so from the received address we discard the upper 3 bits
        addr &= 0x1FFF;
        uint32_t ramAddr = mRamCurrBank * ramBankSize + addr;

        ram[ramAddr] = val;
    }
}



#include "SymFile.h"
#include "gb/GbCommons.h"
#include <string>

namespace fs = std::filesystem;


static BankSymTable initIOTable() {
    BankSymTable table;

    // joypad
    table[0xFF00] = "joypad";

    // serial
    table[0xFF01] = "serial_data";
    table[0xFF02] = "serial_ctrl";

    // timer
    table[0xFF04] = "DIV";
    table[0xFF05] = "TIMA";
    table[0xFF06] = "TMA";
    table[0xFF07] = "TAC";

    // interrupt flags
    table[0xFF0F] = "IF";

    // audio
    for (uint16_t addr = mmap::regs::audio::start; addr <= mmap::regs::audio::end; ++addr) {
        char buf[32] = { 0 };
        snprintf(buf, sizeof(buf), "audio_%04X", addr);

        table[addr] = buf;
    }

    // ppu
    table[0xFF40] = "LCDC";
    table[0xFF41] = "STAT";
    table[0xFF42] = "SCY";
    table[0xFF43] = "SCX";
    table[0xFF44] = "LY";
    table[0xFF45] = "LYC";
    table[0xFF46] = "DMA";
    table[0xFF47] = "BGP";
    table[0xFF48] = "OBP0";
    table[0xFF49] = "OBP1";
    table[0xFF4A] = "WY";
    table[0xFF4B] = "WX";

    return table;
}


SymTable::SymTable()
    :mIO(initIOTable())
{}

void SymTable::reset()
{
    // reset all current symbol tables
    for (auto& bankSymTable : mRomBanks)
        bankSymTable.clear();

    mHiRam.clear();
}

bool SymTable::parseSymbolFile(fs::path path)
{
    reset();

    // look for a file with the same name of the rom file but with the ".sym"
    // extension next to the rom file
    path.replace_extension("sym");

    if (!fs::exists(path))
        return false;

    // try to open file
    std::ifstream ifs(path);
    if (!ifs)
        return false;

    return parseWlalinkSymFile(ifs);
}



bool SymTable::parseWlalinkSymFile(std::ifstream& ifs)
{
    // reference for wlalink symbol files:
    // https://github.com/vhelin/wla-dx/blob/master/doc/symbols.rst

    std::string line;

    // read lines until we find the [labels] section
    while (true) {
        if (!std::getline(ifs, line))
            return false;

        if (line == "[labels]")
            break;
    }

    // parse each label and add it to the maps
    while (true) {
        if (!std::getline(ifs, line)) // eof
            break;
        if (!line.empty() && line[0] == '[') // new section
            break;

        uint32_t bank, addr;
        char label[256] = { 0 };
        if (sscanf(line.c_str(), "%2x:%4x %255s", &bank, &addr, label) == 3) {

            // insert symbol, don't overwrite
            if (auto table = getTable((uint16_t)bank, (uint16_t)addr); table) {
                table->try_emplace((uint16_t)addr, label);
            }

        }
    }

    return true;
}


const std::string* SymTable::getSymbol(uint16_t currRomBank, uint16_t currRamBank, uint16_t addr)
{
    if (addr == mmap::IE) {
        static const std::string ie = "IE";
        return &ie;
    }

    uint16_t bank = 0;
    if (addr >= mmap::rom::bankN::start && addr <= mmap::rom::bankN::end)
        bank = currRomBank;
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end)
        bank = currRamBank;

    if (auto table = getTable(bank, addr); table) {
        if (auto it = table->find(addr); it != table->end())
            return &it->second;
    }

    return nullptr;
}


BankSymTable* SymTable::getTable(uint16_t bank, uint16_t addr)
{
    BankSymTable* table = nullptr;

    if (addr >= mmap::rom::start && addr <= mmap::rom::end) {
        if (bank < maxRomBanks)
            table = &mRomBanks[bank];
    }
    else if (addr >= mmap::external_ram::start && addr <= mmap::external_ram::end) {
        if (bank < maxRamBanks)
            table = &mRamBanks[bank];
    }
    else if (addr >= mmap::regs::start && addr <= mmap::regs::end) {
        table = &mIO;
    }
    else if (addr >= mmap::hiram::start && addr <= mmap::hiram::end) {
        table = &mHiRam;
    }

    return table;
}




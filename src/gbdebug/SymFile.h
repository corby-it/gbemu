

#ifndef GBEMU_SRC_GBDEBUG_SYMFILE_H_
#define GBEMU_SRC_GBDEBUG_SYMFILE_H_


#include <unordered_map>
#include <filesystem>
#include <fstream>



typedef std::unordered_map<uint16_t, std::string>   BankSymTable;

class SymTable {
public:
    SymTable();

    bool parseSymbolFile(std::filesystem::path romPath);

    const std::string* getSymbol(uint16_t currRomBank, uint16_t currRamBank, uint16_t addr);

private:
    void reset();

    BankSymTable* getTable(uint16_t bank, uint16_t addr);

    bool parseWlalinkSymFile(std::ifstream& ifs);

    static constexpr size_t maxRomBanks = 512;
    static constexpr size_t maxRamBanks = 16;

    BankSymTable mRomBanks[maxRomBanks];
    BankSymTable mRamBanks[maxRamBanks];
    BankSymTable mHiRam;
    BankSymTable mIO;
    
};




#endif // GBEMU_SRC_GBDEBUG_SYMFILE_H_
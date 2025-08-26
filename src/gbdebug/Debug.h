

#ifndef GBEMU_SRC_GBDEBUG_DEBUG_H_
#define GBEMU_SRC_GBDEBUG_DEBUG_H_

#include "SymFile.h"
#include <string>
#include <memory>


class GameBoy;


class GBDebug {
public:
    GBDebug();

    bool enabled;

    bool breakOnLdbb;
    bool breakOnRet;

    size_t targetCallNesting;

    std::unique_ptr<SymTable> symTable;

    const std::string& currInstructionStr() const { return mCurrInstruction; }

    std::string updateInstructionToStr(const GameBoy& gb);

private:
    std::string instructionToStr(const GameBoy& gb);
    std::string instructionCBToStr(const GameBoy& gb);

    std::string symbolOrU16(const GameBoy& gb, uint16_t pc);
    std::string symbolOrS8(const GameBoy& gb, uint16_t pc);

    std::string mCurrInstruction;
};




#endif // GBEMU_SRC_GBDEBUG_DEBUG_H_
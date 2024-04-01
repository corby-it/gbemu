

#ifndef GBEMU_SRC_GBDEBUG_DEBUG_H_
#define GBEMU_SRC_GBDEBUG_DEBUG_H_

#include "SymFile.h"
#include <string>


class GameBoyClassic;


class GBDebug {
public:
    GBDebug();

    bool enabled;

    bool breakOnLdbb;
    bool breakOnRet;

    size_t targetCallNesting;

    SymTable symTable;

    const std::string& currInstructionStr() const { return mCurrInstruction; }

    std::string updateInstructionToStr(const GameBoyClassic& gb);

private:
    std::string instructionToStr(const GameBoyClassic& gb);
    std::string instructionCBToStr(const GameBoyClassic& gb);

    std::string symbolOrU16(const GameBoyClassic& gb, uint16_t pc);
    std::string symbolOrS8(const GameBoyClassic& gb, uint16_t pc);

    std::string mCurrInstruction;
};




#endif // GBEMU_SRC_GBDEBUG_DEBUG_H_
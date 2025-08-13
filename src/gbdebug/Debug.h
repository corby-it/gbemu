

#ifndef GBEMU_SRC_GBDEBUG_DEBUG_H_
#define GBEMU_SRC_GBDEBUG_DEBUG_H_

#include "SymFile.h"
#include <string>
#include <memory>


class GameBoyIf;


class GBDebug {
public:
    GBDebug();

    bool enabled;

    bool breakOnLdbb;
    bool breakOnRet;

    size_t targetCallNesting;

    std::unique_ptr<SymTable> symTable;

    const std::string& currInstructionStr() const { return mCurrInstruction; }

    std::string updateInstructionToStr(const GameBoyIf& gb);

private:
    std::string instructionToStr(const GameBoyIf& gb);
    std::string instructionCBToStr(const GameBoyIf& gb);

    std::string symbolOrU16(const GameBoyIf& gb, uint16_t pc);
    std::string symbolOrS8(const GameBoyIf& gb, uint16_t pc);

    std::string mCurrInstruction;
};




#endif // GBEMU_SRC_GBDEBUG_DEBUG_H_
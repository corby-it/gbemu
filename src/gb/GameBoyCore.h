

#ifndef GBEMU_SRC_GB_GAMEBOYCORE_H_
#define GBEMU_SRC_GB_GAMEBOYCORE_H_


#include "Bus.h"
#include "Cpu.h"
#include "Timer.h"
#include "Joypad.h"
#include "Dma.h"
#include "Ppu.h"
#include "Audio.h"
#include "Serial.h"
#include "Cartridge.h"
#include "gbdebug/Debug.h"
#include <chrono>
#include <filesystem>


struct EmulateRes {
    bool stillGoing;
    uint32_t elapsedMCycles;
};


class GameBoyClassic {
public:
    enum class Status {
        Stopped,
        Paused,
        Running,
        Stepping
    };
    static const char* statusToStr(Status st);


    GameBoyClassic();

    EmulateRes emulate();

    void play();
    void pause();
    void stop();
    void step();
    void stepReturn();

    CartridgeLoadingRes loadCartridge(const std::filesystem::path& path);

    std::chrono::nanoseconds avgCycleTime() const { return mCycleTimeAvg; }

    GBBus bus;
    CPU cpu;
    WorkRam wram;
    PPU ppu;
    DMA dma;
    Cartridge cartridge;
    Timer timer;
    Joypad joypad;
    Audio audio;
    Serial serial;
    HiRam hiRam;

    Status status;

    GBDebug dbg;

    // the gb cpu actually runs at 4.194304 MHz but, since we are not counting actual clock
    // cycles but machine cycles (clock cycles / 4) we have to use the clock frequency
    // divided by 4
    static constexpr uint32_t clockFreq = 4194304;
    static constexpr uint32_t machineFreq = 1048576;

    static constexpr std::chrono::nanoseconds clockPeriod = std::chrono::nanoseconds(238);
    static constexpr std::chrono::nanoseconds machinePeriod = std::chrono::nanoseconds(954);

private:

    void gbReset();
    uint32_t gbStep();

    bool mStepInstruction;

    uint64_t mCycleTimeCount;
    std::chrono::nanoseconds mCycleTimeAvg;

};


#endif // GBEMU_SRC_GB_GAMEBOYCORE_H_


#ifndef GBEMU_SRC_GB_GAMEBOYCORE_H_
#define GBEMU_SRC_GB_GAMEBOYCORE_H_


#include "Bus.h"
#include "Cpu.h"
#include "Timer.h"
#include "Joypad.h"
#include "Dma.h"
#include "Ppu.h"
#include "Apu.h"
#include "Serial.h"
#include "Cartridge.h"
#include "gbdebug/Debug.h"
#include <chrono>
#include <filesystem>


struct GbStepRes {
    bool frameReady;
    CpuStepRes cpuRes;
};

struct EmulateRes {
    bool stillGoing;
    GbStepRes stepRes;
};

enum class SaveStateError {
    NoError,
    OpenFileError,
    CartridgeMismatch,
    LoadingError,
    SavingError,
};

const char* saveStateErrorToStr(SaveStateError err);


typedef Ram<8_KB>   WorkRam;
typedef Ram<127>    HiRam;

CEREAL_CLASS_VERSION(WorkRam, 1);
CEREAL_CLASS_VERSION(HiRam, 1);




class GameBoyClassic : public Bus {
public:
    enum class Status {
        Stopped,
        Paused,
        Running,
        Stepping
    };
    static const char* statusToStr(Status st);


    GameBoyClassic();


    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;


    EmulateRes emulate();

    void play();
    void pause();
    void stop();
    void step();
    void stepReturn();


    SaveStateError saveState(const std::filesystem::path& path);
    SaveStateError loadState(const std::filesystem::path& path);

    CartridgeLoadingRes loadCartridge(const std::filesystem::path& path);

    std::filesystem::path romFilePath;


    CPU cpu;
    WorkRam wram;
    PPU ppu;
    DMA dma;
    Cartridge cartridge;
    Timer timer;
    Joypad joypad;
    APU apu;
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

    template<typename TimeT>
    static constexpr uint64_t timeToCycles(TimeT t) {
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(t);
        return nanos / machinePeriod;
    }

private:

    void gbReset();
    GbStepRes gbStep();

    bool mStepInstruction;

};


#endif // GBEMU_SRC_GB_GAMEBOYCORE_H_


#ifndef GBEMU_SRC_GB_GAMEBOYCORE_H_
#define GBEMU_SRC_GB_GAMEBOYCORE_H_


#include "Bus.h"
#include "WorkRam.h"
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
#include <map>



enum class GbType {
    DMG,
    CGB
};

const char* gbTypeToStr(GbType type);


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



typedef Ram<127>    HiRam;

CEREAL_CLASS_VERSION(HiRam, 1);




// ------------------------------------------------------------------------------------------------
// GameBoyIf
// ------------------------------------------------------------------------------------------------

typedef std::map<uint16_t, ReadWriteIf*>    AddressMap;


class GameBoyIf : public Bus {
public:
    enum class Status {
        Stopped,
        Paused,
        Running,
        Stepping
    };
    static const char* statusToStr(Status st);


    GameBoyIf(GbType type);

    GbType type() const { return mType; }

    EmulateRes emulate();

    void play();
    void pause();
    void stop();
    void step();
    void stepReturn();

    virtual SaveStateError saveState(const std::filesystem::path& path) = 0;
    virtual SaveStateError loadState(const std::filesystem::path& path) = 0;

    CartridgeLoadingRes loadCartridge(const std::filesystem::path& path);
    CartridgeLoadingRes loadCartridge(const uint8_t* data, size_t size);


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


protected:


private:

    virtual void gbReset() = 0;
    virtual GbStepRes gbStep() = 0;

    GbType mType;
    bool mStepInstruction;

};



// ------------------------------------------------------------------------------------------------
// GameBoyClassic
// ------------------------------------------------------------------------------------------------

class GameBoyClassic : public GameBoyIf {
public:

    GameBoyClassic();


    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;



    SaveStateError saveState(const std::filesystem::path& path) override;
    SaveStateError loadState(const std::filesystem::path& path) override;

   
    
    

private:

    void gbReset() override;
    GbStepRes gbStep() override;

    AddressMap initAddressMap();

    const AddressMap mAddrMap;
};



// ------------------------------------------------------------------------------------------------
// GameBoyColor
// ------------------------------------------------------------------------------------------------

class GameBoyColor : public GameBoyIf {
public:

    GameBoyColor();


    uint8_t read8(uint16_t addr) const override;
    void write8(uint16_t addr, uint8_t val) override;



    SaveStateError saveState(const std::filesystem::path& path) override;
    SaveStateError loadState(const std::filesystem::path& path) override;





private:

    void gbReset() override;
    GbStepRes gbStep() override;

    AddressMap initAddressMap();

    const AddressMap mAddrMap;
};




#endif // GBEMU_SRC_GB_GAMEBOYCORE_H_
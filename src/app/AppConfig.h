

#ifndef GBEMU_SRC_APP_APPCONFIG_H_
#define GBEMU_SRC_APP_APPCONFIG_H_


#include "gb/GameBoyCore.h"
#include <filesystem>
#include <list>


enum class EmulationSpeed {
    Quarter = 0,
    Half,
    Full,
    Unbound
};

const char* emulationSpeedToStr(EmulationSpeed es);


class AppConfig {
public:
    AppConfig();

    std::filesystem::path currentRomPath;
    CartridgeLoadingRes loadingRes;
    SaveStateError currentSaveStateErr;

    std::list<std::filesystem::path> recentRomsPath;

    EmulationSpeed emulationSpeed;

    bool showMemoryEditor;
    bool showTileViewer;

};


#endif // GBEMU_SRC_APP_APPCONFIG_H_
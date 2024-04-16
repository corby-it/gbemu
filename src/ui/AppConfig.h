

#ifndef GBEMU_SRC_UI_APPCONFIG_H_
#define GBEMU_SRC_UI_APPCONFIG_H_


#include "gb/GameBoyCore.h"
#include <filesystem>


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

    EmulationSpeed emulationSpeed;

};


#endif // GBEMU_SRC_UI_APPCONFIG_H_
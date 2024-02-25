

#ifndef GBEMU_SRC_UI_APPCONFIG_H_
#define GBEMU_SRC_UI_APPCONFIG_H_


#include "gb/GameBoyCore.h"
#include <filesystem>


class AppConfig {
public:
    AppConfig();

    std::filesystem::path currentRomPath;
    CartridgeLoadingRes loadingRes;

};


#endif // GBEMU_SRC_UI_APPCONFIG_H_
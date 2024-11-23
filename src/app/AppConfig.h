

#ifndef GBEMU_SRC_APP_APPCONFIG_H_
#define GBEMU_SRC_APP_APPCONFIG_H_


#include "gb/GameBoyCore.h"
#include "AppUtils.h"
#include "imgui.h"
#include <filesystem>
#include <list>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/list.hpp>



enum class EmulationSpeed {
    Quarter = 0,
    Half,
    Full,
    Unbound
};

const char* emulationSpeedToStr(EmulationSpeed es);


enum InputFn : uint32_t {
    Up = 0,
    Down,
    Left,
    Right,
    A,
    B,
    Start,
    Select,
    Pause,
    InputFnMax
};

const char* inputFnToStr(InputFn fn);



class InputConfig {
public:
    InputConfig();

    ImGuiKey& operator[](InputFn func) { return mValues[func]; }

    const ImGuiKey& operator[](InputFn func) const { return mValues[func]; }


    template<class Archive>
    void serialize(Archive& ar) {
        ar(mValues);
    }


private:

    std::array<ImGuiKey, InputFnMax> mValues;

};

CEREAL_CLASS_VERSION(InputConfig, 1);




class AppConfig {
public:
    AppConfig();

    template<class Archive>
    void save(Archive& ar, uint32_t const /*version*/) const {
        ar(cereal::make_nvp("recentRomsFolder", recentRomsFolder));
        ar(cereal::make_nvp("recentRomsPath", recentRomsPath));
        ar(cereal::make_nvp("inputCfg", inputCfg));
        ar(cereal::make_nvp("audioVolume", audioVolume));
    }

    template<class Archive>
    void load(Archive& ar, uint32_t const /*version*/) {
        ar(cereal::make_nvp("recentRomsFolder", recentRomsFolder));
        ar(cereal::make_nvp("recentRomsPath", recentRomsPath));
        ar(cereal::make_nvp("inputCfg", inputCfg));
        ar(cereal::make_nvp("audioVolume", audioVolume));
    }



    std::filesystem::path currentRomPath;
    CartridgeLoadingRes loadingRes;
    SaveStateError currentSaveStateErr;

    std::filesystem::path recentRomsFolder;
    std::list<std::filesystem::path> recentRomsPath;

    InputConfig inputCfg;

    EmulationSpeed emulationSpeed;

    float audioVolume;

    bool showMemoryEditor;
    bool showTileViewer;
    bool showBackgroundViewer;
    bool showInputConfigWindow;
    bool showAudioVisual;
    bool showSerialLog;

};


CEREAL_CLASS_VERSION(AppConfig, 1);



#endif // GBEMU_SRC_APP_APPCONFIG_H_
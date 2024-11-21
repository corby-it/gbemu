

#include "AppConfig.h"

const char* emulationSpeedToStr(EmulationSpeed es)
{
    switch (es)
    {
    case EmulationSpeed::Quarter: return "25%";
    case EmulationSpeed::Half: return "50%";
    case EmulationSpeed::Full: return "100%";
    case EmulationSpeed::Unbound: return "Unbound";
    default:
        return "???";
    }
}

const char* inputFnToStr(InputFn fn)
{
    switch (fn)
    {
    case Up: return "Up";
    case Down: return "Down";
    case Left: return "Left";
    case Right: return "Right";
    case A: return "A";
    case B: return "B";
    case Start: return "Start";
    case Select: return "Select";
    case Pause: return "Pause";

    case InputFnMax: 
    default:
        return "unknown";
    }
}


InputConfig::InputConfig()
{
    mValues[InputFn::Up] = ImGuiKey_W;
    mValues[InputFn::Left] = ImGuiKey_A;
    mValues[InputFn::Down] = ImGuiKey_S;
    mValues[InputFn::Right] = ImGuiKey_D;

    mValues[InputFn::A] = ImGuiKey_N;
    mValues[InputFn::B] = ImGuiKey_M;
    mValues[InputFn::Start] = ImGuiKey_Enter;
    mValues[InputFn::Select] = ImGuiKey_0;

    mValues[InputFn::Pause] = ImGuiKey_P;
}







AppConfig::AppConfig()
    : loadingRes(CartridgeLoadingRes::Ok)
    , currentSaveStateErr(SaveStateError::NoError)
    , emulationSpeed(EmulationSpeed::Full)
    , audioVolume(0.8f)
    , showMemoryEditor(false)
    , showTileViewer(false)
    , showBackgroundViewer(false)
    , showInputConfigWindow(false)
    , showAudioVisual(false)
{}


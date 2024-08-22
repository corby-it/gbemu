
#ifndef GBEMU_SRC_UI_APP_H_
#define GBEMU_SRC_UI_APP_H_


#include "AppBase.h"
#include "AppConfig.h"
#include "gb/GameBoyCore.h"
#include <optional>
#include <chrono>


class App : public AppBase {
public:
    App();
    virtual ~App() {}

    void startup() override;
    bool emulate() override;
    void updateUI() override;


private:

    bool emulateFullSpeed(std::chrono::nanoseconds currTime);
    bool emulateOtherSpeeds(std::chrono::nanoseconds currTime);

    std::optional<std::chrono::nanoseconds> emulateFor() const;

    void UIDrawMenu();
    void UIDrawControlWindow();
    void UIDrawEmulationWindow();
    
    void UIDrawRegsTables();
    void UIDrawCpuRegTable();
    void UIDrawCpuFlagsTable();
    void UIDrawTimerRegTable();
    void UIDrawPpuRegTable();

    
    AppConfig mConfig;
    GameBoyClassic mGameboy;

    RgbaBuffer mDisplayBuffer;
    GLuint mGLDisplayTexture;

    int mEmulationSpeedComboIdx;

    std::chrono::nanoseconds mLastEmulateCall;

};


#endif // GBEMU_SRC_UI_APP_H_
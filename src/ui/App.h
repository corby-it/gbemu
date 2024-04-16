
#ifndef GBEMU_SRC_UI_APP_H_
#define GBEMU_SRC_UI_APP_H_


#include "AppBase.h"
#include "AppConfig.h"
#include "gb/GameBoyCore.h"


class App : public AppBase {
public:
    App();
    virtual ~App() {}

    void startup() override {}
    bool emulate() override;
    void updateUI() override;


private:

    void UIDrawMenu();
    void UIDrawControlWindow();
    void UIDrawEmulationWindow();
    
    void UIDrawRegsTables();
    void UIDrawCpuRegTable();
    void UIDrawCpuFlagsTable();
    void UIDrawTimerRegTable();
    void UIDrawPpuRegTable();

    std::set<Joypad::Btn> getPressedButtons() const;

    AppConfig mConfig;
    GameBoyClassic mGameboy;

    RgbBuffer mDisplayBuffer;
    GLuint mGLDisplayTexture;

};


#endif // GBEMU_SRC_UI_APP_H_
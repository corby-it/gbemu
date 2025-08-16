
#ifndef GBEMU_SRC_APP_APP_H_
#define GBEMU_SRC_APP_APP_H_


#include "AppBase.h"
#include "AppConfig.h"
#include "AudioHandler.h"
#include "SerialLogWindow.h"
#include "gb/GameBoyCore.h"
#include "gb/Matrix.h"
#include <optional>
#include <chrono>
#include <memory>
#include <fstream>





// ------------------------------------------------------------------------------------------------
// App
// ------------------------------------------------------------------------------------------------

class App : public AppBase {
public:
    App();
    virtual ~App();

    void startup() override;
    bool emulate() override;
    void updateUI() override;
    

private:

    Joypad::PressedButton getPressedButtons();

    bool emulateFullSpeed(std::chrono::nanoseconds currTime);
    bool emulateOtherSpeeds(std::chrono::nanoseconds currTime);

    std::optional<std::chrono::nanoseconds> emulateFor() const;

    void UISetupDocking();
    void UICheckAudioInitialization();
    void UIDrawMenu();
    void UIDrawControlWindow();
    void UIDrawGBDisplayWindow();
    void UIDrawMemoryEditorWindow();
    void UIDrawTileViewerWindow();
    void UIDrawBackgroundViewerWindow();
    void UIDrawInputConfigWindow();
    void UIDrawAudioVisualWindow();
    void UIDrawSerialLogWindow();
    
    void UIDrawRegsTables();
    void UIDrawCpuRegTable();
    void UIDrawCpuFlagsTable();
    void UIDrawTimerRegTable();
    void UIDrawPpuRegTable();
    void UIDrawApuRegTable();

    bool loadRomFile(const std::filesystem::path& path);

    
    static float getResamplingRatio(EmulationSpeed speed);

    AppConfig mConfig;
    std::filesystem::path mConfigSavePath;

    GameBoyColor mGameboy;

    RgbaBuffer mDisplayBuffer;

    GLuint mGLDisplayTexture;
    std::vector<GLuint> mTileTextures;
    std::vector<GLuint> mOamTextures;
    std::vector<GLuint> mBgTextures;

    std::chrono::nanoseconds mLastEmulateCall;

    bool mAudioInitSuccess;
    AudioHandler mAudioHandler;

    SerialLogWindow mSerialLog;

};


#endif // GBEMU_SRC_APP_APP_H_

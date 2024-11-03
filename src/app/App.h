
#ifndef GBEMU_SRC_APP_APP_H_
#define GBEMU_SRC_APP_APP_H_


#include "AppBase.h"
#include "AppConfig.h"
#include "gb/GameBoyCore.h"
#include "gb/Matrix.h"
#include <miniaudio.h>
#include <optional>
#include <chrono>
#include <memory>


class App : public AppBase {
public:
    App();
    virtual ~App();

    void startup() override;
    bool emulate() override;
    void updateUI() override;
    

private:

    Joypad::PressedButton getPressedButtons();
    void audioSetup();
    void audioTeardown();

    bool emulateFullSpeed(std::chrono::nanoseconds currTime);
    bool emulateOtherSpeeds(std::chrono::nanoseconds currTime);

    std::optional<std::chrono::nanoseconds> emulateFor() const;

    void UISetupDocking();
    void UIDrawMenu();
    void UIDrawControlWindow();
    void UIDrawGBDisplayWindow();
    void UIDrawMemoryEditorWindow();
    void UIDrawTileViewerWindow();
    void UIDrawBackgroundViewerWindow();
    void UIDrawInputConfigWindow();
    
    void UIDrawRegsTables();
    void UIDrawCpuRegTable();
    void UIDrawCpuFlagsTable();
    void UIDrawTimerRegTable();
    void UIDrawPpuRegTable();

    bool loadRomFile(const std::filesystem::path& path);

    void onAudioSampleReady(float sampleL, float sampleR);
    static void audioDataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

    
    AppConfig mConfig;
    std::filesystem::path mConfigSavePath;

    GameBoyClassic mGameboy;

    RgbaBuffer mDisplayBuffer;

    GLuint mGLDisplayTexture;
    std::vector<GLuint> mTileTextures;
    std::vector<GLuint> mOamTextures;
    std::vector<GLuint> mBgTextures;

    std::chrono::nanoseconds mLastEmulateCall;

    std::unique_ptr<ma_device> mAudioDevice;
    ma_pcm_rb mAudioRingBuffer;
    ma_resampler mAudioResampler;

};


#endif // GBEMU_SRC_APP_APP_H_

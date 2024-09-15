
#ifndef GBEMU_SRC_APP_APP_H_
#define GBEMU_SRC_APP_APP_H_


#include "AppBase.h"
#include "AppConfig.h"
#include "gb/GameBoyCore.h"
#include "gb/Matrix.h"
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

    void UISetupDocking();
    void UIDrawMenu();
    void UIDrawControlWindow();
    void UIDrawEmulationWindow();
    void UIDrawMemoryEditorWindow();
    void UIDrawTileViewerWindow();
    
    void UIDrawRegsTables();
    void UIDrawCpuRegTable();
    void UIDrawCpuFlagsTable();
    void UIDrawTimerRegTable();
    void UIDrawPpuRegTable();

    bool loadRomFile(const std::filesystem::path& path);

    
    AppConfig mConfig;
    GameBoyClassic mGameboy;

    RgbaBuffer mDisplayBuffer;
    GLuint mGLDisplayTexture;

    std::vector<RgbaBufferArray<TileData::w, TileData::h>> mTileBuffers;
    std::vector<GLuint> mTileTextures;

    std::vector<RgbaBufferArray<TileData::w, TileData::h>> mOamBuffers;
    std::vector<GLuint> mOamTextures;

    int mEmulationSpeedComboIdx;

    std::chrono::nanoseconds mLastEmulateCall;

};


#endif // GBEMU_SRC_APP_APP_H_
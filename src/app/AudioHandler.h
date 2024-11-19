
#ifndef GBEMU_SRC_APP_AUDIOHANLDER_H_
#define GBEMU_SRC_APP_AUDIOHANLDER_H_

#include <memory>
#include <vector>
#include <atomic>
#include <filesystem>
#include <miniaudio.h>



class AudioHandler {
public:
    AudioHandler(float resamplingRatio = 1.00456238751f);
    ~AudioHandler();

    bool initialize();
    std::string getInitResult();

    void onAudioSampleReady(float sampleL, float sampleR);

    void setResamplingRatio(float ratio) { mResamplingRatio = ratio; }
    void setVolume(float vol);

    void enableSynthesizedFileOutput(const std::filesystem::path& fname = "audio-synth.wav");
    void enablePlayedFileOutput(const std::filesystem::path& fname = "audio-played.wav");

    static constexpr uint32_t sampleRate = 44100;
    static constexpr uint32_t channels = 2;
    static constexpr ma_format format = ma_format_f32;


private:

    static constexpr uint32_t ringBufferSize = sampleRate * 5; // 5 seconds of audio

    static void audioDataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 requestedFrames);

    void processAudioDataFixedRate(float* pOutput, ma_uint32 requestedFrames, float resamplingRatio);
    void processAudioDataUnbound(float* pOutput, ma_uint32 requestedFrames);

    ma_result mInitResult;

    std::atomic<float> mResamplingRatio;
    std::atomic<float> mVolume;

    std::atomic<bool> mSynthFileOutputEnabled;
    std::atomic<bool> mPlayedFileOutputEnabled;
    
    std::unique_ptr<ma_device> mAudioDevice;
    ma_pcm_rb mAudioRingBuffer;
    ma_resampler mAudioResampler;
    ma_encoder mAudioWavEncoderSynthesized;
    ma_encoder mAudioWavEncoderPlayed;

    std::vector<float> mAudioBuf;

    float mResamplingErrComp;
};


#endif // GBEMU_SRC_APP_AUDIOHANLDER_H_
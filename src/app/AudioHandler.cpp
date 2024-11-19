
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "AudioHandler.h"
#include <tracy/Tracy.hpp>
#include <cmath>
#include <algorithm>



const char* const plotRequestedAudioFrames = "RequestedAudioFrames";
const char* const plotAvailableAudioFrames = "AvailableAudioFrames";
const char* const plotResamplingErrComp = "ResamplingErrComp";


AudioHandler::AudioHandler(float resamplingRatio)
    : mInitResult(MA_SUCCESS)
    , mResamplingRatio(resamplingRatio)
    , mVolume(1.f)
    , mSynthFileOutputEnabled(false)
    , mPlayedFileOutputEnabled(false)
    , mAudioDevice(std::make_unique<ma_device>())
    , mResamplingErrComp(0)
{
    // allocate memory for 0.5 seconds of audio (it takes two floats for 
    // a single sample so we need sampleRate values for 0.5 seconds of audio)
    mAudioBuf.reserve(sampleRate * channels / 2);
}

AudioHandler::~AudioHandler()
{
    if(mSynthFileOutputEnabled)
        ma_encoder_uninit(&mAudioWavEncoderSynthesized);
    if(mPlayedFileOutputEnabled)
        ma_encoder_uninit(&mAudioWavEncoderPlayed);

    ma_device_uninit(mAudioDevice.get());
    ma_resampler_uninit(&mAudioResampler, nullptr);
    ma_pcm_rb_uninit(&mAudioRingBuffer);
}

void AudioHandler::setVolume(float vol)
{
    mVolume = std::clamp(vol, 0.f, 1.f);
}

void AudioHandler::enableSynthesizedFileOutput(const std::filesystem::path& fname)
{
    if (mSynthFileOutputEnabled)
        return;

    ma_encoder_config encoderCfg = ma_encoder_config_init(ma_encoding_format_wav, format, channels, sampleRate);
    
    auto res = ma_encoder_init_file(fname.string().c_str(), &encoderCfg, &mAudioWavEncoderSynthesized);
    
    if(res == MA_SUCCESS)
        mSynthFileOutputEnabled = true;
}

void AudioHandler::enablePlayedFileOutput(const std::filesystem::path& fname)
{
    if (mPlayedFileOutputEnabled)
        return;

    ma_encoder_config encoderCfg = ma_encoder_config_init(ma_encoding_format_wav, format, channels, sampleRate);

    auto res = ma_encoder_init_file(fname.string().c_str(), &encoderCfg, &mAudioWavEncoderPlayed);

    if (res == MA_SUCCESS)
        mPlayedFileOutputEnabled = true;
}

bool AudioHandler::initialize()
{
    // setup tracy plots
    TracyPlotConfig(plotRequestedAudioFrames, tracy::PlotFormatType::Number, false, false, 0);
    TracyPlotConfig(plotAvailableAudioFrames, tracy::PlotFormatType::Number, false, false, 0);
    TracyPlotConfig(plotResamplingErrComp, tracy::PlotFormatType::Number, false, false, 0);

    bool success = true;

    // setup the ring buffer for 5 seconds of audio
    ma_result res = ma_pcm_rb_init(format, channels, ringBufferSize, nullptr, nullptr, &mAudioRingBuffer);
    if (res == MA_SUCCESS) {
        ma_pcm_rb_reset(&mAudioRingBuffer);
    }
    else {
        success &= false;
        mInitResult = res;
    }

    // setup the resampler
    ma_uint32 resamplingFreq = lroundf(sampleRate * mResamplingRatio);

    ma_resampler_config resamplerConfig = ma_resampler_config_init(
        format,
        channels,
        resamplingFreq,
        sampleRate,
        ma_resample_algorithm_linear);

    res = ma_resampler_init(&resamplerConfig, nullptr, &mAudioResampler);
    if (res != MA_SUCCESS) {
        success &= false;
        mInitResult = res;
    }

    // setup the audio device 
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = format;
    deviceConfig.playback.channels = channels;
    deviceConfig.sampleRate = sampleRate;
    deviceConfig.dataCallback = AudioHandler::audioDataCallback;
    deviceConfig.pUserData = this;

    res = ma_device_init(NULL, &deviceConfig, mAudioDevice.get());
    if (res == MA_SUCCESS) {
        res = ma_device_start(mAudioDevice.get());
        if (res != MA_SUCCESS) {
            success &= false;
            mInitResult = res;
        }
    }
    else {
        success &= false;
        mInitResult = res;
    }

    return success;
}

std::string AudioHandler::getInitResult()
{
    return ma_result_description(mInitResult);
}

void AudioHandler::onAudioSampleReady(float sampleL, float sampleR)
{
    void* pvBuf;
    uint32_t frames = 1;
    ma_pcm_rb_acquire_write(&mAudioRingBuffer, &frames, &pvBuf);

    if (frames == 1) {
        auto pfBuf = static_cast<float*>(pvBuf);
        *pfBuf = sampleL;
        ++pfBuf;
        *pfBuf = sampleR;
    }

    ma_pcm_rb_commit_write(&mAudioRingBuffer, frames);
}


void AudioHandler::audioDataCallback(ma_device* pDevice, void* pOutput, const void* /*pInput*/, ma_uint32 requestedFrames)
{
    float* pfOutBuf = static_cast<float*>(pOutput);
    AudioHandler* handler = static_cast<AudioHandler*>(pDevice->pUserData);
    
    TracyPlot(plotRequestedAudioFrames, (int64_t)requestedFrames);
    TracyPlot(plotAvailableAudioFrames, (int64_t)ma_pcm_rb_available_read(&handler->mAudioRingBuffer));

    // the emulation doesn't run perfectly synchronized to the audio sampling rate, it's faster or slower
    // the number of frames we need to extract from the ring buffer depends on the current resampling ratio
    // also set the correct ratio value in the resampler

    float ratio = handler->mResamplingRatio.load();
    if (ratio < 0.f)
        handler->processAudioDataUnbound(pfOutBuf, requestedFrames);
    else 
        handler->processAudioDataFixedRate(pfOutBuf, requestedFrames, ratio);
}

void AudioHandler::processAudioDataFixedRate(float* pfOutBuf, ma_uint32 requestedFrames, float resamplingRatio)
{
    // the emulation doesn't run perfectly synchronized to the audio sampling rate, it's faster or slower
    // the number of frames we need to extract from the ring buffer depends on the current resampling ratio
    // also set the correct ratio value in the resampler

    ma_resampler_set_rate_ratio(&mAudioResampler, resamplingRatio);

    // the number of needed frames has a decimal part but the number
    // of processed frames must be an integer
    // to compensate for this we store the decimal part in mResamplingErrComp
    // and, when it reaches 1, we process an additional sample

    float fNeededFrames = requestedFrames * resamplingRatio;
    ma_uint32 neededFrames = (ma_uint32)fNeededFrames;

    mResamplingErrComp += fNeededFrames - neededFrames;

    TracyPlot(plotResamplingErrComp, mResamplingErrComp);

    if (mResamplingErrComp >= 1.f) {
        neededFrames++;
        mResamplingErrComp -= 1.f;
    }


    // extract sample data from the ring buffer now, three possible cases
    // - the buffer is empty:
    //      nothing to do, the output buffer is already silenced
    // - we extract a number of frames >= than the needed frames:
    //      resample them into the requested frames and write them to the output buffer
    // - we extract a number of frames that is < than the needed frames:
    //      this might happen because there are no more frames in the buffer or
    //      because the frames are available but the ring buffer must loop around so the first
    //      time we request a read it returns a number of available frames lower than what's actually available
    //      in this case we try to read again, if we get more frames we proceed to resample them, 
    //      otherwise we wait for the next call to get more frames

    // keep extracting frames from the buffer until we have the needed number of frames to process them or 
    // until we can't get enough of them and we leave the output buffer silent

    while (mAudioBuf.size() < neededFrames * channels) {
        void* pvInBuf;
        ma_uint32 availableFrames = neededFrames - ((ma_uint32)mAudioBuf.size() / channels);

        ma_pcm_rb_acquire_read(&mAudioRingBuffer, &availableFrames, &pvInBuf);

        float* pfInBuf = static_cast<float*>(pvInBuf);
        mAudioBuf.insert(mAudioBuf.end(), pfInBuf, pfInBuf + availableFrames * channels);

        ma_pcm_rb_commit_read(&mAudioRingBuffer, availableFrames);

        // if the buffer was empty we can stop
        if (availableFrames == 0)
            break;
    }

    // do the resampling (if needed)
    if (mAudioBuf.size() == neededFrames * channels) {
        // we have enough frames for the resampling operation, write them directly to the output buffer

        ma_uint64 resamplerFramesIn = neededFrames;
        ma_uint64 resamplerFramesOut = requestedFrames;

        ma_resampler_process_pcm_frames(&mAudioResampler, mAudioBuf.data(), &resamplerFramesIn, pfOutBuf, &resamplerFramesOut);

        if(mSynthFileOutputEnabled)
            ma_encoder_write_pcm_frames(&mAudioWavEncoderSynthesized, mAudioBuf.data(), neededFrames, nullptr);
        if(mPlayedFileOutputEnabled)
            ma_encoder_write_pcm_frames(&mAudioWavEncoderPlayed, pfOutBuf, requestedFrames, nullptr);

        // clear the buffer and get ready for the next round
        mAudioBuf.clear();

        // apply volume to the output buffer
        auto vol = mVolume.load();
        for (uint32_t i = 0; i < requestedFrames; ++i) {
            pfOutBuf[i * 2] *= vol;
            pfOutBuf[i * 2 + 1] *= vol;
        }
    }
    else {
        TracyMessageL("Not enough audio frames!");
    }
}

void AudioHandler::processAudioDataUnbound(float* pfOutBuf, ma_uint32 requestedFrames)
{
    // when the emulation is unbound we resample everything we find in the ring buffer into the output buffer

    void* pvInBuf;
    ma_uint32 availableFrames = ringBufferSize;

    ma_pcm_rb_acquire_read(&mAudioRingBuffer, &availableFrames, &pvInBuf);

    float* pfInBuf = static_cast<float*>(pvInBuf);

    // the new resampling ratio is availableFrames / requested frames
    ma_resampler_set_rate_ratio(&mAudioResampler, (float)availableFrames/requestedFrames);

    ma_uint64 resamplerFramesIn = availableFrames;
    ma_uint64 resamplerFramesOut = requestedFrames;

    ma_resampler_process_pcm_frames(&mAudioResampler, pfInBuf, &resamplerFramesIn, pfOutBuf, &resamplerFramesOut);

    ma_pcm_rb_commit_read(&mAudioRingBuffer, availableFrames);

    // apply volume to the output buffer
    auto vol = mVolume.load();
    for (uint32_t i = 0; i < requestedFrames; ++i) {
        pfOutBuf[i * 2] *= vol;
        pfOutBuf[i * 2 + 1] *= vol;
    }
}

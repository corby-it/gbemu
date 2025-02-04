
#include "libretro.h"
#include "Version.h"
#include "gb/GameBoyCore.h"
#include <limits>
#include <memory>


static GameBoyClassic gb;
static std::unique_ptr<uint8_t[]> framebuf;

// color data for each pixel will be stored as RGB565 so 2 bytes for each pixel
static constexpr size_t framebuf_size = Display::w * Display::h * 2;


// callbacks
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }



unsigned retro_api_version()
{
    return RETRO_API_VERSION;
}


void retro_get_system_info(retro_system_info* info)
{
    memset(info, 0, sizeof(retro_system_info));

    info->library_name = PROJECT_NAME_STR;
    info->library_version = VERSION_STR;
    info->need_fullpath = false;
    info->valid_extensions = "gb|gbc";
}


void retro_get_system_av_info(retro_system_av_info* info)
{
    memset(info, 0, sizeof(retro_system_av_info));
    
    info->timing.fps = 60.0f;
    info->timing.sample_rate = 44100;

    info->geometry.base_width = Display::w;
    info->geometry.base_height = Display::h;
    info->geometry.max_width = Display::w;
    info->geometry.max_height = Display::h;
    info->geometry.aspect_ratio = (float)Display::w / (float)Display::h;

    // set pixel format to RGB565 instead of 0RBG1555
    auto pixFormat = RETRO_PIXEL_FORMAT_RGB565;
    environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixFormat);
}

static void error_msg(const char* err_str)
{
    struct retro_message_ext msg;
    msg.msg = err_str;
    msg.duration = 10000; // 10 seconds
    msg.level = RETRO_LOG_ERROR;
    msg.priority = 0;
    msg.target = RETRO_MESSAGE_TARGET_OSD;
    msg.type = RETRO_MESSAGE_TYPE_NOTIFICATION;
    msg.progress = 0;

    environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE_EXT, &msg);
}


static void gb_audio_callback(float fLeft, float fRight)
{
    // the emulator produces audio samples as floats in the 0.0 - 1.0 range
    // libretro audio callback expects uint16_t values in the 0 - 65535 range
    static constexpr auto u16Max = std::numeric_limits<uint16_t>::max();

    auto uLeft = static_cast<uint16_t>(u16Max * fLeft);
    auto uRight = static_cast<uint16_t>(u16Max * fRight);

    audio_cb(uLeft, uRight);
}

void retro_init()
{
    framebuf = std::make_unique<uint8_t[]>(framebuf_size);
    gb.apu.setSampleCallback(gb_audio_callback);
}

void retro_deinit()
{
    gb.apu.setSampleCallback(OnSampleReadyCallback{});
    framebuf.reset();
}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    // this emulator can run even if a game is not loaded, nothing
    // will be displayed but it will run
    bool no_content = true;
    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);
}

void retro_set_controller_port_device(unsigned /*port*/, unsigned /*device*/)
{
    // nothing to do, we can only have a joypad for player 0
}

static Joypad::PressedButton get_pressed_buttons()
{
    Joypad::PressedButton btns;

    input_poll_cb();

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP)) btns.add(Joypad::Btn::Up);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT)) btns.add(Joypad::Btn::Left);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN)) btns.add(Joypad::Btn::Down);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT)) btns.add(Joypad::Btn::Right);

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A)) btns.add(Joypad::Btn::A);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B)) btns.add(Joypad::Btn::B);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START)) btns.add(Joypad::Btn::Start);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT)) btns.add(Joypad::Btn::Select);
    
    return btns;
}

static void transfer_frame()
{
    // keep the most significant 5 or 6 bits of each color value
    static constexpr uint8_t mask5 = 0xF8;
    static constexpr uint8_t mask6 = 0xFC;

    static const uint32_t white = whiteA.asU32();
    static const uint32_t lightGrey = lightGreyA.asU32();
    static const uint32_t darkGrey = darkGreyA.asU32();
    static const uint32_t black = blackA.asU32();

    const auto& display = gb.ppu.display.getFrontBuf();

    uint8_t* ptr = framebuf.get();

    for (uint32_t r = 0; r < display.height(); ++r) {
        for (uint32_t c = 0; c < display.width(); ++c) {
            uint32_t colorRGBA = 0;

            switch (display.get(c, r)) {
            default:
            case 0: colorRGBA = white; break;
            case 1: colorRGBA = lightGrey; break;
            case 2: colorRGBA = darkGrey; break;
            case 3: colorRGBA = black; break;
            }

            // convert to RGB565
            auto red = uint8_t((colorRGBA & 0xFF000000) >> 24);
            auto green = uint8_t((colorRGBA & 0x00FF0000) >> 16);
            auto blue = uint8_t((colorRGBA & 0x0000FF00) >> 8);
            
            uint16_t colorRGB565 = ((red & mask5) << 8) | ((green & mask6) << 3) | ((blue & mask5) >> 3);

            *ptr++ = (uint8_t)colorRGB565;
            *ptr++ = (uint8_t)(colorRGB565 >> 8);
        }
    }

    video_cb(framebuf.get(), Display::w, Display::h, Display::w * 2);
}

void retro_run(void)
{
    // check input
    gb.joypad.action(get_pressed_buttons());
    
    // emulate 1 frame
    while (!gb.emulate().stepRes.frameReady) {}

    // transfer the frame to the frontend
    transfer_frame();
}

bool retro_load_game(const struct retro_game_info* info)
{
    if (info && info->data) {
        auto res = gb.loadCartridge((uint8_t*)info->data, info->size);

        if (res != CartridgeLoadingRes::Ok) {
            error_msg(cartridgeLoadingResToStr(res));
            return false;
        }
        else {
            gb.play();
            return true;
        }
    }
    else {
        return false;
    }
}

bool retro_load_game_special(unsigned /*game_type*/, const struct retro_game_info* /*info*/, size_t /*num_info*/)
{
    return false;
}

void retro_unload_game()
{
    gb.stop();
}

void retro_reset()
{
    gb.stop();
    gb.play();
}

size_t retro_serialize_size(void)
{
    return 0;
}

bool retro_serialize(void* /*data*/, size_t /*size*/)
{
    return false;
}

bool retro_unserialize(const void* /*data*/, size_t /*size*/)
{
    return false;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned /*index*/, bool /*enabled*/, const char* /*code*/)
{
}

unsigned retro_get_region(void)
{
    // doesn't mean much with the gameboy so return NTSC as stated in the libretro docs
    return RETRO_REGION_NTSC;
}

void* retro_get_memory_data(unsigned /*id*/)
{
    return NULL;
}

size_t retro_get_memory_size(unsigned /*id*/)
{
    return 0;
}

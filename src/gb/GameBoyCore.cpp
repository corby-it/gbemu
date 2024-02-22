

#include "GameBoyCore.h"
#include "GbCommons.h"


GameBoyClassic::GameBoyClassic()
    : mBus()
    , mCpu(mBus)
    , mWram(mmap::wram::start)
    , mPpu(mBus)
    , mDma(mBus)
    , mCartridge()
    , mTimer(mBus)
    , mJoypad(mBus)
    , mAudio()
    , mSerial()
    , mHiRam(mmap::hiram::start)
{
    mBus.connect(mCpu);
    mBus.connect(mWram);
    mBus.connect(mPpu);
    mBus.connect(mDma);
    mBus.connect(mCartridge);
    mBus.connect(mTimer);
    mBus.connect(mJoypad);
    mBus.connect(mAudio);
    mBus.connect(mSerial);
    mBus.connect(mHiRam);
}

void GameBoyClassic::run()
{


}

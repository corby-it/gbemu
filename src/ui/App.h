
#ifndef GBEMU_SRC_UI_APP_H_
#define GBEMU_SRC_UI_APP_H_


#include "AppBase.h"
#include "gb/Ppu.h"


class App : public AppBase {
public:
    App();
    virtual ~App() {}

    void startup() override {}
    void update() override;


private:

    TestBus bus;
    PPU p;

    RgbBuffer mDisplayBuffer;

};


#endif // GBEMU_SRC_UI_APP_H_
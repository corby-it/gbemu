
#ifndef GBEMU_SRC_UI_APP_H_
#define GBEMU_SRC_UI_APP_H_


#include "AppBase.h"


class App : public AppBase {
public:

    virtual ~App() {}

    void startup() override {}
    void update() override {}


private:

};


#endif // GBEMU_SRC_UI_APP_H_
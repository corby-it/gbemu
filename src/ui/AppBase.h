

#ifndef GBEMU_SRC_UI_APPBASE_H_
#define GBEMU_SRC_UI_APPBASE_H_

#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "imgui.h"



class AppBase {
public:
    AppBase();
    virtual ~AppBase();

    void run();

    virtual void startup() = 0;
    virtual void update() = 0;

private:

    GLFWwindow* mWindow;
    ImVec4 mClearColor;
};


#endif // GBEMU_SRC_UI_APPBASE_H_

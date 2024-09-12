

#ifndef GBEMU_SRC_UI_APPBASE_H_
#define GBEMU_SRC_UI_APPBASE_H_

#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "imgui.h"
#include <vector>
#include <chrono>



class AppBase {
public:
    AppBase();
    virtual ~AppBase();

    void run();

    virtual void startup() = 0;
    virtual bool emulate() = 0;
    virtual void updateUI() = 0;

protected:

    void closeWindow();

    std::chrono::nanoseconds getCurrTime() const;


private:
    static constexpr double fpsLimit = 1.0 / 60.0;
    double mLastFrameTime;

    GLFWwindow* mWindow;
    ImVec4 mClearColor;
};


#endif // GBEMU_SRC_UI_APPBASE_H_

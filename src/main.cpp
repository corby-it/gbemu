
#include "Version.h"
#include <cstdio>



// this is required to implement a test runner for doctest, otherwise it won't compile
// in release mode everything will be stripped away
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

#include "ui/App.h"


int commonMain(int /*argc*/, char** /*argv*/)
{
    App app;
    app.run();

    return 0;
}


#ifdef _WIN32

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    static char* fakeArgv[] = { "gbemu" };

    return commonMain(1, fakeArgv);
}

#endif // _WIN32


#ifdef __linux__

int main(int argc, char** argv)
{
    return commonMain(argc, argv);
}

#endif // __linux__

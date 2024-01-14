
#include "Version.h"
#include <cstdio>


// this is required to implement a test runner for doctest, otherwise it won't compile
// in release mode everything will be stripped away
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

#include "ui/App.h"

// Main code
int main(int /*argc*/, char** /*argv*/)
{
    App app;
    app.run();

    return 0;
}

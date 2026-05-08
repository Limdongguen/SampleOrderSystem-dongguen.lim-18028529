#include <iostream>
#include <windows.h>

#ifdef _DEBUG
#include "gmock/gmock.h"
#endif

#ifdef _DEBUG

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}

#else

#include <windows.h>
#include "controller/MainController.h"

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    MainController().run();
    return 0;
}

#endif

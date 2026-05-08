#include <iostream>

#ifdef _DEBUG
#include "gmock/gmock.h"
#endif

#ifdef _DEBUG

int main() {
    ::testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}

#else

#include "controller/MainController.h"

int main()
{
    MainController().run();
    return 0;
}

#endif

#pragma once
#include <string>

class ConsoleHelper {
public:
    static int         readInt(int minVal, int maxVal);
    static std::string readLine();
    static double      readDouble();

private:
    static void clearInputStream();
};

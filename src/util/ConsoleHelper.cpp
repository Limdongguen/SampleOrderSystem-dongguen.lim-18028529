#include "util/ConsoleHelper.h"
#include <iostream>
#include <limits>

namespace {
    constexpr auto kStreamMax = std::numeric_limits<std::streamsize>::max();
}

void ConsoleHelper::clearInputStream() {
    std::cin.clear();
    std::cin.ignore(kStreamMax, '\n');
}

int ConsoleHelper::readInt(int minVal, int maxVal) {
    int value{};
    while (true) {
        std::cin >> value;
        if (std::cin.fail() || value < minVal || value > maxVal) {
            clearInputStream();
        } else {
            std::cin.ignore(kStreamMax, '\n');
            return value;
        }
    }
}

std::string ConsoleHelper::readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

double ConsoleHelper::readDouble() {
    double value{};
    while (true) {
        std::cin >> value;
        if (std::cin.fail()) {
            clearInputStream();
        } else {
            std::cin.ignore(kStreamMax, '\n');
            return value;
        }
    }
}

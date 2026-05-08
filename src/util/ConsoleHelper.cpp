#include "util/ConsoleHelper.h"
#include <iostream>
#include <limits>

int ConsoleHelper::readInt(int minVal, int maxVal) {
    int value{};
    while (true) {
        std::cin >> value;
        if (std::cin.fail() || value < minVal || value > maxVal) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  [" << minVal << "~" << maxVal << "] 범위의 숫자를 입력하세요: ";
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  유효한 숫자를 입력하세요: ";
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

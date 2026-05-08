#pragma once
#include <string>

class MainView {
public:
    void printMenu() const;
    void printNotImplemented(const std::string& feature) const;
    void printExit() const;
};

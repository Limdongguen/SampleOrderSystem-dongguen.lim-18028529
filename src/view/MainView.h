#pragma once
#include "model/MenuSummary.h"
#include <string>

class MainView {
public:
    void printMenu() const;
    void printMenu(const MenuSummary& s) const;
    void printNotImplemented(const std::string& feature) const;
    void printExit() const;
};

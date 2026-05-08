#pragma once
#include "controller/SampleController.h"
#include <memory>

class MainController {
public:
    MainController();
    void run();

private:
    std::unique_ptr<SampleController> m_sampleController;

    void printMenu() const;
    int  readChoice() const;
    void handleChoice(int choice);
};

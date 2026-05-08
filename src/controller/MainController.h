#pragma once
#include "controller/SampleController.h"
#include "view/MainView.h"
#include <memory>

class MainController {
public:
    MainController();
    void run();

private:
    std::unique_ptr<SampleController> m_sampleController;
    MainView                          m_view;

    int  readChoice() const;
    void handleChoice(int choice);
};

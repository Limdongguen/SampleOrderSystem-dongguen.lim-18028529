#pragma once
#include "controller/SampleController.h"
#include "controller/OrderController.h"
#include "view/MainView.h"
#include <memory>

class MainController {
public:
    MainController();
    void run();

private:
    std::unique_ptr<SampleController> m_sampleController;
    std::unique_ptr<OrderController>  m_orderController;
    MainView                          m_view;

    int  readChoice() const;
    void handleChoice(int choice);
};

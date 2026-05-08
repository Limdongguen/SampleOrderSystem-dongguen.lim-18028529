#pragma once
#include "service/SampleService.h"
#include "view/SampleView.h"
#include <memory>

class SampleController {
public:
    SampleController();
    void run();

private:
    std::unique_ptr<SampleService> m_service;
    SampleView                     m_view;

    void handleRegister();
    void handleList() const;
    void handleSearch() const;
};

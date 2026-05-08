#include "controller/SampleController.h"
#include "util/ConsoleHelper.h"
#include <iostream>

namespace {
    constexpr int kMenuMin  = 0;
    constexpr int kMenuMax  = 3;
    constexpr int kMenuExit = 0;
}

SampleController::SampleController()
    : m_service(std::make_unique<SampleService>())
{
}

void SampleController::run() {
    int choice = -1;
    while (choice != kMenuExit) {
        m_view.printSubMenu();
        choice = ConsoleHelper::readInt(kMenuMin, kMenuMax);
        switch (choice) {
        case 1: handleRegister(); break;
        case 2: handleList();     break;
        case 3: handleSearch();   break;
        default: break;
        }
    }
}

void SampleController::handleRegister() {
    m_view.printRegisterForm();

    std::cout << "이름: ";
    std::string name = ConsoleHelper::readLine();

    std::cout << "평균 생산시간 (min/ea): ";
    double avgProdTime = ConsoleHelper::readDouble();

    std::cout << "수율 (0.0 ~ 1.0): ";
    double yield = ConsoleHelper::readDouble();

    if (m_service->registerSample(name, avgProdTime, yield)) {
        auto all = m_service->getAllSamples();
        if (!all.empty()) {
            m_view.printRegisterSuccess(all.back().sampleId);
        }
    } else {
        m_view.printError("입력값이 유효하지 않거나 중복된 이름입니다.");
    }
}

void SampleController::handleList() const {
    auto samples = m_service->getAllSamples();
    m_view.printList(samples);
}

void SampleController::handleSearch() const {
    m_view.printSearchPrompt();
    std::string keyword = ConsoleHelper::readLine();
    auto results = m_service->searchByName(keyword);
    if (results.empty()) {
        m_view.printNotFound();
    } else {
        m_view.printList(results);
    }
}

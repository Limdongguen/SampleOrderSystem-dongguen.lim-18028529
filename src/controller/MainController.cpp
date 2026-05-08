#include "controller/MainController.h"
#include "util/ConsoleHelper.h"

namespace {
    constexpr int kMenuMin  = 0;
    constexpr int kMenuMax  = 6;
    constexpr int kMenuExit = 0;
}

MainController::MainController()
    : m_sampleController(std::make_unique<SampleController>())
    , m_orderController(std::make_unique<OrderController>())
{
}

void MainController::run() {
    int choice = -1;
    while (choice != kMenuExit) {
        m_view.printMenu();
        choice = readChoice();
        handleChoice(choice);
    }
}

int MainController::readChoice() const {
    return ConsoleHelper::readInt(kMenuMin, kMenuMax);
}

void MainController::handleChoice(int choice) {
    switch (choice) {
    case 1: m_sampleController->run(); break;
    case 2: m_orderController->reserve(); break;
    case 3: m_view.printNotImplemented("[3] 주문 승인/거절"); break;
    case 4: m_view.printNotImplemented("[4] 모니터링"); break;
    case 5: m_view.printNotImplemented("[5] 생산라인 조회"); break;
    case 6: m_view.printNotImplemented("[6] 출고 처리"); break;
    case 0: m_view.printExit(); break;
    default: break;
    }
}

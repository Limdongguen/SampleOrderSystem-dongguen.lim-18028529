#include "controller/MonitoringController.h"
#include "util/ConsoleHelper.h"
#include <conio.h>
#include <chrono>
#include <thread>

namespace {
    constexpr int kMenuMin       = 0;
    constexpr int kMenuMax       = 2;
    constexpr int kMenuBack      = 0;
    constexpr int kMenuOrder     = 1;
    constexpr int kMenuStock     = 2;
    constexpr int kRefreshSec    = 60;
    constexpr int kPollMs        = 500;
    constexpr char kKeyBack      = '0';
    constexpr char kKeyRefreshLo = 'r';
    constexpr char kKeyRefreshHi = 'R';
}

MonitoringController::MonitoringController(
        std::shared_ptr<ProductionService> productionService)
    : m_monitoringService(std::make_unique<MonitoringService>())
    , m_productionService(std::move(productionService))
{
}

void MonitoringController::run() {
    int choice = -1;
    while (choice != kMenuBack) {
        m_view.printSubMenu();
        choice = readSubMenuChoice();
        if      (choice == kMenuOrder) runOrderSummary();
        else if (choice == kMenuStock) runStockStatus();
    }
}

int MonitoringController::readSubMenuChoice() const {
    return ConsoleHelper::readInt(kMenuMin, kMenuMax);
}

void MonitoringController::runOrderSummary() {
    runRealTime(true);
}

void MonitoringController::runStockStatus() {
    runRealTime(false);
}

void MonitoringController::runRealTime(bool isOrder) {
    using Clock = std::chrono::steady_clock;

    while (true) {
        m_productionService->tickCheck();
        if (isOrder) {
            m_view.printOrderSummary(m_monitoringService->getOrderSummary());
        } else {
            m_view.printStockStatusList(m_monitoringService->getStockStatusList());
        }

        auto deadline = Clock::now() + std::chrono::seconds(kRefreshSec);
        bool doRefresh = false;

        while (Clock::now() < deadline) {
            if (_kbhit()) {
                char ch = static_cast<char>(_getch());
                if (ch == kKeyBack) { m_view.clearCountdown(); return; }
                if (ch == kKeyRefreshLo || ch == kKeyRefreshHi) {
                    doRefresh = true;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(kPollMs));
            int remaining = static_cast<int>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    deadline - Clock::now()).count());
            m_view.printCountdown(remaining);
        }
        m_view.clearCountdown();
        if (!doRefresh && Clock::now() >= deadline) continue;
    }
}

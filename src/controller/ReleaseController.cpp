#include "controller/ReleaseController.h"
#include "util/ConsoleHelper.h"
#include <iostream>

namespace {
    constexpr int kExitChoice = 0;
}

ReleaseController::ReleaseController(
    std::shared_ptr<ReleaseService>    releaseService,
    std::shared_ptr<ProductionService> productionService
)
    : m_releaseService   (std::move(releaseService))
    , m_productionService(std::move(productionService))
{
}

void ReleaseController::run() {
    m_productionService->tickCheck();

    auto confirmed = m_releaseService->getConfirmedOrders();
    if (confirmed.empty()) {
        m_view.printNoConfirmedOrders();
        return;
    }

    m_view.printConfirmedList(confirmed);
    std::cout << "\n  번호 선택 (0: 뒤로): ";
    int choice = ConsoleHelper::readInt(kExitChoice, static_cast<int>(confirmed.size()));
    if (choice == kExitChoice) return;

    const Order& selected = confirmed[choice - 1];
    bool ok = m_releaseService->release(selected.orderId);
    if (ok) {
        m_view.printReleaseSuccess(selected.orderId, selected.quantity);
    } else {
        m_view.printInvalidSelection();
    }
}

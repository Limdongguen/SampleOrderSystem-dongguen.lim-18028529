#include "controller/ProductionController.h"
#include "util/ConsoleHelper.h"
#include "util/TimeUtil.h"
#include <iostream>

namespace {
    constexpr int kExitChoice = 0;
    constexpr int kMenuMin    = 0;
    constexpr int kMenuMax    = 0;
}

ProductionController::ProductionController(std::shared_ptr<ProductionService> productionService)
    : m_productionService(std::move(productionService))
{
}

void ProductionController::run() {
    m_productionService->tickCheck();

    auto currentJob  = m_productionService->getCurrentJob();
    auto waitingJobs = m_productionService->getWaitingJobs();

    if (currentJob.has_value()) {
        int secondsLeft = TimeUtil::secondsUntil(currentJob->estimatedEndTime);
        m_view.printCurrentJob(currentJob.value(), secondsLeft);
    } else {
        m_view.printNoJobRunning();
    }

    m_view.printWaitingQueue(waitingJobs);

    std::cout << "\n  [0] 뒤로\n  선택: ";
    ConsoleHelper::readInt(kMenuMin, kMenuMax);
}

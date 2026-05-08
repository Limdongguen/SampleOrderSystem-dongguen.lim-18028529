#include "controller/MainController.h"
#include "util/ConsoleHelper.h"
#include "util/TimeUtil.h"

namespace {
    constexpr int kMenuMin  = 0;
    constexpr int kMenuMax  = 6;
    constexpr int kMenuExit = 0;
}

MainController::MainController()
    : m_productionService(std::make_shared<ProductionService>())
    , m_releaseService   (std::make_shared<ReleaseService>())
    , m_sampleController (std::make_unique<SampleController>())
    , m_orderController  (std::make_unique<OrderController>())
    , m_approvalController(std::make_unique<ApprovalController>())
    , m_monitoringController(std::make_unique<MonitoringController>(m_productionService))
    , m_productionController(std::make_unique<ProductionController>(m_productionService))
    , m_releaseController(std::make_unique<ReleaseController>(m_releaseService, m_productionService))
{
}

void MainController::run() {
    int choice = -1;
    while (choice != kMenuExit) {
        m_productionService->tickCheck();
        MenuSummary summary = collectSummary();
        m_view.printMenu(summary);
        choice = readChoice();
        handleChoice(choice);
    }
}

int MainController::readChoice() const {
    return ConsoleHelper::readInt(kMenuMin, kMenuMax);
}

void MainController::handleChoice(int choice) {
    switch (choice) {
    case 1: m_sampleController->run();          break;
    case 2: m_orderController->reserve();       break;
    case 3: m_approvalController->run();        break;
    case 4: m_monitoringController->run();      break;
    case 5: m_productionController->run();      break;
    case 6: m_releaseController->run();         break;
    case 0: m_view.printExit();                 break;
    default: break;
    }
}

MenuSummary MainController::collectSummary() const {
    SampleService  sampleSvc;
    OrderService   orderSvc;
    MonitoringService monSvc;

    auto samples = sampleSvc.getAllSamples();
    int totalStock = 0;
    for (const auto& s : samples) totalStock += s.stock;

    auto summary = monSvc.getOrderSummary();
    MenuSummary ms;
    ms.currentTime   = TimeUtil::nowString();
    ms.sampleCount   = static_cast<int>(samples.size());
    ms.totalStock    = totalStock;
    ms.orderCount    = summary.total;
    ms.producingCount = summary.producing;
    return ms;
}

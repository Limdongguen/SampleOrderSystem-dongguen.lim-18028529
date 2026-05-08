#include "controller/MainController.h"
#include "util/ConsoleHelper.h"

namespace {
    constexpr int kMenuMin          = 0;
    constexpr int kMenuMax          = 7;
    constexpr int kMenuExit         = 0;
    constexpr int kOrderIdDate      = 4;  // "ORD-" 이후 날짜 시작 위치
    constexpr int kOrderIdSeq       = 13; // "ORD-YYYYMMDD-" 이후 시퀀스 위치
    constexpr int kOrderIdMin       = 17; // ORD-YYYYMMDD-NNNN 최소 길이
    constexpr int kSampleIdMin      = 3;  // "S-N" 최소 길이
    constexpr int kSampleIdPrefixLen= 2;  // "S-" 길이
    constexpr int kDateYearOffset   = 0;  // 타임스탬프에서 연도 시작 위치
    constexpr int kDateYearLen      = 4;  // 연도 자릿수
    constexpr int kDateMonOffset    = 5;  // 타임스탬프에서 월 시작 위치
    constexpr int kDateMonLen       = 2;  // 월 자릿수
    constexpr int kDateDayOffset    = 8;  // 타임스탬프에서 일 시작 위치
    constexpr int kDateDayLen       = 2;  // 일 자릿수
    constexpr int kDateTotalLen     = 8;  // YYYYMMDD 길이 (연4+월2+일2)
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
    syncIdGenerator(); // 기존 데이터와 시퀀스 동기화
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
    case 7: handleDummyGen();                   break;
    case 0: m_view.printExit();                 break;
    default: break;
    }
}

void MainController::handleDummyGen() {
    m_view.printDummyGenConfirm();
    DummyGen gen;
    if (gen.generate()) {
        reloadAll();       // 모든 서비스·컨트롤러를 새 JSON 데이터로 재초기화
        syncIdGenerator(); // ID 시퀀스를 기존 데이터 최댓값으로 동기화
        m_view.printDummyGenSuccess();
    } else {
        m_view.printDummyGenFail();
    }
}

void MainController::reloadAll() {
    m_productionService    = std::make_shared<ProductionService>();
    m_releaseService       = std::make_shared<ReleaseService>();
    m_sampleController     = std::make_unique<SampleController>();
    m_orderController      = std::make_unique<OrderController>();
    m_approvalController   = std::make_unique<ApprovalController>();
    m_monitoringController = std::make_unique<MonitoringController>(m_productionService);
    m_productionController = std::make_unique<ProductionController>(m_productionService);
    m_releaseController    = std::make_unique<ReleaseController>(m_releaseService, m_productionService);
}

void MainController::syncIdGenerator() {
    // 시료 ID 동기화: S-NNN 형식에서 최대 순번 탐색
    auto samplesJson = JsonFileManager::load("data/samples.json");
    int maxSample = 0;
    for (const auto& item : samplesJson) {
        std::string id = item.value("sampleId", "");
        if (static_cast<int>(id.size()) >= kSampleIdMin) {
            try { maxSample = std::max(maxSample, std::stoi(id.substr(kSampleIdPrefixLen))); }
            catch (...) {}
        }
    }
    IdGenerator::initSampleSeq(maxSample);

    // 주문 ID 동기화: ORD-YYYYMMDD-NNNN 형식에서 오늘 날짜 최대 순번 탐색
    const std::string now   = TimeUtil::nowString();
    const std::string today = now.substr(kDateYearOffset, kDateYearLen)
                           + now.substr(kDateMonOffset,  kDateMonLen)
                           + now.substr(kDateDayOffset,  kDateDayLen);
    auto ordersJson = JsonFileManager::load("data/orders.json");
    int maxOrder = 0;
    for (const auto& item : ordersJson) {
        std::string id = item.value("orderId", "");
        if (static_cast<int>(id.size()) >= kOrderIdMin &&
            id.substr(kOrderIdDate, kDateTotalLen) == today) {
            try { maxOrder = std::max(maxOrder, std::stoi(id.substr(kOrderIdSeq))); }
            catch (...) {}
        }
    }
    IdGenerator::initOrderSeq(maxOrder);
}

MenuSummary MainController::collectSummary() const {
    SampleService     sampleSvc;
    MonitoringService monSvc;

    auto samples = sampleSvc.getAllSamples();
    int totalStock = 0;
    for (const auto& s : samples) totalStock += s.stock;

    auto summary = monSvc.getOrderSummary();
    MenuSummary ms;
    ms.currentTime    = TimeUtil::nowString();
    ms.sampleCount    = static_cast<int>(samples.size());
    ms.totalStock     = totalStock;
    ms.orderCount     = summary.total;
    ms.producingCount = summary.producing;
    return ms;
}

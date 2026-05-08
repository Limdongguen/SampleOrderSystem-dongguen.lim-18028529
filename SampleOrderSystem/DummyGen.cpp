#include "DummyGen.h"
#include "util/JsonFileManager.h"
#include "util/TimeUtil.h"
#include "model/Sample.h"
#include "model/Order.h"
#include "model/ProductionJob.h"
#include "model/Enums.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <vector>
#include <cmath>

namespace {
    // 시연용 avgProdTime 범위 (분): 0.05 ~ 0.20
    constexpr double kMinAvgProdTime = 0.05;
    constexpr double kMaxAvgProdTime = 0.20;
    // 수율 범위
    constexpr double kMinYield = 0.85;
    constexpr double kMaxYield = 0.98;
    // RUNNING Job의 estimatedEndTime 오프셋 (분)
    constexpr double kRunningJobOffsetMin = 2.0;
    // 생산량 공식 yield 버퍼
    constexpr double kYieldBuffer = 0.9;

    // 10종 실제 반도체 소재명
    const std::vector<std::string> kSampleNames = {
        "실리콘 웨이퍼-8인치",
        "GaN 에피택셜-4인치",
        "SiC 파워기판-6인치",
        "포토레지스트-PR7",
        "산화막 웨이퍼-SiO2",
        "HfO2 게이트유전체",
        "Cu 배선재료",
        "Low-k 절연막",
        "Si3N4 질화막",
        "Al2O3 ALD막"
    };

    // avgProdTime 값 (10개 고정, 0.05 ~ 0.20 범위)
    const std::vector<double> kAvgProdTimes = {
        0.10, 0.15, 0.20, 0.05, 0.12,
        0.08, 0.18, 0.07, 0.13, 0.16
    };

    // yield 값 (10개 고정, 0.85 ~ 0.98 범위)
    const std::vector<double> kYields = {
        0.92, 0.88, 0.95, 0.85, 0.97,
        0.90, 0.98, 0.86, 0.93, 0.91
    };

    // 초기 재고
    const std::vector<int> kStocks = {
        500, 300, 0,   150, 200,
        80,  0,   400, 250, 100
    };
}

DummyGen::DummyGen(
    const std::string& samplesPath,
    const std::string& ordersPath,
    const std::string& productionPath
)
    : m_samplesPath(samplesPath)
    , m_ordersPath(ordersPath)
    , m_productionPath(productionPath)
{}

bool DummyGen::generate() {
    generateSamples();
    generateOrders();
    generateProductionJobs();
    printSummary();
    return true;
}

void DummyGen::generateSamples() {
    std::vector<Sample> samples;
    for (int i = 0; i < 10; ++i) {
        Sample s;
        s.sampleId   = "S-" + std::string(2 - std::to_string(i+1).size(), '0') + std::to_string(i+1);
        s.name        = kSampleNames[i];
        s.avgProdTime = kAvgProdTimes[i];
        s.yield       = kYields[i];
        s.stock       = kStocks[i];
        samples.push_back(s);
    }
    nlohmann::json j = samples;
    JsonFileManager::save(m_samplesPath, j);
}

void DummyGen::generateOrders() {
    // 상태 분포: RESERVED 3, CONFIRMED 5, PRODUCING 3, RELEASED 8, REJECTED 1
    // 총 20건
    struct OrderSpec {
        std::string sampleId;
        std::string customerName;
        int         quantity;
        OrderStatus status;
        std::string orderId;
        std::string createdAt;
        std::string updatedAt;
    };

    const std::string kBaseTime = "2026-05-08 09:00:00";

    // orderId 형식: ORD-YYYYMMDD-NNNN
    auto makeOrderId = [](int seq) -> std::string {
        std::string seqStr = std::to_string(seq);
        while (seqStr.size() < 4) seqStr = "0" + seqStr;
        return "ORD-20260508-" + seqStr;
    };

    std::vector<OrderSpec> specs = {
        // RESERVED 3건
        { "S-001", "삼성전자",  100, OrderStatus::RESERVED,  makeOrderId(1),  kBaseTime, kBaseTime },
        { "S-002", "SK하이닉스", 50, OrderStatus::RESERVED,  makeOrderId(2),  kBaseTime, kBaseTime },
        { "S-003", "LG이노텍",   80, OrderStatus::RESERVED,  makeOrderId(3),  kBaseTime, kBaseTime },
        // CONFIRMED 5건
        { "S-004", "인텔코리아", 150, OrderStatus::CONFIRMED, makeOrderId(4),  kBaseTime, kBaseTime },
        { "S-005", "TSMC한국",   200, OrderStatus::CONFIRMED, makeOrderId(5),  kBaseTime, kBaseTime },
        { "S-006", "마이크론",    60, OrderStatus::CONFIRMED, makeOrderId(6),  kBaseTime, kBaseTime },
        { "S-007", "퀄컴코리아",  40, OrderStatus::CONFIRMED, makeOrderId(7),  kBaseTime, kBaseTime },
        { "S-008", "AMD코리아",   90, OrderStatus::CONFIRMED, makeOrderId(8),  kBaseTime, kBaseTime },
        // PRODUCING 3건
        { "S-003", "엔비디아",   500, OrderStatus::PRODUCING, makeOrderId(9),  kBaseTime, kBaseTime },
        { "S-006", "아날로그D",  300, OrderStatus::PRODUCING, makeOrderId(10), kBaseTime, kBaseTime },
        { "S-009", "TI코리아",   200, OrderStatus::PRODUCING, makeOrderId(11), kBaseTime, kBaseTime },
        // RELEASED 8건
        { "S-001", "한화에어로", 120, OrderStatus::RELEASED,  makeOrderId(12), kBaseTime, kBaseTime },
        { "S-002", "두산전자",    70, OrderStatus::RELEASED,  makeOrderId(13), kBaseTime, kBaseTime },
        { "S-004", "현대모비스",  85, OrderStatus::RELEASED,  makeOrderId(14), kBaseTime, kBaseTime },
        { "S-005", "LG전자",     110, OrderStatus::RELEASED,  makeOrderId(15), kBaseTime, kBaseTime },
        { "S-007", "삼성SDI",     45, OrderStatus::RELEASED,  makeOrderId(16), kBaseTime, kBaseTime },
        { "S-008", "SK이노베이션",130, OrderStatus::RELEASED, makeOrderId(17), kBaseTime, kBaseTime },
        { "S-009", "코오롱인더",  95, OrderStatus::RELEASED,  makeOrderId(18), kBaseTime, kBaseTime },
        { "S-010", "OCI",         75, OrderStatus::RELEASED,  makeOrderId(19), kBaseTime, kBaseTime },
        // REJECTED 1건
        { "S-001", "중소벤처A",  999, OrderStatus::REJECTED,  makeOrderId(20), kBaseTime, kBaseTime },
    };

    std::vector<Order> orders;
    for (const auto& sp : specs) {
        Order o;
        o.orderId      = sp.orderId;
        o.sampleId     = sp.sampleId;
        o.customerName = sp.customerName;
        o.quantity     = sp.quantity;
        o.status       = sp.status;
        o.createdAt    = sp.createdAt;
        o.updatedAt    = sp.updatedAt;
        orders.push_back(o);
    }

    nlohmann::json j = orders;
    JsonFileManager::save(m_ordersPath, j);
}

void DummyGen::generateProductionJobs() {
    // PRODUCING 주문 3건과 1:1 매핑 (orderId 9, 10, 11)
    // 1개 RUNNING + 2개 WAITING
    // RUNNING Job: estimatedEndTime = 현재 + 2분
    auto makeJobId = [](const std::string& orderId) -> std::string {
        return "JOB-" + orderId;
    };

    const std::string kRunningEnd = TimeUtil::addMinutes(TimeUtil::nowString(), kRunningJobOffsetMin);

    // 시료 수율/avgProdTime 참조
    // S-003: yield=0.95, avgProdTime=0.20, stock=0
    // S-006: yield=0.90, avgProdTime=0.08, stock=80
    // S-009: yield=0.93, avgProdTime=0.13, stock=250

    // shortage = quantity - stock
    // actualProduction = ceil(shortage / (yield * 0.9))
    auto calcActual = [](int shortage, double yield) -> int {
        return (int)std::ceil(shortage / (yield * kYieldBuffer));
    };

    // ORD-20260508-0009: S-003, qty=500, stock=0
    int sh9 = 500 - 0;   // shortage=500
    int ap9 = calcActual(sh9, 0.95);
    double tt9 = 0.20 * ap9;

    // ORD-20260508-0010: S-006, qty=300, stock=80
    int sh10 = 300 - 80; // shortage=220
    int ap10 = calcActual(sh10, 0.90);
    double tt10 = 0.08 * ap10;

    // ORD-20260508-0011: S-009, qty=200, stock=250
    // stock >= qty이지만 PRODUCING으로 설정 (더미 데이터 시나리오)
    int sh11 = 200 - 0; // 시연 목적 shortage
    int ap11 = calcActual(sh11, 0.93);
    double tt11 = 0.13 * ap11;

    std::vector<ProductionJob> jobs;

    // RUNNING job (첫 번째)
    ProductionJob job1;
    job1.jobId            = makeJobId("ORD-20260508-0009");
    job1.orderId          = "ORD-20260508-0009";
    job1.sampleId         = "S-003";
    job1.shortage         = sh9;
    job1.actualProduction = ap9;
    job1.totalTime        = tt9;
    job1.status           = JobStatus::RUNNING;
    job1.estimatedEndTime = kRunningEnd;
    jobs.push_back(job1);

    // WAITING job 1 (두 번째)
    ProductionJob job2;
    job2.jobId            = makeJobId("ORD-20260508-0010");
    job2.orderId          = "ORD-20260508-0010";
    job2.sampleId         = "S-006";
    job2.shortage         = sh10;
    job2.actualProduction = ap10;
    job2.totalTime        = tt10;
    job2.status           = JobStatus::WAITING;
    job2.estimatedEndTime = "";
    jobs.push_back(job2);

    // WAITING job 2 (세 번째)
    ProductionJob job3;
    job3.jobId            = makeJobId("ORD-20260508-0011");
    job3.orderId          = "ORD-20260508-0011";
    job3.sampleId         = "S-009";
    job3.shortage         = sh11;
    job3.actualProduction = ap11;
    job3.totalTime        = tt11;
    job3.status           = JobStatus::WAITING;
    job3.estimatedEndTime = "";
    jobs.push_back(job3);

    nlohmann::json j = jobs;
    JsonFileManager::save(m_productionPath, j);
}

void DummyGen::printSummary() const {
    std::cout << "\n=== 더미 데이터 생성 완료 ===\n";
    std::cout << "  시료:       10종 -> " << m_samplesPath     << "\n";
    std::cout << "  주문:       20건 -> " << m_ordersPath      << "\n";
    std::cout << "  생산 작업:   3건 -> " << m_productionPath  << "\n";
    std::cout << "  (RUNNING 1개 / WAITING 2개)\n\n";
}

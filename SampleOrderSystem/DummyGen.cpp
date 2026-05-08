#include "DummyGen.h"
#include "util/JsonFileManager.h"
#include "util/TimeUtil.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {
    constexpr int    kSampleCount          = 10;
    constexpr int    kOrderCount           = 20;
    constexpr int    kJobCount             = 3;
    constexpr double kRunningJobOffsetMin  = 2.0;  // RUNNING Job의 완료 예정까지 (분)
    constexpr double kYieldBuffer          = 0.9;
    constexpr int    kSampleIdPadWidth     = 3;
    constexpr int    kOrderSeqPadWidth     = 4;
    const std::string kOrderDatePrefix     = "ORD-20260508-";
    const std::string kDummyTimestamp      = "2026-05-08 09:00:00";

    const std::string kSampleNames[kSampleCount] = {
        "실리콘 웨이퍼-8인치", "GaN 에피택셜-4인치", "SiC 파워기판-6인치",
        "포토레지스트-PR7",    "산화막 웨이퍼-SiO2", "HfO2 게이트유전체",
        "Cu 배선재료",         "Low-k 절연막",        "Si3N4 질화막",
        "Al2O3 ALD막"
    };
    const double kAvgProdTimes[kSampleCount] = {
        0.10, 0.15, 0.20, 0.05, 0.12, 0.08, 0.18, 0.07, 0.13, 0.16
    };
    const double kYieldValues[kSampleCount] = {
        0.92, 0.88, 0.95, 0.85, 0.97, 0.90, 0.98, 0.86, 0.93, 0.91
    };
    const int kStocks[kSampleCount] = {
        500, 300, 0, 150, 200, 80, 0, 400, 250, 100
    };
}

// ─── 헬퍼 ──────────────────────────────────────────────────────────────────

std::string DummyGen::makeSampleId(int index) {
    // index: 0-based → "S-001" ~ "S-010"
    std::ostringstream oss;
    oss << "S-" << std::setfill('0') << std::setw(kSampleIdPadWidth) << (index + 1);
    return oss.str();
}

std::string DummyGen::makeOrderId(int seq) {
    std::ostringstream oss;
    oss << kOrderDatePrefix << std::setfill('0') << std::setw(kOrderSeqPadWidth) << seq;
    return oss.str();
}

Order DummyGen::buildOrder(
    const std::string& sampleId, const std::string& customerName,
    int quantity, OrderStatus status, const std::string& orderId,
    const std::string& timestamp)
{
    Order o;
    o.orderId      = orderId;
    o.sampleId     = sampleId;
    o.customerName = customerName;
    o.quantity     = quantity;
    o.status       = status;
    o.createdAt    = timestamp;
    o.updatedAt    = timestamp;
    return o;
}

ProductionJob DummyGen::buildJob(
    const std::string& orderId, const std::string& sampleId,
    int shortage, double yield, double avgProdTime,
    JobStatus status, const std::string& estimatedEndTime)
{
    int actualProduction = (int)std::ceil(shortage / (yield * kYieldBuffer));
    ProductionJob j;
    j.jobId            = "JOB-" + orderId;
    j.orderId          = orderId;
    j.sampleId         = sampleId;
    j.shortage         = shortage;
    j.actualProduction = actualProduction;
    j.totalTime        = avgProdTime * actualProduction;
    j.status           = status;
    j.estimatedEndTime = estimatedEndTime;
    return j;
}

// ─── 생성 메서드 ────────────────────────────────────────────────────────────

DummyGen::DummyGen(
    const std::string& samplesPath,
    const std::string& ordersPath,
    const std::string& productionPath)
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
    for (int i = 0; i < kSampleCount; ++i) {
        Sample s;
        s.sampleId   = makeSampleId(i);
        s.name        = kSampleNames[i];
        s.avgProdTime = kAvgProdTimes[i];
        s.yield       = kYieldValues[i];
        s.stock       = kStocks[i];
        samples.push_back(s);
    }
    nlohmann::json j = samples;
    JsonFileManager::save(m_samplesPath, j);
}

void DummyGen::generateOrders() {
    // 상태 분포: RESERVED 3, CONFIRMED 5, PRODUCING 3, RELEASED 8, REJECTED 1 (총 20건)
    const std::string& ts = kDummyTimestamp;
    using OS = OrderStatus;
    std::vector<Order> orders = {
        buildOrder("S-001", "삼성전자",    100, OS::RESERVED,  makeOrderId(1),  ts),
        buildOrder("S-002", "SK하이닉스",   50, OS::RESERVED,  makeOrderId(2),  ts),
        buildOrder("S-003", "LG이노텍",     80, OS::RESERVED,  makeOrderId(3),  ts),
        buildOrder("S-004", "인텔코리아",  150, OS::CONFIRMED, makeOrderId(4),  ts),
        buildOrder("S-005", "TSMC한국",    200, OS::CONFIRMED, makeOrderId(5),  ts),
        buildOrder("S-006", "마이크론",     60, OS::CONFIRMED, makeOrderId(6),  ts),
        buildOrder("S-007", "퀄컴코리아",   40, OS::CONFIRMED, makeOrderId(7),  ts),
        buildOrder("S-008", "AMD코리아",    90, OS::CONFIRMED, makeOrderId(8),  ts),
        buildOrder("S-003", "엔비디아",    500, OS::PRODUCING, makeOrderId(9),  ts),
        buildOrder("S-006", "아날로그D",   300, OS::PRODUCING, makeOrderId(10), ts),
        buildOrder("S-009", "TI코리아",    200, OS::PRODUCING, makeOrderId(11), ts),
        buildOrder("S-001", "한화에어로",  120, OS::RELEASED,  makeOrderId(12), ts),
        buildOrder("S-002", "두산전자",     70, OS::RELEASED,  makeOrderId(13), ts),
        buildOrder("S-004", "현대모비스",   85, OS::RELEASED,  makeOrderId(14), ts),
        buildOrder("S-005", "LG전자",      110, OS::RELEASED,  makeOrderId(15), ts),
        buildOrder("S-007", "삼성SDI",      45, OS::RELEASED,  makeOrderId(16), ts),
        buildOrder("S-008", "SK이노베이션",130, OS::RELEASED,  makeOrderId(17), ts),
        buildOrder("S-009", "코오롱인더",   95, OS::RELEASED,  makeOrderId(18), ts),
        buildOrder("S-010", "OCI",          75, OS::RELEASED,  makeOrderId(19), ts),
        buildOrder("S-001", "중소벤처A",   999, OS::REJECTED,  makeOrderId(20), ts),
    };
    nlohmann::json j = orders;
    JsonFileManager::save(m_ordersPath, j);
}

void DummyGen::generateProductionJobs() {
    // PRODUCING 주문 3건과 1:1 매핑 (orderId 9, 10, 11)
    // 1개 RUNNING + 2개 WAITING
    const std::string kRunningEnd =
        TimeUtil::addMinutes(TimeUtil::nowString(), kRunningJobOffsetMin);

    // S-003: stock=0,  qty=500 → shortage=500, yield=0.95, avgProdTime=0.20
    // S-006: stock=80, qty=300 → shortage=220, yield=0.90, avgProdTime=0.08
    // S-009: stock=0,  qty=200 → shortage=200, yield=0.93, avgProdTime=0.13
    std::vector<ProductionJob> jobs = {
        buildJob(makeOrderId(9),  "S-003", 500, 0.95, 0.20, JobStatus::RUNNING,  kRunningEnd),
        buildJob(makeOrderId(10), "S-006", 220, 0.90, 0.08, JobStatus::WAITING,  ""),
        buildJob(makeOrderId(11), "S-009", 200, 0.93, 0.13, JobStatus::WAITING,  ""),
    };
    nlohmann::json j = jobs;
    JsonFileManager::save(m_productionPath, j);
}

void DummyGen::printSummary() const {
    std::cout << "\n=== 더미 데이터 생성 완료 ===\n"
              << "  시료:       " << kSampleCount << "종 -> " << m_samplesPath    << "\n"
              << "  주문:       " << kOrderCount  << "건 -> " << m_ordersPath     << "\n"
              << "  생산 작업:  " << kJobCount    << "건 -> " << m_productionPath << "\n"
              << "  (RUNNING 1개 / WAITING 2개)\n\n";
}

#include "gmock/gmock.h"
#include "service/SampleService.h"
#include "service/OrderService.h"
#include "service/ProductionService.h"
#include "service/ReleaseService.h"
#include "service/MonitoringService.h"
#include "model/MonitoringTypes.h"
#include "util/IdGenerator.h"
#include "util/TimeUtil.h"
#include "DummyGen.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;

namespace {
    const std::string kS = "data/rt_samples.json";
    const std::string kO = "data/rt_orders.json";
    const std::string kP = "data/rt_production.json";
}

class RegressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        IdGenerator::reset();
        fs::remove(kS); fs::remove(kO); fs::remove(kP);
    }
    void TearDown() override {
        fs::remove(kS); fs::remove(kO); fs::remove(kP);
    }
};

// 시나리오 A: DummyGen → 시료 추가 → 주문 → 승인 → 출고/생산 확인
TEST_F(RegressionTest, ScenarioA_FullWorkflow) {
    // 1. 더미 데이터 생성
    DummyGen gen(kS, kO, kP);
    ASSERT_TRUE(gen.generate()) << "DummyGen 실패";

    // 2. 시료 추가 (인스턴스 A)
    SampleService svcA(kS);
    ASSERT_TRUE(svcA.registerSample("RT-웨이퍼", 0.1, 0.95)) << "시료 등록 실패";

    // 3. 다른 인스턴스(B)에서 새 시료 조회 — 크로스 서비스 버그 검증
    SampleService svcB(kS);
    auto hits = svcB.searchByName("RT-웨이퍼");
    ASSERT_EQ(hits.size(), 1u) << "크로스 인스턴스 조회 실패: 새 시료가 보이지 않음";
    const std::string newId = hits[0].sampleId;

    // 4. 주문 접수 (OrderService 생성자: samplePath, orderPath, productionPath)
    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(newId, "회귀테스트-고객", 1)) << "주문 접수 실패";

    auto reserved = orderSvc.getReservedOrders();
    // DummyGen이 만든 RESERVED 3건 + 방금 접수한 1건
    bool found = std::any_of(reserved.begin(), reserved.end(),
        [&](const Order& o){ return o.sampleId == newId; });
    ASSERT_TRUE(found) << "접수된 주문이 RESERVED 목록에 없음";
    const std::string orderId = (*std::find_if(reserved.begin(), reserved.end(),
        [&](const Order& o){ return o.sampleId == newId; })).orderId;

    // 5. 승인
    ASSERT_TRUE(orderSvc.approve(orderId)) << "주문 승인 실패";

    // 6. 승인 후 상태: CONFIRMED(재고 충분) 또는 PRODUCING(재고 부족)
    auto all = orderSvc.getAllOrders();
    auto it = std::find_if(all.begin(), all.end(),
        [&](const Order& o){ return o.orderId == orderId; });
    ASSERT_NE(it, all.end());
    bool validStatus = (it->status == OrderStatus::CONFIRMED ||
                        it->status == OrderStatus::PRODUCING);
    EXPECT_TRUE(validStatus) << "승인 후 상태가 CONFIRMED/PRODUCING 이어야 함";

    // 7. 모니터링 조회
    MonitoringService monSvc(kS, kO);
    auto summary = monSvc.getOrderSummary();
    EXPECT_GT(summary.total, 0) << "모니터링 주문 집계가 0";
    auto stockList = monSvc.getStockStatusList();
    EXPECT_FALSE(stockList.empty()) << "모니터링 재고 목록 비어있음";

    // 8. 생산라인 조회
    ProductionService prodSvc(kP, kS, kO);
    // CONFIRMED이면 생산큐 없을 수 있음. PRODUCING이면 큐에 있어야 함
    if (it->status == OrderStatus::PRODUCING) {
        auto current = prodSvc.getCurrentJob();
        EXPECT_TRUE(current.has_value()) << "PRODUCING 상태인데 생산 큐가 비어있음";
    }

    // 9. CONFIRMED이면 출고
    if (it->status == OrderStatus::CONFIRMED) {
        ReleaseService relSvc(kO, kS);
        EXPECT_TRUE(relSvc.release(orderId)) << "출고 처리 실패";
    }
}

// 시나리오 B: REJECTED 주문이 모니터링 집계에서 제외
TEST_F(RegressionTest, ScenarioB_RejectedExcludedFromMonitoring) {
    DummyGen gen(kS, kO, kP);
    ASSERT_TRUE(gen.generate());

    MonitoringService monSvc(kS, kO);
    auto summary = monSvc.getOrderSummary();
    // 더미: RESERVED 3, CONFIRMED 5, PRODUCING 3, RELEASED 8, REJECTED 1 → 합계 19
    EXPECT_EQ(summary.total, 19) << "REJECTED 1건 제외 후 합계 19 기대";
}

// 시나리오 C: DummyGen 후 RESERVED 목록 정상 조회
TEST_F(RegressionTest, ScenarioC_ReservedOrdersAfterDummyGen) {
    DummyGen gen(kS, kO, kP);
    ASSERT_TRUE(gen.generate());

    OrderService orderSvc(kS, kO, kP);
    auto reserved = orderSvc.getReservedOrders();
    EXPECT_EQ(static_cast<int>(reserved.size()), 3) << "더미 RESERVED 3건 기대";
}

// 시나리오 D: DummyGen 후 생산 RUNNING Job 조회
TEST_F(RegressionTest, ScenarioD_ProductionRunningJobAfterDummyGen) {
    DummyGen gen(kS, kO, kP);
    ASSERT_TRUE(gen.generate());

    ProductionService prodSvc(kP, kS, kO);
    auto current = prodSvc.getCurrentJob();
    EXPECT_TRUE(current.has_value()) << "RUNNING Job이 있어야 함";
    if (current.has_value()) {
        EXPECT_EQ(current->status, JobStatus::RUNNING);
        EXPECT_FALSE(current->estimatedEndTime.empty());
    }
}

// 시나리오 E: 크로스 서비스 — 한 서비스가 저장한 시료를 다른 서비스가 주문에 사용
TEST_F(RegressionTest, ScenarioE_CrossService_NewSampleOrderable) {
    DummyGen gen(kS, kO, kP);
    ASSERT_TRUE(gen.generate());

    // SampleService 인스턴스 A로 등록
    { SampleService svcA(kS); ASSERT_TRUE(svcA.registerSample("크로스-시료", 0.05, 0.90)); }

    // OrderService(별도 인스턴스)로 주문 — 핵심 버그 재현 지점
    OrderService orderSvc(kS, kO, kP);
    SampleService svcB(kS);
    auto found = svcB.searchByName("크로스-시료");
    ASSERT_EQ(found.size(), 1u);

    bool ok = orderSvc.reserve(found[0].sampleId, "크로스-고객", 1);
    EXPECT_TRUE(ok) << "새로 추가된 시료로 주문 접수 실패 (크로스 서비스 버그)";
}

// ============================================================
// F. 시료 입력 유효성 검증
// ============================================================

TEST_F(RegressionTest, F1_EmptyName_RegisterFails) {
    SampleService svc(kS);
    bool result = svc.registerSample("", 0.1, 0.9);
    EXPECT_FALSE(result) << "빈 이름 시료 등록은 실패해야 함";
}

TEST_F(RegressionTest, F2_ZeroYield_RegisterFails) {
    SampleService svc(kS);
    bool result = svc.registerSample("테스트-시료", 0.1, 0.0);
    EXPECT_FALSE(result) << "수율 0.0 시료 등록은 실패해야 함";
}

TEST_F(RegressionTest, F3_YieldExceedsOne_RegisterFails) {
    SampleService svc(kS);
    bool result = svc.registerSample("테스트-시료", 0.1, 1.1);
    EXPECT_FALSE(result) << "수율 1.0 초과 시료 등록은 실패해야 함";
}

TEST_F(RegressionTest, F4_ZeroProdTime_RegisterFails) {
    SampleService svc(kS);
    bool result = svc.registerSample("테스트-시료", 0.0, 0.9);
    EXPECT_FALSE(result) << "생산시간 0 이하 시료 등록은 실패해야 함";
}

TEST_F(RegressionTest, F5_DuplicateName_RegisterFails) {
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("중복-시료", 0.1, 0.9));
    bool result = svc.registerSample("중복-시료", 0.2, 0.8);
    EXPECT_FALSE(result) << "중복 이름 시료 등록은 실패해야 함";
}

// ============================================================
// G. 주문 입력 유효성 검증
// ============================================================

TEST_F(RegressionTest, G1_NonExistentSampleId_ReserveFails) {
    OrderService orderSvc(kS, kO, kP);
    bool result = orderSvc.reserve("S-999", "테스트-고객", 10);
    EXPECT_FALSE(result) << "존재하지 않는 sampleId 주문 접수는 실패해야 함";
}

TEST_F(RegressionTest, G2_ZeroQuantity_ReserveFails) {
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("G2-시료", 0.1, 0.9));
    auto found = svc.searchByName("G2-시료");
    ASSERT_EQ(found.size(), 1u);

    OrderService orderSvc(kS, kO, kP);
    bool result = orderSvc.reserve(found[0].sampleId, "테스트-고객", 0);
    EXPECT_FALSE(result) << "수량 0 주문 접수는 실패해야 함";
}

TEST_F(RegressionTest, G3_NegativeQuantity_ReserveFails) {
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("G3-시료", 0.1, 0.9));
    auto found = svc.searchByName("G3-시료");
    ASSERT_EQ(found.size(), 1u);

    OrderService orderSvc(kS, kO, kP);
    bool result = orderSvc.reserve(found[0].sampleId, "테스트-고객", -5);
    EXPECT_FALSE(result) << "수량 음수 주문 접수는 실패해야 함";
}

// ============================================================
// H. 주문 상태 전이 검증
// ============================================================

TEST_F(RegressionTest, H1_SufficientStock_ReservedToConfirmed) {
    // 재고 충분 시나리오: stock >= quantity → CONFIRMED
    // DummyGen으로 재고 있는 시료 확보
    DummyGen gen(kS, kO, kP);
    ASSERT_TRUE(gen.generate());

    SampleService svc(kS);
    auto allSamples = svc.getAllSamples();
    bool found = false;
    std::string sampleId;
    for (const auto& s : allSamples) {
        if (s.stock >= 10) { sampleId = s.sampleId; found = true; break; }
    }
    ASSERT_TRUE(found) << "재고 있는 시료가 없음";

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "H1-고객", 1));
    auto reserved = orderSvc.getReservedOrders();
    std::string orderId;
    for (const auto& o : reserved) {
        if (o.sampleId == sampleId) { orderId = o.orderId; break; }
    }
    ASSERT_FALSE(orderId.empty());

    ASSERT_TRUE(orderSvc.approve(orderId));
    auto allOrders = orderSvc.getAllOrders();
    auto it = std::find_if(allOrders.begin(), allOrders.end(),
        [&](const Order& o){ return o.orderId == orderId; });
    ASSERT_NE(it, allOrders.end());
    EXPECT_EQ(it->status, OrderStatus::CONFIRMED) << "재고 충분 시 CONFIRMED 기대";
}

TEST_F(RegressionTest, H2_InsufficientStock_ReservedToProducing) {
    // 재고 0 시료에 수량 10 주문 → PRODUCING
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("H2-시료", 0.1, 0.9));
    auto found = svc.searchByName("H2-시료");
    ASSERT_EQ(found.size(), 1u);
    const std::string sampleId = found[0].sampleId;

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "H2-고객", 10));
    auto reserved = orderSvc.getReservedOrders();
    std::string orderId;
    for (const auto& o : reserved) {
        if (o.sampleId == sampleId) { orderId = o.orderId; break; }
    }
    ASSERT_FALSE(orderId.empty());

    ASSERT_TRUE(orderSvc.approve(orderId));
    auto allOrders = orderSvc.getAllOrders();
    auto it = std::find_if(allOrders.begin(), allOrders.end(),
        [&](const Order& o){ return o.orderId == orderId; });
    ASSERT_NE(it, allOrders.end());
    EXPECT_EQ(it->status, OrderStatus::PRODUCING) << "재고 부족 시 PRODUCING 기대";
}

TEST_F(RegressionTest, H3_Reject_ReservedToRejected) {
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("H3-시료", 0.1, 0.9));
    auto found = svc.searchByName("H3-시료");
    ASSERT_EQ(found.size(), 1u);

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(found[0].sampleId, "H3-고객", 5));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    const std::string orderId = reserved[0].orderId;

    ASSERT_TRUE(orderSvc.reject(orderId));
    // getAllOrders()는 REJECTED 제외하므로 findAll과 다름 — 직접 상태 확인
    // getAllOrders는 REJECTED를 제외하므로, REJECTED 주문이 목록에 없어야 함
    auto allOrders = orderSvc.getAllOrders();
    bool found2 = std::any_of(allOrders.begin(), allOrders.end(),
        [&](const Order& o){ return o.orderId == orderId; });
    EXPECT_FALSE(found2) << "REJECTED 주문은 getAllOrders()에서 제외되어야 함";
}

TEST_F(RegressionTest, H4_RejectedOrder_ExcludedFromReserved) {
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("H4-시료", 0.1, 0.9));
    auto found = svc.searchByName("H4-시료");
    ASSERT_EQ(found.size(), 1u);

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(found[0].sampleId, "H4-고객", 5));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    const std::string orderId = reserved[0].orderId;

    ASSERT_TRUE(orderSvc.reject(orderId));
    auto reservedAfter = orderSvc.getReservedOrders();
    bool found3 = std::any_of(reservedAfter.begin(), reservedAfter.end(),
        [&](const Order& o){ return o.orderId == orderId; });
    EXPECT_FALSE(found3) << "REJECTED 주문은 getReservedOrders()에서 제외되어야 함";
}

TEST_F(RegressionTest, H5_Release_StockDecreased) {
    // 재고 있는 시료를 직접 생성 후 CONFIRMED 주문 출고 → 재고 차감
    // JSON 파일에 재고=100인 시료를 직접 생성
    {
        nlohmann::json samples = nlohmann::json::array();
        nlohmann::json s;
        s["sampleId"]    = "S-001";
        s["name"]        = "H5-웨이퍼";
        s["avgProdTime"] = 0.1;
        s["yield"]       = 0.9;
        s["stock"]       = 100;
        samples.push_back(s);
        std::ofstream ofs(kS);
        ofs << samples.dump(2);
    }

    const std::string sampleId   = "S-001";
    const int         quantity   = 5;
    const int         stockStart = 100;

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "H5-고객", quantity));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    const std::string orderId = reserved[0].orderId;
    ASSERT_TRUE(orderSvc.approve(orderId));

    auto allOrders = orderSvc.getAllOrders();
    auto it = std::find_if(allOrders.begin(), allOrders.end(),
        [&](const Order& o){ return o.orderId == orderId; });
    ASSERT_NE(it, allOrders.end());
    ASSERT_EQ(it->status, OrderStatus::CONFIRMED);

    // approve 시 재고 차감: stockStart - quantity
    SampleService svcMid(kS);
    auto midOpt = svcMid.getSampleById(sampleId);
    ASSERT_TRUE(midOpt.has_value());
    EXPECT_EQ(midOpt->stock, stockStart - quantity)
        << "approve 후 재고 = stockStart - quantity 기대";

    ReleaseService relSvc(kO, kS);
    ASSERT_TRUE(relSvc.release(orderId));

    SampleService svcAfter(kS);
    auto sampleOpt = svcAfter.getSampleById(sampleId);
    ASSERT_TRUE(sampleOpt.has_value());
    // release 시 재고 추가 차감: (stockStart - quantity) - quantity
    EXPECT_EQ(sampleOpt->stock, stockStart - quantity - quantity)
        << "출고 후 재고 = stockStart - quantity*2 기대";
}

// ============================================================
// I. 생산 공식 정확성
// ============================================================

TEST_F(RegressionTest, I1_ProductionFormula_Shortage100_Yield1) {
    // shortage=100, yield=1.0 → actualProduction = ceil(100/(1.0*0.9)) = ceil(111.11) = 112
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("I1-시료", 0.1, 1.0));
    auto found = svc.searchByName("I1-시료");
    ASSERT_EQ(found.size(), 1u);
    const std::string sampleId = found[0].sampleId;

    // stock=0, quantity=100 → shortage=100
    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "I1-고객", 100));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    ASSERT_TRUE(orderSvc.approve(reserved[0].orderId));

    ProductionService prodSvc(kP, kS, kO);
    auto job = prodSvc.getCurrentJob();
    ASSERT_TRUE(job.has_value());
    EXPECT_EQ(job->shortage, 100);
    EXPECT_EQ(job->actualProduction, 112)
        << "ceil(100/(1.0*0.9))=112 기대, 실제=" << job->actualProduction;
}

TEST_F(RegressionTest, I2_ProductionFormula_Shortage170_Yield092) {
    // shortage=170, yield=0.92 → actualProduction = ceil(170/(0.92*0.9)) = ceil(205.36) = 206
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("I2-시료", 0.1, 0.92));
    auto found = svc.searchByName("I2-시료");
    ASSERT_EQ(found.size(), 1u);
    const std::string sampleId = found[0].sampleId;

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "I2-고객", 170));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    ASSERT_TRUE(orderSvc.approve(reserved[0].orderId));

    ProductionService prodSvc(kP, kS, kO);
    auto job = prodSvc.getCurrentJob();
    ASSERT_TRUE(job.has_value());
    EXPECT_EQ(job->shortage, 170);
    EXPECT_EQ(job->actualProduction, 206)
        << "ceil(170/(0.92*0.9))=206 기대, 실제=" << job->actualProduction;
}

TEST_F(RegressionTest, I3_ProductionFormula_Shortage1_Yield085) {
    // shortage=1, yield=0.85 → actualProduction = ceil(1/(0.85*0.9)) = ceil(1.307) = 2
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("I3-시료", 0.1, 0.85));
    auto found = svc.searchByName("I3-시료");
    ASSERT_EQ(found.size(), 1u);
    const std::string sampleId = found[0].sampleId;

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "I3-고객", 1));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    ASSERT_TRUE(orderSvc.approve(reserved[0].orderId));

    ProductionService prodSvc(kP, kS, kO);
    auto job = prodSvc.getCurrentJob();
    ASSERT_TRUE(job.has_value());
    EXPECT_EQ(job->shortage, 1);
    EXPECT_EQ(job->actualProduction, 2)
        << "ceil(1/(0.85*0.9))=2 기대, 실제=" << job->actualProduction;
}

TEST_F(RegressionTest, I4_TotalTime_Accuracy) {
    // totalTime = avgProdTime * actualProduction 검증
    // avgProdTime=0.5, shortage=10, yield=1.0 → actualProduction=ceil(10/0.9)=12, totalTime=6.0
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("I4-시료", 0.5, 1.0));
    auto found = svc.searchByName("I4-시료");
    ASSERT_EQ(found.size(), 1u);
    const std::string sampleId = found[0].sampleId;

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "I4-고객", 10));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    ASSERT_TRUE(orderSvc.approve(reserved[0].orderId));

    ProductionService prodSvc(kP, kS, kO);
    auto job = prodSvc.getCurrentJob();
    ASSERT_TRUE(job.has_value());
    // actualProduction = ceil(10/0.9) = 12, totalTime = 0.5 * 12 = 6.0
    EXPECT_EQ(job->actualProduction, 12);
    EXPECT_DOUBLE_EQ(job->totalTime, 0.5 * job->actualProduction)
        << "totalTime = avgProdTime * actualProduction 기대";
}

// ============================================================
// J. tickCheck 자동 완료
// ============================================================

TEST_F(RegressionTest, J1_PastEndTime_TickCheckTrue_StockIncreased_OrderConfirmed) {
    // 과거 estimatedEndTime → tickCheck()=true, 재고 증가, PRODUCING→CONFIRMED
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("J1-시료", 0.1, 0.9));
    auto found = svc.searchByName("J1-시료");
    ASSERT_EQ(found.size(), 1u);
    const std::string sampleId = found[0].sampleId;

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "J1-고객", 10));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    const std::string orderId = reserved[0].orderId;
    ASSERT_TRUE(orderSvc.approve(orderId));

    // 생산 Job의 estimatedEndTime을 과거로 조작
    ProductionService prodSvc(kP, kS, kO);
    auto jobOpt = prodSvc.getCurrentJob();
    ASSERT_TRUE(jobOpt.has_value());

    // JSON 파일 직접 수정으로 estimatedEndTime을 과거로 설정
    {
        std::ifstream ifs(kP);
        nlohmann::json j;
        ifs >> j;
        ifs.close();
        for (auto& item : j) {
            if (item["status"] == "RUNNING") {
                item["estimatedEndTime"] = TimeUtil::addMinutes(TimeUtil::nowString(), -10.0);
            }
        }
        std::ofstream ofs(kP);
        ofs << j.dump(2);
    }

    // 재고 조회 (tickCheck 전)
    SampleService svcBefore(kS);
    auto sampleBefore = svcBefore.getSampleById(sampleId);
    ASSERT_TRUE(sampleBefore.has_value());
    const int stockBefore = sampleBefore->stock;

    // 새 인스턴스로 tickCheck
    ProductionService prodSvc2(kP, kS, kO);
    bool ticked = prodSvc2.tickCheck();
    EXPECT_TRUE(ticked) << "과거 estimatedEndTime → tickCheck()=true 기대";

    // 재고 증가 확인
    SampleService svcAfter(kS);
    auto sampleAfter = svcAfter.getSampleById(sampleId);
    ASSERT_TRUE(sampleAfter.has_value());
    EXPECT_GT(sampleAfter->stock, stockBefore) << "tickCheck 후 재고 증가 기대";

    // 주문 상태 CONFIRMED 확인
    OrderService orderSvc2(kS, kO, kP);
    auto allOrders = orderSvc2.getAllOrders();
    auto it = std::find_if(allOrders.begin(), allOrders.end(),
        [&](const Order& o){ return o.orderId == orderId; });
    ASSERT_NE(it, allOrders.end());
    EXPECT_EQ(it->status, OrderStatus::CONFIRMED) << "tickCheck 후 PRODUCING→CONFIRMED 기대";
}

TEST_F(RegressionTest, J2_FutureEndTime_TickCheckFalse_NoChange) {
    // 미래 estimatedEndTime → tickCheck()=false, 상태 변화 없음
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("J2-시료", 0.1, 0.9));
    auto found = svc.searchByName("J2-시료");
    ASSERT_EQ(found.size(), 1u);

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(found[0].sampleId, "J2-고객", 10));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    ASSERT_TRUE(orderSvc.approve(reserved[0].orderId));

    // estimatedEndTime이 미래이므로 tickCheck는 false를 반환해야 함
    ProductionService prodSvc(kP, kS, kO);
    auto jobOpt = prodSvc.getCurrentJob();
    ASSERT_TRUE(jobOpt.has_value());
    // 기본 enqueue 시 estimatedEndTime = now + totalTime (미래)
    bool ticked = prodSvc.tickCheck();
    EXPECT_FALSE(ticked) << "미래 estimatedEndTime → tickCheck()=false 기대";
}

TEST_F(RegressionTest, J3_WaitingJob_AutoStartAfterFirstComplete) {
    // WAITING Job 2개: 첫 번째 완료 → 두 번째 자동 RUNNING 전환 확인
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("J3-시료A", 0.1, 0.9));
    ASSERT_TRUE(svc.registerSample("J3-시료B", 0.1, 0.9));
    auto foundA = svc.searchByName("J3-시료A");
    auto foundB = svc.searchByName("J3-시료B");
    ASSERT_EQ(foundA.size(), 1u);
    ASSERT_EQ(foundB.size(), 1u);

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(foundA[0].sampleId, "J3-고객A", 10));
    ASSERT_TRUE(orderSvc.reserve(foundB[0].sampleId, "J3-고객B", 10));

    auto reserved = orderSvc.getReservedOrders();
    ASSERT_GE(reserved.size(), 2u);
    ASSERT_TRUE(orderSvc.approve(reserved[0].orderId));
    ASSERT_TRUE(orderSvc.approve(reserved[1].orderId));

    // 첫 번째 Job의 estimatedEndTime을 과거로 조작
    {
        std::ifstream ifs(kP);
        nlohmann::json j;
        ifs >> j;
        ifs.close();
        bool first = true;
        for (auto& item : j) {
            if (item["status"] == "RUNNING" && first) {
                item["estimatedEndTime"] = TimeUtil::addMinutes(TimeUtil::nowString(), -10.0);
                first = false;
            }
        }
        std::ofstream ofs(kP);
        ofs << j.dump(2);
    }

    // tickCheck → 첫 번째 완료
    ProductionService prodSvc(kP, kS, kO);
    bool ticked = prodSvc.tickCheck();
    EXPECT_TRUE(ticked) << "첫 번째 Job 완료 기대";

    // 두 번째 Job이 RUNNING으로 전환되었는지 확인
    ProductionService prodSvc2(kP, kS, kO);
    auto nextJob = prodSvc2.getCurrentJob();
    EXPECT_TRUE(nextJob.has_value()) << "두 번째 Job이 RUNNING으로 전환되어야 함";
    if (nextJob.has_value()) {
        EXPECT_EQ(nextJob->status, JobStatus::RUNNING)
            << "두 번째 Job 상태가 RUNNING이어야 함";
    }
}

// ============================================================
// K. 모니터링 경계값
// ============================================================

TEST_F(RegressionTest, K1_StockZero_Exhausted) {
    // stock=0 → 고갈
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("K1-시료", 0.1, 0.9));

    MonitoringService monSvc(kS, kO);
    auto stockList = monSvc.getStockStatusList();
    ASSERT_FALSE(stockList.empty());
    auto it = std::find_if(stockList.begin(), stockList.end(),
        [](const StockStatus& ss){ return ss.name == "K1-시료"; });
    ASSERT_NE(it, stockList.end());
    EXPECT_EQ(it->status, "고갈") << "stock=0 → 고갈 기대";
}

TEST_F(RegressionTest, K2_StockLessThanActiveSum_Shortage) {
    // stock < activeSum → 부족
    // DummyGen으로 시료 생성 후, 재고보다 많은 수량 주문 승인 → PRODUCING 상태
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("K2-시료", 0.1, 0.9));
    auto found = svc.searchByName("K2-시료");
    ASSERT_EQ(found.size(), 1u);
    const std::string sampleId = found[0].sampleId;

    // stock=0이지만 PRODUCING 주문이 있으면 activeSum>0 → 부족 (0 < activeSum)
    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "K2-고객", 10));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    ASSERT_TRUE(orderSvc.approve(reserved[0].orderId));

    // 생산 Job 생성 후 stock을 수동으로 5로 설정 (0 < 10)
    // JSON 파일 직접 수정
    {
        std::ifstream ifs(kS);
        nlohmann::json j;
        ifs >> j;
        ifs.close();
        for (auto& item : j) {
            if (item["sampleId"] == sampleId) {
                item["stock"] = 5;
            }
        }
        std::ofstream ofs(kS);
        ofs << j.dump(2);
    }

    MonitoringService monSvc(kS, kO);
    auto stockList = monSvc.getStockStatusList();
    auto it = std::find_if(stockList.begin(), stockList.end(),
        [&](const StockStatus& ss){ return ss.sampleId == sampleId; });
    ASSERT_NE(it, stockList.end());
    EXPECT_EQ(it->status, "부족")
        << "stock=5 < activeSum=10 → 부족 기대, 실제=" << it->status;
}

TEST_F(RegressionTest, K3_StockEqualsActiveSum_Sufficient) {
    // stock == activeSum → 여유 (경계값)
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("K3-시료", 0.1, 0.9));
    auto found = svc.searchByName("K3-시료");
    ASSERT_EQ(found.size(), 1u);
    const std::string sampleId = found[0].sampleId;

    // PRODUCING 주문 생성 후 stock을 activeSum과 동일하게 설정
    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "K3-고객", 10));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    ASSERT_TRUE(orderSvc.approve(reserved[0].orderId));

    // stock = 10 (activeSum과 동일)
    {
        std::ifstream ifs(kS);
        nlohmann::json j;
        ifs >> j;
        ifs.close();
        for (auto& item : j) {
            if (item["sampleId"] == sampleId) {
                item["stock"] = 10;
            }
        }
        std::ofstream ofs(kS);
        ofs << j.dump(2);
    }

    MonitoringService monSvc(kS, kO);
    auto stockList = monSvc.getStockStatusList();
    auto it = std::find_if(stockList.begin(), stockList.end(),
        [&](const StockStatus& ss){ return ss.sampleId == sampleId; });
    ASSERT_NE(it, stockList.end());
    EXPECT_EQ(it->status, "여유")
        << "stock=10 == activeSum=10 → 여유 기대, 실제=" << it->status;
}

TEST_F(RegressionTest, K4_StockGreaterThanActiveSum_Sufficient_ReservedExcluded) {
    // stock > activeSum → 여유 (RESERVED는 activeSum에 미포함 확인)
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("K4-시료", 0.1, 0.9));
    auto found = svc.searchByName("K4-시료");
    ASSERT_EQ(found.size(), 1u);
    const std::string sampleId = found[0].sampleId;

    // stock=50 설정
    {
        std::ifstream ifs(kS);
        nlohmann::json j;
        ifs >> j;
        ifs.close();
        for (auto& item : j) {
            if (item["sampleId"] == sampleId) {
                item["stock"] = 50;
            }
        }
        std::ofstream ofs(kS);
        ofs << j.dump(2);
    }

    // RESERVED 주문 100개 → activeSum에 포함되지 않아야 함
    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "K4-고객-RESERVED", 100));

    MonitoringService monSvc(kS, kO);
    auto stockList = monSvc.getStockStatusList();
    auto it = std::find_if(stockList.begin(), stockList.end(),
        [&](const StockStatus& ss){ return ss.sampleId == sampleId; });
    ASSERT_NE(it, stockList.end());
    // RESERVED는 activeSum에 포함되지 않으므로 activeSum=0, stock=50 → 여유
    EXPECT_EQ(it->status, "여유")
        << "RESERVED는 activeSum 제외 → stock=50, activeSum=0 → 여유 기대";
    EXPECT_EQ(it->activeSum, 0) << "RESERVED 주문은 activeSum에 포함되지 않아야 함";
}

// ============================================================
// L. 출고 오류 케이스
// ============================================================

TEST_F(RegressionTest, L1_ReservedOrder_ReleaseFails) {
    // RESERVED 상태 주문 출고 시도 → false 반환
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("L1-시료", 0.1, 0.9));
    auto found = svc.searchByName("L1-시료");
    ASSERT_EQ(found.size(), 1u);

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(found[0].sampleId, "L1-고객", 5));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    const std::string orderId = reserved[0].orderId;

    ReleaseService relSvc(kO, kS);
    bool result = relSvc.release(orderId);
    EXPECT_FALSE(result) << "RESERVED 상태 주문 출고는 실패해야 함";
}

TEST_F(RegressionTest, L2_RejectedOrder_ReleaseFails) {
    // REJECTED 상태 주문 출고 시도 → false 반환
    SampleService svc(kS);
    ASSERT_TRUE(svc.registerSample("L2-시료", 0.1, 0.9));
    auto found = svc.searchByName("L2-시료");
    ASSERT_EQ(found.size(), 1u);

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(found[0].sampleId, "L2-고객", 5));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    const std::string orderId = reserved[0].orderId;
    ASSERT_TRUE(orderSvc.reject(orderId));

    ReleaseService relSvc(kO, kS);
    bool result = relSvc.release(orderId);
    EXPECT_FALSE(result) << "REJECTED 상태 주문 출고는 실패해야 함";
}

TEST_F(RegressionTest, L3_Release_StockDecremented) {
    // 출고 후 재고가 stock -= quantity 만큼 차감됨
    // JSON 파일에 재고=200인 시료를 직접 생성
    {
        nlohmann::json samples = nlohmann::json::array();
        nlohmann::json s;
        s["sampleId"]    = "S-001";
        s["name"]        = "L3-웨이퍼";
        s["avgProdTime"] = 0.1;
        s["yield"]       = 0.9;
        s["stock"]       = 200;
        samples.push_back(s);
        std::ofstream ofs(kS);
        ofs << samples.dump(2);
    }

    const std::string sampleId   = "S-001";
    const int         quantity   = 5;
    const int         stockStart = 200;

    OrderService orderSvc(kS, kO, kP);
    ASSERT_TRUE(orderSvc.reserve(sampleId, "L3-고객", quantity));
    auto reserved = orderSvc.getReservedOrders();
    ASSERT_FALSE(reserved.empty());
    const std::string orderId = reserved[0].orderId;
    ASSERT_TRUE(orderSvc.approve(orderId));

    auto allOrders = orderSvc.getAllOrders();
    auto it = std::find_if(allOrders.begin(), allOrders.end(),
        [&](const Order& o){ return o.orderId == orderId; });
    ASSERT_NE(it, allOrders.end());
    ASSERT_EQ(it->status, OrderStatus::CONFIRMED);

    // approve 후 재고 확인
    SampleService svcMid(kS);
    auto sampleMid = svcMid.getSampleById(sampleId);
    ASSERT_TRUE(sampleMid.has_value());
    const int stockAfterApprove = sampleMid->stock;
    ASSERT_EQ(stockAfterApprove, stockStart - quantity);

    ReleaseService relSvc(kO, kS);
    ASSERT_TRUE(relSvc.release(orderId));

    SampleService svcAfter(kS);
    auto sampleAfter = svcAfter.getSampleById(sampleId);
    ASSERT_TRUE(sampleAfter.has_value());
    EXPECT_EQ(sampleAfter->stock, stockAfterApprove - quantity)
        << "출고 후 재고 -= quantity 기대";
}

// ============================================================
// M. ID 연속성
// ============================================================

TEST_F(RegressionTest, M1_SyncIdGenerator_NextSampleIdIsS011) {
    // DummyGen(10종) 후 initSampleSeq(10) → 다음 시료 ID가 S-011
    DummyGen gen(kS, kO, kP);
    ASSERT_TRUE(gen.generate());

    // DummyGen 후 시료가 10개임을 확인
    SampleService svc(kS);
    auto allSamples = svc.getAllSamples();
    ASSERT_EQ(allSamples.size(), 10u) << "더미 시료 10종 기대";

    // IdGenerator 동기화
    IdGenerator::reset();
    IdGenerator::initSampleSeq(static_cast<int>(allSamples.size()));

    // 다음 시료 ID가 S-011이어야 함
    std::string nextId = IdGenerator::nextSampleId();
    EXPECT_EQ(nextId, "S-011") << "DummyGen 10종 후 다음 ID는 S-011 기대";
}

TEST_F(RegressionTest, M2_NextSampleId_SequentialOrder) {
    // nextSampleId() 연속 호출 시 S-011, S-012 순서 보장
    DummyGen gen(kS, kO, kP);
    ASSERT_TRUE(gen.generate());

    SampleService svc(kS);
    auto allSamples = svc.getAllSamples();

    IdGenerator::reset();
    IdGenerator::initSampleSeq(static_cast<int>(allSamples.size()));

    std::string id1 = IdGenerator::nextSampleId();
    std::string id2 = IdGenerator::nextSampleId();
    EXPECT_EQ(id1, "S-011") << "첫 번째 연속 호출: S-011 기대";
    EXPECT_EQ(id2, "S-012") << "두 번째 연속 호출: S-012 기대";
}

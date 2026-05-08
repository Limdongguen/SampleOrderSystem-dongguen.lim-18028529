#include "gmock/gmock.h"
#include "service/SampleService.h"
#include "service/OrderService.h"
#include "service/ProductionService.h"
#include "service/ReleaseService.h"
#include "service/MonitoringService.h"
#include "model/MonitoringTypes.h"
#include "util/IdGenerator.h"
#include "DummyGen.h"
#include <filesystem>
#include <algorithm>

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

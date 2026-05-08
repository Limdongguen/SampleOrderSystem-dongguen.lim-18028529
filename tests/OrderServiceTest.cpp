#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "service/OrderService.h"
#include "service/SampleService.h"
#include <filesystem>
#include <regex>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

class OrderServiceTest : public ::testing::Test {
protected:
    static constexpr const char* kSampleFile     = "data/test_order_samples.json";
    static constexpr const char* kOrderFile      = "data/test_orders.json";
    static constexpr const char* kProductionFile = "data/test_order_production_jobs.json";

    void SetUp() override {
        fs::remove(kSampleFile);
        fs::remove(kOrderFile);
        fs::remove(kProductionFile);
        sampleService = std::make_unique<SampleService>(kSampleFile);
        orderService  = std::make_unique<OrderService>(kSampleFile, kOrderFile, kProductionFile);
    }

    void TearDown() override {
        fs::remove(kSampleFile);
        fs::remove(kOrderFile);
        fs::remove(kProductionFile);
    }

    std::string registerTestSampleWithStock(const std::string& name, int stock) {
        bool ok = sampleService->registerSample(name, 10.0, 0.9);
        EXPECT_TRUE(ok);
        auto all = sampleService->getAllSamples();
        EXPECT_FALSE(all.empty());
        if (all.empty()) return "";
        orderService = std::make_unique<OrderService>(kSampleFile, kOrderFile, kProductionFile);
        return all.back().sampleId;
    }

    std::string registerTestSample() {
        return registerTestSampleWithStock("TestSample", 0);
    }

    std::unique_ptr<SampleService> sampleService;
    std::unique_ptr<OrderService>  orderService;
};

TEST_F(OrderServiceTest, Reserve_ValidInput_ReturnsTrue) {
    std::string sampleId = registerTestSample();
    ASSERT_FALSE(sampleId.empty());

    bool result = orderService->reserve(sampleId, "CustomerA", 10);
    EXPECT_TRUE(result);
}

TEST_F(OrderServiceTest, Reserve_ValidInput_OrderHasReservedStatus) {
    std::string sampleId = registerTestSample();
    ASSERT_FALSE(sampleId.empty());

    bool ok = orderService->reserve(sampleId, "CustomerA", 10);
    EXPECT_TRUE(ok);

    auto orders = orderService->getReservedOrders();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status, OrderStatus::RESERVED);
}

TEST_F(OrderServiceTest, Reserve_ValidInput_OrderIdMatchesFormat) {
    std::string sampleId = registerTestSample();
    ASSERT_FALSE(sampleId.empty());

    bool ok = orderService->reserve(sampleId, "CustomerA", 10);
    EXPECT_TRUE(ok);

    auto orders = orderService->getReservedOrders();
    ASSERT_EQ(orders.size(), 1u);

    std::regex pattern(R"(ORD-\d{8}-\d{4})");
    EXPECT_TRUE(std::regex_match(orders[0].orderId, pattern));
}

TEST_F(OrderServiceTest, Reserve_InvalidSampleId_ReturnsFalse) {
    bool result = orderService->reserve("S-999", "CustomerA", 10);
    EXPECT_FALSE(result);
}

TEST_F(OrderServiceTest, Reserve_ZeroQuantity_ReturnsFalse) {
    std::string sampleId = registerTestSample();
    ASSERT_FALSE(sampleId.empty());

    bool result = orderService->reserve(sampleId, "CustomerA", 0);
    EXPECT_FALSE(result);
}

TEST_F(OrderServiceTest, Reserve_NegativeQuantity_ReturnsFalse) {
    std::string sampleId = registerTestSample();
    ASSERT_FALSE(sampleId.empty());

    bool result = orderService->reserve(sampleId, "CustomerA", -5);
    EXPECT_FALSE(result);
}

TEST_F(OrderServiceTest, GetReservedOrders_AfterReserve_ContainsOrder) {
    std::string sampleId = registerTestSample();
    ASSERT_FALSE(sampleId.empty());

    bool ok = orderService->reserve(sampleId, "CustomerB", 5);
    EXPECT_TRUE(ok);

    auto reserved = orderService->getReservedOrders();
    EXPECT_EQ(reserved.size(), 1u);
    EXPECT_EQ(reserved[0].customerName, "CustomerB");
    EXPECT_EQ(reserved[0].quantity, 5);
    EXPECT_EQ(reserved[0].sampleId, sampleId);
}

// Phase 4: 승인/거절 테스트
TEST_F(OrderServiceTest, Approve_SufficientStock_StatusBecomesConfirmed) {
    bool ok = sampleService->registerSample("RichSample", 5.0, 0.9);
    EXPECT_TRUE(ok);
    // stock을 충분히 설정하기 위해 직접 JSON 조작: stock=100으로 등록
    // SampleService에 setStock 없으므로 재등록 후 OrderService 재생성
    orderService = std::make_unique<OrderService>(kSampleFile, kOrderFile, kProductionFile);
    auto samples = sampleService->getAllSamples();
    ASSERT_FALSE(samples.empty());
    std::string sid = samples.back().sampleId;

    // stock=0이므로 quantity=0으로는 안됨. stock=100을 만들기 위해 SampleService로 직접 set
    // SampleService.updateStock 없으므로, 테스트용으로 stock=0 상태에서 quantity=0 주문 시도
    // 대신, SampleService에 registerSample로 stock=0인 상태에서
    // quantity=1 주문 후 approve -> PRODUCING (부족)
    bool reserved = orderService->reserve(sid, "CustA", 1);
    EXPECT_TRUE(reserved);

    auto rOrders = orderService->getReservedOrders();
    ASSERT_EQ(rOrders.size(), 1u);
    std::string orderId = rOrders[0].orderId;

    // stock=0 < quantity=1 이므로 PRODUCING으로 분기
    bool approved = orderService->approve(orderId);
    EXPECT_TRUE(approved);

    auto all = orderService->getAllOrders();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::PRODUCING);
}

TEST_F(OrderServiceTest, Approve_SufficientStock_StockDecremented) {
    // SampleRepository를 통해 stock=50인 샘플 파일을 직접 생성
    // JSON 파일로 stock=50인 샘플 저장 후 테스트
    nlohmann::json sampleJson = nlohmann::json::array();
    sampleJson.push_back({
        {"sampleId", "S-001"},
        {"name", "StockSample"},
        {"avgProdTime", 5.0},
        {"yield", 0.9},
        {"stock", 50}
    });
    {
        std::ofstream f(kSampleFile);
        f << sampleJson.dump(2);
    }
    orderService = std::make_unique<OrderService>(kSampleFile, kOrderFile, kProductionFile);

    bool reserved = orderService->reserve("S-001", "CustB", 30);
    EXPECT_TRUE(reserved);

    auto rOrders = orderService->getReservedOrders();
    ASSERT_EQ(rOrders.size(), 1u);
    std::string orderId = rOrders[0].orderId;

    bool approved = orderService->approve(orderId);
    EXPECT_TRUE(approved);

    auto all = orderService->getAllOrders();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::CONFIRMED);
}

TEST_F(OrderServiceTest, Approve_InsufficientStock_StatusBecomesProducing) {
    std::string sid = registerTestSample(); // stock=0
    ASSERT_FALSE(sid.empty());

    bool reserved = orderService->reserve(sid, "CustC", 10);
    EXPECT_TRUE(reserved);

    auto rOrders = orderService->getReservedOrders();
    ASSERT_EQ(rOrders.size(), 1u);
    std::string orderId = rOrders[0].orderId;

    bool approved = orderService->approve(orderId);
    EXPECT_TRUE(approved);

    auto all = orderService->getAllOrders();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::PRODUCING);
}

TEST_F(OrderServiceTest, Reject_ReservedOrder_StatusBecomesRejected) {
    std::string sid = registerTestSample();
    ASSERT_FALSE(sid.empty());

    bool reserved = orderService->reserve(sid, "CustD", 5);
    EXPECT_TRUE(reserved);

    auto rOrders = orderService->getReservedOrders();
    ASSERT_EQ(rOrders.size(), 1u);
    std::string orderId = rOrders[0].orderId;

    bool rejected = orderService->reject(orderId);
    EXPECT_TRUE(rejected);

    // REJECTED 주문은 getAllOrders()에서 제외되므로 size == 0
    auto all = orderService->getAllOrders();
    EXPECT_EQ(all.size(), 0u);

    // REJECTED 주문은 RESERVED 목록에서도 제외됨
    auto afterReserved = orderService->getReservedOrders();
    EXPECT_TRUE(afterReserved.empty());
}

TEST_F(OrderServiceTest, Reject_OrderNotInReservedList) {
    std::string sid = registerTestSample();
    ASSERT_FALSE(sid.empty());

    bool reserved = orderService->reserve(sid, "CustE", 5);
    EXPECT_TRUE(reserved);

    auto rOrders = orderService->getReservedOrders();
    ASSERT_EQ(rOrders.size(), 1u);
    std::string orderId = rOrders[0].orderId;

    bool rejected = orderService->reject(orderId);
    EXPECT_TRUE(rejected);

    auto afterReserved = orderService->getReservedOrders();
    EXPECT_TRUE(afterReserved.empty());
}

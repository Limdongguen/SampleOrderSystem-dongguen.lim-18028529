#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "service/OrderService.h"
#include "service/SampleService.h"
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

class OrderServiceTest : public ::testing::Test {
protected:
    static constexpr const char* kSampleFile = "data/test_order_samples.json";
    static constexpr const char* kOrderFile  = "data/test_orders.json";

    void SetUp() override {
        fs::remove(kSampleFile);
        fs::remove(kOrderFile);
        sampleService = std::make_unique<SampleService>(kSampleFile);
        // orderService는 sampleService로 샘플을 등록한 후 재생성하여 최신 파일을 읽도록 함
        orderService  = std::make_unique<OrderService>(kSampleFile, kOrderFile);
    }

    void TearDown() override {
        fs::remove(kSampleFile);
        fs::remove(kOrderFile);
    }

    std::string registerTestSample() {
        bool ok = sampleService->registerSample("TestSample", 10.0, 0.9);
        EXPECT_TRUE(ok);
        auto all = sampleService->getAllSamples();
        EXPECT_FALSE(all.empty());
        if (all.empty()) return "";
        // SampleService가 파일에 저장한 후, OrderService를 재생성하여 최신 샘플 목록을 로드
        orderService = std::make_unique<OrderService>(kSampleFile, kOrderFile);
        return all[0].sampleId;
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

    // ORD-YYYYMMDD-NNNN
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

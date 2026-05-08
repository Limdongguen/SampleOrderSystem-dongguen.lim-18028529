#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "service/ReleaseService.h"
#include "service/SampleService.h"
#include "service/OrderService.h"
#include <filesystem>

namespace fs = std::filesystem;

class ReleaseServiceTest : public ::testing::Test {
protected:
    static constexpr const char* kSampleFile = "data/test_release_samples.json";
    static constexpr const char* kOrderFile  = "data/test_release_orders.json";

    void SetUp() override {
        fs::remove(kSampleFile);
        fs::remove(kOrderFile);

        sampleService  = std::make_unique<SampleService>(kSampleFile);
        releaseService = std::make_unique<ReleaseService>(kOrderFile, kSampleFile);
    }

    void TearDown() override {
        fs::remove(kSampleFile);
        fs::remove(kOrderFile);
    }

    std::unique_ptr<SampleService>  sampleService;
    std::unique_ptr<ReleaseService> releaseService;
};

// CONFIRMED 주문 출고 -> RELEASED, stock -= quantity
TEST_F(ReleaseServiceTest, Release_ConfirmedOrder_TransitionsToReleased) {
    bool ok = sampleService->registerSample("RelSample", 5.0, 0.9);
    EXPECT_TRUE(ok);
    auto samples = sampleService->getAllSamples();
    ASSERT_FALSE(samples.empty());
    const std::string sampleId = samples[0].sampleId;

    // CONFIRMED 상태 주문을 직접 OrderRepository에 삽입
    OrderRepository orderRepo(kOrderFile);
    Order order;
    order.orderId      = "ORD-20260508-0010";
    order.sampleId     = sampleId;
    order.customerName = "RelCustomer";
    order.quantity     = 10;
    order.status       = OrderStatus::CONFIRMED;
    order.createdAt    = "2026-05-08 10:00:00";
    order.updatedAt    = "2026-05-08 10:00:00";
    orderRepo.add(order);
    orderRepo.save();

    // SampleRepository에도 재고 업데이트
    SampleRepository sampleRepo(kSampleFile);
    auto allSamples = sampleRepo.findAll();
    ASSERT_FALSE(allSamples.empty());
    Sample s = allSamples[0];
    s.stock = 50;
    sampleRepo.update(s);
    sampleRepo.save();

    // ReleaseService 재생성 (파일 기반)
    auto rs = std::make_unique<ReleaseService>(kOrderFile, kSampleFile);
    auto confirmed = rs->getConfirmedOrders();
    ASSERT_EQ(confirmed.size(), 1u);

    bool released = rs->release("ORD-20260508-0010");
    EXPECT_TRUE(released);

    // 상태 확인
    SampleRepository sampleRepo2(kSampleFile);
    auto updatedSample = sampleRepo2.findById(sampleId);
    ASSERT_TRUE(updatedSample.has_value());
    EXPECT_EQ(updatedSample->stock, 40); // 50 - 10

    OrderRepository orderRepo2(kOrderFile);
    auto updatedOrder = orderRepo2.findById("ORD-20260508-0010");
    ASSERT_TRUE(updatedOrder.has_value());
    EXPECT_EQ(updatedOrder->status, OrderStatus::RELEASED);
}

// CONFIRMED 아닌 주문 출고 시도 -> false 반환
TEST_F(ReleaseServiceTest, Release_NonConfirmedOrder_ReturnsFalse) {
    OrderRepository orderRepo(kOrderFile);
    Order order;
    order.orderId      = "ORD-20260508-0011";
    order.sampleId     = "S-001";
    order.customerName = "Cust";
    order.quantity     = 5;
    order.status       = OrderStatus::RESERVED;
    order.createdAt    = "2026-05-08 10:00:00";
    order.updatedAt    = "2026-05-08 10:00:00";
    orderRepo.add(order);
    orderRepo.save();

    auto rs = std::make_unique<ReleaseService>(kOrderFile, kSampleFile);
    bool released = rs->release("ORD-20260508-0011");
    EXPECT_FALSE(released);
}

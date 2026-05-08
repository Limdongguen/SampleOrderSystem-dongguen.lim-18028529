// MonitoringServiceTest.cpp
#include <gtest/gtest.h>
#include "service/MonitoringService.h"
#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"
#include "model/Sample.h"
#include "model/Order.h"
#include "model/Enums.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {
    const std::string kTestSamples = "data/test_monitoring_samples.json";
    const std::string kTestOrders  = "data/test_monitoring_orders.json";

    void removeIfExists(const std::string& path) {
        if (fs::exists(path)) fs::remove(path);
    }

    Sample makeSample(const std::string& id, const std::string& name, int stock) {
        Sample s;
        s.sampleId   = id;
        s.name       = name;
        s.avgProdTime = 10.0;
        s.yield      = 0.9;
        s.stock      = stock;
        return s;
    }

    Order makeOrder(const std::string& id, const std::string& sampleId,
                    int qty, OrderStatus status)
    {
        Order o;
        o.orderId      = id;
        o.sampleId     = sampleId;
        o.customerName = "TestCustomer";
        o.quantity     = qty;
        o.status       = status;
        o.createdAt    = "2026-05-08 00:00:00";
        o.updatedAt    = "2026-05-08 00:00:00";
        return o;
    }
}

class MonitoringServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        removeIfExists(kTestSamples);
        removeIfExists(kTestOrders);
    }
    void TearDown() override {
        removeIfExists(kTestSamples);
        removeIfExists(kTestOrders);
    }
};

// --- OrderSummary ---

TEST_F(MonitoringServiceTest, OrderSummary_ExcludesRejected) {
    SampleRepository sampleRepo(kTestSamples);
    sampleRepo.add(makeSample("S-001", "Alpha", 100));
    sampleRepo.save();

    OrderRepository orderRepo(kTestOrders);
    orderRepo.add(makeOrder("ORD-001", "S-001", 10, OrderStatus::RESERVED));
    orderRepo.add(makeOrder("ORD-002", "S-001", 20, OrderStatus::CONFIRMED));
    orderRepo.add(makeOrder("ORD-003", "S-001", 30, OrderStatus::PRODUCING));
    orderRepo.add(makeOrder("ORD-004", "S-001", 40, OrderStatus::RELEASED));
    orderRepo.add(makeOrder("ORD-005", "S-001", 50, OrderStatus::REJECTED)); // 제외
    orderRepo.save();

    MonitoringService svc(kTestSamples, kTestOrders);
    auto summary = svc.getOrderSummary();

    EXPECT_EQ(summary.reserved,  1);
    EXPECT_EQ(summary.confirmed, 1);
    EXPECT_EQ(summary.producing, 1);
    EXPECT_EQ(summary.released,  1);
    EXPECT_EQ(summary.total,     4); // REJECTED 제외
}

TEST_F(MonitoringServiceTest, OrderSummary_AllZeroWhenEmpty) {
    MonitoringService svc(kTestSamples, kTestOrders);
    auto summary = svc.getOrderSummary();

    EXPECT_EQ(summary.reserved,  0);
    EXPECT_EQ(summary.confirmed, 0);
    EXPECT_EQ(summary.producing, 0);
    EXPECT_EQ(summary.released,  0);
    EXPECT_EQ(summary.total,     0);
}

// --- StockStatus ---

TEST_F(MonitoringServiceTest, StockStatus_SufficientWhenStockGeActiveSum) {
    // stock == activeSum → 여유
    SampleRepository sampleRepo(kTestSamples);
    sampleRepo.add(makeSample("S-001", "Alpha", 80));
    sampleRepo.save();

    OrderRepository orderRepo(kTestOrders);
    orderRepo.add(makeOrder("ORD-001", "S-001", 50, OrderStatus::CONFIRMED));
    orderRepo.add(makeOrder("ORD-002", "S-001", 30, OrderStatus::PRODUCING));
    orderRepo.save();

    MonitoringService svc(kTestSamples, kTestOrders);
    auto list = svc.getStockStatusList();

    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].status, "여유");
    EXPECT_EQ(list[0].activeSum, 80);
    EXPECT_EQ(list[0].stock, 80);
}

TEST_F(MonitoringServiceTest, StockStatus_ShortageWhenStockLtActiveSum) {
    // 0 < stock < activeSum → 부족
    SampleRepository sampleRepo(kTestSamples);
    sampleRepo.add(makeSample("S-001", "Alpha", 79));
    sampleRepo.save();

    OrderRepository orderRepo(kTestOrders);
    orderRepo.add(makeOrder("ORD-001", "S-001", 50, OrderStatus::CONFIRMED));
    orderRepo.add(makeOrder("ORD-002", "S-001", 30, OrderStatus::PRODUCING));
    orderRepo.save();

    MonitoringService svc(kTestSamples, kTestOrders);
    auto list = svc.getStockStatusList();

    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].status, "부족");
}

TEST_F(MonitoringServiceTest, StockStatus_ExhaustedWhenStockIsZero) {
    // stock == 0 → 고갈
    SampleRepository sampleRepo(kTestSamples);
    sampleRepo.add(makeSample("S-001", "Alpha", 0));
    sampleRepo.save();

    OrderRepository orderRepo(kTestOrders);
    orderRepo.add(makeOrder("ORD-001", "S-001", 30, OrderStatus::CONFIRMED));
    orderRepo.save();

    MonitoringService svc(kTestSamples, kTestOrders);
    auto list = svc.getStockStatusList();

    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].status, "고갈");
}

TEST_F(MonitoringServiceTest, StockStatus_SufficientWhenNoActiveOrders) {
    // activeSum == 0, stock > 0 → 여유
    SampleRepository sampleRepo(kTestSamples);
    sampleRepo.add(makeSample("S-001", "Alpha", 50));
    sampleRepo.save();

    MonitoringService svc(kTestSamples, kTestOrders);
    auto list = svc.getStockStatusList();

    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].status, "여유");
    EXPECT_DOUBLE_EQ(list[0].remainRatio, 100.0);
}

TEST_F(MonitoringServiceTest, StockStatus_RemainRatioCalculation) {
    // stock=100, activeSum=80 → ratio = 100/80*100 = 125.0
    SampleRepository sampleRepo(kTestSamples);
    sampleRepo.add(makeSample("S-001", "Alpha", 100));
    sampleRepo.save();

    OrderRepository orderRepo(kTestOrders);
    orderRepo.add(makeOrder("ORD-001", "S-001", 80, OrderStatus::CONFIRMED));
    orderRepo.save();

    MonitoringService svc(kTestSamples, kTestOrders);
    auto list = svc.getStockStatusList();

    ASSERT_EQ(list.size(), 1u);
    EXPECT_DOUBLE_EQ(list[0].remainRatio, 125.0);
}

TEST_F(MonitoringServiceTest, StockStatus_ReservedNotCountedInActiveSum) {
    // RESERVED는 activeSum에 미포함 → 여유
    SampleRepository sampleRepo(kTestSamples);
    sampleRepo.add(makeSample("S-001", "Alpha", 10));
    sampleRepo.save();

    OrderRepository orderRepo(kTestOrders);
    orderRepo.add(makeOrder("ORD-001", "S-001", 100, OrderStatus::RESERVED)); // 미포함
    orderRepo.save();

    MonitoringService svc(kTestSamples, kTestOrders);
    auto list = svc.getStockStatusList();

    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].activeSum, 0);
    EXPECT_EQ(list[0].status, "여유");
}

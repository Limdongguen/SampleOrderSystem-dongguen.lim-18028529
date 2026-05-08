#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "service/ProductionService.h"
#include "service/SampleService.h"
#include "service/OrderService.h"
#include <filesystem>
#include <cmath>

namespace fs = std::filesystem;

class ProductionServiceTest : public ::testing::Test {
protected:
    static constexpr const char* kSampleFile     = "data/test_prod_samples.json";
    static constexpr const char* kOrderFile      = "data/test_prod_orders.json";
    static constexpr const char* kProductionFile = "data/test_production_jobs.json";

    void SetUp() override {
        fs::remove(kSampleFile);
        fs::remove(kOrderFile);
        fs::remove(kProductionFile);

        sampleService     = std::make_unique<SampleService>(kSampleFile);
        productionService = std::make_unique<ProductionService>(kProductionFile);
    }

    void TearDown() override {
        fs::remove(kSampleFile);
        fs::remove(kOrderFile);
        fs::remove(kProductionFile);
    }

    std::unique_ptr<SampleService>     sampleService;
    std::unique_ptr<ProductionService> productionService;
};

// shortage=170, yield=0.92 -> actualProduction = ceil(170/(0.92*0.9)) = 206
TEST_F(ProductionServiceTest, Enqueue_CalculatesActualProduction_Correctly) {
    bool ok = sampleService->registerSample("TestSample", 5.0, 0.92);
    EXPECT_TRUE(ok);

    auto samples = sampleService->getAllSamples();
    ASSERT_FALSE(samples.empty());
    Sample sample = samples[0];

    Order order;
    order.orderId      = "ORD-20260508-0001";
    order.sampleId     = sample.sampleId;
    order.customerName = "CustomerA";
    order.quantity     = 200;
    order.status       = OrderStatus::RESERVED;
    order.createdAt    = "2026-05-08 10:00:00";
    order.updatedAt    = "2026-05-08 10:00:00";

    // stock = 30, shortage = 200 - 30 = 170
    sample.stock = 30;

    bool enqueued = productionService->enqueue(order, sample);
    EXPECT_TRUE(enqueued);

    auto queue = productionService->getQueue();
    ASSERT_EQ(queue.size(), 1u);

    int expectedActual = (int)std::ceil(170.0 / (0.92 * 0.9));
    EXPECT_EQ(queue[0].shortage, 170);
    EXPECT_EQ(queue[0].actualProduction, expectedActual); // 206
}

TEST_F(ProductionServiceTest, Enqueue_ZeroShortage_ReturnsFalse) {
    bool ok = sampleService->registerSample("TestSample2", 5.0, 0.9);
    EXPECT_TRUE(ok);

    auto samples = sampleService->getAllSamples();
    ASSERT_FALSE(samples.empty());
    Sample sample = samples[0];
    sample.stock = 100;

    Order order;
    order.orderId      = "ORD-20260508-0002";
    order.sampleId     = sample.sampleId;
    order.customerName = "CustomerB";
    order.quantity     = 50; // stock >= quantity -> shortage <= 0
    order.status       = OrderStatus::RESERVED;
    order.createdAt    = "2026-05-08 10:00:00";
    order.updatedAt    = "2026-05-08 10:00:00";

    bool enqueued = productionService->enqueue(order, sample);
    EXPECT_FALSE(enqueued);
}

TEST_F(ProductionServiceTest, Enqueue_TotalTimeCalculated_Correctly) {
    bool ok = sampleService->registerSample("TimeSample", 10.0, 0.9);
    EXPECT_TRUE(ok);

    auto samples = sampleService->getAllSamples();
    ASSERT_FALSE(samples.empty());
    Sample sample = samples[0];
    sample.stock = 0;

    Order order;
    order.orderId      = "ORD-20260508-0003";
    order.sampleId     = sample.sampleId;
    order.customerName = "CustomerC";
    order.quantity     = 100;
    order.status       = OrderStatus::RESERVED;
    order.createdAt    = "2026-05-08 10:00:00";
    order.updatedAt    = "2026-05-08 10:00:00";

    bool enqueued = productionService->enqueue(order, sample);
    EXPECT_TRUE(enqueued);

    auto queue = productionService->getQueue();
    ASSERT_EQ(queue.size(), 1u);

    int expectedActual = (int)std::ceil(100.0 / (0.9 * 0.9));
    double expectedTime = 10.0 * expectedActual;
    EXPECT_EQ(queue[0].actualProduction, expectedActual);
    EXPECT_DOUBLE_EQ(queue[0].totalTime, expectedTime);
}

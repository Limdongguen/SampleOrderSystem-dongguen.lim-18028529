#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "service/ProductionService.h"
#include "service/SampleService.h"
#include "service/OrderService.h"
#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"
#include "repository/ProductionJobRepository.h"
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
        productionService = std::make_unique<ProductionService>(kProductionFile, kSampleFile, kOrderFile);
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

// tickCheck: 과거 estimatedEndTime -> 재고 증가 + CONFIRMED 전환
TEST_F(ProductionServiceTest, TickCheck_PastEstimatedEndTime_CompletesJob) {
    // 샘플 등록 후 SampleRepository에 재고 설정
    bool ok = sampleService->registerSample("TickSample", 5.0, 0.9);
    EXPECT_TRUE(ok);
    auto samples = sampleService->getAllSamples();
    ASSERT_FALSE(samples.empty());
    const std::string sampleId = samples[0].sampleId;

    SampleRepository sampleRepo(kSampleFile);
    auto allSamples = sampleRepo.findAll();
    ASSERT_FALSE(allSamples.empty());
    Sample s = allSamples[0];
    s.stock = 0;
    sampleRepo.update(s);
    sampleRepo.save();

    // 주문 등록
    OrderRepository orderRepo(kOrderFile);
    Order order;
    order.orderId      = "ORD-20260508-0020";
    order.sampleId     = sampleId;
    order.customerName = "TickCustomer";
    order.quantity     = 50;
    order.status       = OrderStatus::PRODUCING;
    order.createdAt    = "2026-05-08 10:00:00";
    order.updatedAt    = "2026-05-08 10:00:00";
    orderRepo.add(order);
    orderRepo.save();

    // ProductionJob을 직접 RUNNING 상태로 삽입 (과거 estimatedEndTime)
    const int actualProduction = (int)std::ceil(50.0 / (0.9 * 0.9));
    ProductionJobRepository jobRepo(kProductionFile);
    ProductionJob job;
    job.jobId            = "JOB-ORD-20260508-0020";
    job.orderId          = "ORD-20260508-0020";
    job.sampleId         = sampleId;
    job.shortage         = 50;
    job.actualProduction = actualProduction;
    job.totalTime        = 5.0 * actualProduction;
    job.status           = JobStatus::RUNNING;
    job.estimatedEndTime = "2020-01-01 00:00:00"; // 과거 시각
    jobRepo.add(job);
    jobRepo.save();

    // ProductionService 재생성 (파일 기반 로드)
    auto ps = std::make_unique<ProductionService>(kProductionFile, kSampleFile, kOrderFile);
    bool tickResult = ps->tickCheck();
    EXPECT_TRUE(tickResult);

    // 재고 증가 확인
    SampleRepository sampleRepo2(kSampleFile);
    auto updatedSample = sampleRepo2.findById(sampleId);
    ASSERT_TRUE(updatedSample.has_value());
    EXPECT_EQ(updatedSample->stock, actualProduction);

    // 주문 상태 CONFIRMED 전환 확인
    OrderRepository orderRepo2(kOrderFile);
    auto updatedOrder = orderRepo2.findById("ORD-20260508-0020");
    ASSERT_TRUE(updatedOrder.has_value());
    EXPECT_EQ(updatedOrder->status, OrderStatus::CONFIRMED);
}

// tickCheck: Job 2개 - 첫 번째 완료 후 두 번째 자동 RUNNING 전환
TEST_F(ProductionServiceTest, TickCheck_TwoJobs_SecondBecomesRunningAfterFirst) {
    // 샘플 등록
    bool ok = sampleService->registerSample("QueueSample", 5.0, 0.9);
    EXPECT_TRUE(ok);
    auto samples = sampleService->getAllSamples();
    ASSERT_FALSE(samples.empty());
    const std::string sampleId = samples[0].sampleId;

    SampleRepository sampleRepo(kSampleFile);
    auto allSamples = sampleRepo.findAll();
    Sample s = allSamples[0];
    s.stock = 0;
    sampleRepo.update(s);
    sampleRepo.save();

    // 주문 2개 등록
    OrderRepository orderRepo(kOrderFile);
    Order order1;
    order1.orderId = "ORD-20260508-0030";
    order1.sampleId = sampleId;
    order1.customerName = "QueueCust1";
    order1.quantity = 10;
    order1.status = OrderStatus::PRODUCING;
    order1.createdAt = "2026-05-08 10:00:00";
    order1.updatedAt = "2026-05-08 10:00:00";
    orderRepo.add(order1);

    Order order2;
    order2.orderId = "ORD-20260508-0031";
    order2.sampleId = sampleId;
    order2.customerName = "QueueCust2";
    order2.quantity = 20;
    order2.status = OrderStatus::PRODUCING;
    order2.createdAt = "2026-05-08 10:00:00";
    order2.updatedAt = "2026-05-08 10:00:00";
    orderRepo.add(order2);
    orderRepo.save();

    // Job 2개 등록: RUNNING(과거) + WAITING
    const int actual1 = (int)std::ceil(10.0 / (0.9 * 0.9));
    const int actual2 = (int)std::ceil(20.0 / (0.9 * 0.9));
    ProductionJobRepository jobRepo(kProductionFile);
    ProductionJob job1;
    job1.jobId = "JOB-ORD-20260508-0030";
    job1.orderId = "ORD-20260508-0030";
    job1.sampleId = sampleId;
    job1.shortage = 10;
    job1.actualProduction = actual1;
    job1.totalTime = 5.0 * actual1;
    job1.status = JobStatus::RUNNING;
    job1.estimatedEndTime = "2020-01-01 00:00:00"; // 과거
    jobRepo.add(job1);

    ProductionJob job2;
    job2.jobId = "JOB-ORD-20260508-0031";
    job2.orderId = "ORD-20260508-0031";
    job2.sampleId = sampleId;
    job2.shortage = 20;
    job2.actualProduction = actual2;
    job2.totalTime = 5.0 * actual2;
    job2.status = JobStatus::WAITING;
    job2.estimatedEndTime = "";
    jobRepo.add(job2);
    jobRepo.save();

    // ProductionService 재생성 후 tickCheck
    auto ps = std::make_unique<ProductionService>(kProductionFile, kSampleFile, kOrderFile);
    bool tickResult = ps->tickCheck();
    EXPECT_TRUE(tickResult);

    // 두 번째 Job이 RUNNING으로 전환되었는지 확인
    auto currentJob = ps->getCurrentJob();
    ASSERT_TRUE(currentJob.has_value());
    EXPECT_EQ(currentJob->jobId, "JOB-ORD-20260508-0031");
    EXPECT_EQ(currentJob->status, JobStatus::RUNNING);
    EXPECT_FALSE(currentJob->estimatedEndTime.empty());
}

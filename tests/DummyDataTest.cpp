#include "gmock/gmock.h"
#include "util/JsonFileManager.h"
#include "util/TimeUtil.h"
#include "model/Sample.h"
#include "model/Order.h"
#include "model/ProductionJob.h"
#include "model/Enums.h"
#include <string>
#include <vector>
#include <filesystem>

// DummyGen 헤더 포함 (프로젝트 디렉토리에 위치)
#include "DummyGen.h"

using ::testing::Eq;
using ::testing::SizeIs;
using ::testing::Ne;

namespace fs = std::filesystem;

// 테스트용 임시 경로
static const std::string kTestSamplesPath     = "data/test_dummy_samples.json";
static const std::string kTestOrdersPath      = "data/test_dummy_orders.json";
static const std::string kTestProductionPath  = "data/test_dummy_production.json";

class DummyDataTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 이전 테스트 파일 정리
        if (fs::exists(kTestSamplesPath))    fs::remove(kTestSamplesPath);
        if (fs::exists(kTestOrdersPath))     fs::remove(kTestOrdersPath);
        if (fs::exists(kTestProductionPath)) fs::remove(kTestProductionPath);
    }
    void TearDown() override {
        if (fs::exists(kTestSamplesPath))    fs::remove(kTestSamplesPath);
        if (fs::exists(kTestOrdersPath))     fs::remove(kTestOrdersPath);
        if (fs::exists(kTestProductionPath)) fs::remove(kTestProductionPath);
    }
};

// 시료 10종이 생성되어야 한다
TEST_F(DummyDataTest, GeneratesSampleCount) {
    DummyGen gen(kTestSamplesPath, kTestOrdersPath, kTestProductionPath);
    bool r = gen.generate();
    EXPECT_TRUE(r);

    auto jSamples = JsonFileManager::load(kTestSamplesPath);
    auto samples  = jSamples.get<std::vector<Sample>>();
    EXPECT_THAT(samples, SizeIs(10));
}

// 시료 avgProdTime이 0.05 ~ 0.2 범위여야 한다
TEST_F(DummyDataTest, SampleAvgProdTimeInRange) {
    DummyGen gen(kTestSamplesPath, kTestOrdersPath, kTestProductionPath);
    bool r = gen.generate();
    EXPECT_TRUE(r);

    auto jSamples = JsonFileManager::load(kTestSamplesPath);
    auto samples  = jSamples.get<std::vector<Sample>>();
    for (const auto& s : samples) {
        EXPECT_GE(s.avgProdTime, 0.05);
        EXPECT_LE(s.avgProdTime, 0.20);
        EXPECT_GE(s.yield,       0.85);
        EXPECT_LE(s.yield,       0.98);
    }
}

// 주문 20건이 생성되어야 한다
TEST_F(DummyDataTest, GeneratesOrderCount) {
    DummyGen gen(kTestSamplesPath, kTestOrdersPath, kTestProductionPath);
    bool r = gen.generate();
    EXPECT_TRUE(r);

    auto jOrders = JsonFileManager::load(kTestOrdersPath);
    auto orders  = jOrders.get<std::vector<Order>>();
    EXPECT_THAT(orders, SizeIs(20));
}

// 주문 상태 분포: RESERVED 3, CONFIRMED 5, PRODUCING 3, RELEASED 8, REJECTED 1
TEST_F(DummyDataTest, OrderStatusDistribution) {
    DummyGen gen(kTestSamplesPath, kTestOrdersPath, kTestProductionPath);
    bool r = gen.generate();
    EXPECT_TRUE(r);

    auto jOrders = JsonFileManager::load(kTestOrdersPath);
    auto orders  = jOrders.get<std::vector<Order>>();

    int reserved=0, confirmed=0, producing=0, released=0, rejected=0;
    for (const auto& o : orders) {
        switch (o.status) {
            case OrderStatus::RESERVED:  ++reserved;  break;
            case OrderStatus::CONFIRMED: ++confirmed; break;
            case OrderStatus::PRODUCING: ++producing; break;
            case OrderStatus::RELEASED:  ++released;  break;
            case OrderStatus::REJECTED:  ++rejected;  break;
        }
    }
    EXPECT_THAT(reserved,  Eq(3));
    EXPECT_THAT(confirmed, Eq(5));
    EXPECT_THAT(producing, Eq(3));
    EXPECT_THAT(released,  Eq(8));
    EXPECT_THAT(rejected,  Eq(1));
}

// 생산 작업 3건: RUNNING 1 + WAITING 2
TEST_F(DummyDataTest, ProductionJobDistribution) {
    DummyGen gen(kTestSamplesPath, kTestOrdersPath, kTestProductionPath);
    bool r = gen.generate();
    EXPECT_TRUE(r);

    auto jJobs = JsonFileManager::load(kTestProductionPath);
    auto jobs  = jJobs.get<std::vector<ProductionJob>>();
    EXPECT_THAT(jobs, SizeIs(3));

    int running=0, waiting=0;
    for (const auto& j : jobs) {
        if (j.status == JobStatus::RUNNING) ++running;
        if (j.status == JobStatus::WAITING) ++waiting;
    }
    EXPECT_THAT(running, Eq(1));
    EXPECT_THAT(waiting, Eq(2));
}

// RUNNING Job의 estimatedEndTime이 현재 시각 +2분 이후여야 한다
TEST_F(DummyDataTest, RunningJobEstimatedEndTimeIsInFuture) {
    DummyGen gen(kTestSamplesPath, kTestOrdersPath, kTestProductionPath);
    bool r = gen.generate();
    EXPECT_TRUE(r);

    auto jJobs = JsonFileManager::load(kTestProductionPath);
    auto jobs  = jJobs.get<std::vector<ProductionJob>>();

    for (const auto& j : jobs) {
        if (j.status == JobStatus::RUNNING) {
            // estimatedEndTime은 현재보다 미래
            bool past = TimeUtil::isPast(j.estimatedEndTime);
            EXPECT_FALSE(past);
        }
    }
}

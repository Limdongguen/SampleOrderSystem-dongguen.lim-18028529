#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "service/SampleService.h"
#include <filesystem>

namespace fs = std::filesystem;

class SampleServiceTest : public ::testing::Test {
protected:
    static constexpr const char* kTestFile = "data/test_service_samples.json";

    void SetUp() override {
        fs::remove(kTestFile);
        service = std::make_unique<SampleService>(kTestFile);
    }

    void TearDown() override {
        fs::remove(kTestFile);
    }

    std::unique_ptr<SampleService> service;
};

TEST_F(SampleServiceTest, RegisterSample_ValidInput_ReturnsTrue) {
    bool result = service->registerSample("ValidSample", 10.0, 0.9);
    EXPECT_TRUE(result);
}

TEST_F(SampleServiceTest, RegisterSample_DuplicateName_ReturnsFalse) {
    bool r1 = service->registerSample("Dup", 10.0, 0.9);
    EXPECT_TRUE(r1);

    bool r2 = service->registerSample("Dup", 5.0, 0.8);
    EXPECT_FALSE(r2);
}

TEST_F(SampleServiceTest, RegisterSample_YieldZero_ReturnsFalse) {
    bool result = service->registerSample("ZeroYield", 10.0, 0.0);
    EXPECT_FALSE(result);
}

TEST_F(SampleServiceTest, RegisterSample_YieldNegative_ReturnsFalse) {
    bool result = service->registerSample("NegYield", 10.0, -0.1);
    EXPECT_FALSE(result);
}

TEST_F(SampleServiceTest, RegisterSample_YieldAboveOne_ReturnsFalse) {
    bool result = service->registerSample("HighYield", 10.0, 1.1);
    EXPECT_FALSE(result);
}

TEST_F(SampleServiceTest, RegisterSample_AvgProdTimeZero_ReturnsFalse) {
    bool result = service->registerSample("ZeroTime", 0.0, 0.9);
    EXPECT_FALSE(result);
}

TEST_F(SampleServiceTest, RegisterSample_AvgProdTimeNegative_ReturnsFalse) {
    bool result = service->registerSample("NegTime", -1.0, 0.9);
    EXPECT_FALSE(result);
}

TEST_F(SampleServiceTest, RegisterSample_EmptyName_ReturnsFalse) {
    bool result = service->registerSample("", 10.0, 0.9);
    EXPECT_FALSE(result);
}

TEST_F(SampleServiceTest, GetAllSamples_AfterRegister_ReturnsAll) {
    bool r1 = service->registerSample("A", 10.0, 0.9);
    bool r2 = service->registerSample("B", 5.0, 0.8);
    EXPECT_TRUE(r1);
    EXPECT_TRUE(r2);

    auto all = service->getAllSamples();
    EXPECT_EQ(all.size(), 2u);
}

TEST_F(SampleServiceTest, SearchByName_PartialMatch_ReturnsMatches) {
    bool r1 = service->registerSample("AlphaSample", 10.0, 0.9);
    bool r2 = service->registerSample("BetaSample", 5.0, 0.8);
    bool r3 = service->registerSample("Gamma", 7.0, 0.85);
    EXPECT_TRUE(r1);
    EXPECT_TRUE(r2);
    EXPECT_TRUE(r3);

    auto results = service->searchByName("Sample");
    EXPECT_EQ(results.size(), 2u);
}

TEST_F(SampleServiceTest, GetSampleById_Existing_ReturnsSample) {
    bool r = service->registerSample("FindMe", 10.0, 0.9);
    EXPECT_TRUE(r);

    auto all = service->getAllSamples();
    ASSERT_FALSE(all.empty());
    const std::string id = all[0].sampleId;

    auto result = service->getSampleById(id);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "FindMe");
}

TEST_F(SampleServiceTest, GetSampleById_NonExisting_ReturnsNullopt) {
    auto result = service->getSampleById("S-999");
    EXPECT_FALSE(result.has_value());
}

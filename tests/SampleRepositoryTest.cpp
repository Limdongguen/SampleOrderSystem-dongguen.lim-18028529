#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "repository/SampleRepository.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class SampleRepositoryTest : public ::testing::Test {
protected:
    static constexpr const char* kTestFile = "data/test_samples.json";

    void SetUp() override {
        fs::remove(kTestFile);
        repo = std::make_unique<SampleRepository>(kTestFile);
    }

    void TearDown() override {
        fs::remove(kTestFile);
    }

    std::unique_ptr<SampleRepository> repo;
};

TEST_F(SampleRepositoryTest, AddThenFindAll_ContainsSample) {
    Sample s;
    s.sampleId   = "S-001";
    s.name       = "TestSample";
    s.avgProdTime = 10.0;
    s.yield      = 0.9;
    s.stock      = 100;

    repo->add(s);
    auto all = repo->findAll();

    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].sampleId, "S-001");
    EXPECT_EQ(all[0].name, "TestSample");
}

TEST_F(SampleRepositoryTest, FindById_ExistingId_ReturnsSample) {
    Sample s;
    s.sampleId   = "S-001";
    s.name       = "Alpha";
    s.avgProdTime = 5.0;
    s.yield      = 0.8;
    s.stock      = 50;

    repo->add(s);
    auto result = repo->findById("S-001");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "Alpha");
}

TEST_F(SampleRepositoryTest, FindById_NonExistingId_ReturnsNullopt) {
    auto result = repo->findById("S-999");
    EXPECT_FALSE(result.has_value());
}

TEST_F(SampleRepositoryTest, FindByName_PartialMatch_ReturnsMatches) {
    Sample s1;
    s1.sampleId = "S-001"; s1.name = "AlphaSample";
    s1.avgProdTime = 5.0; s1.yield = 0.8; s1.stock = 10;

    Sample s2;
    s2.sampleId = "S-002"; s2.name = "BetaSample";
    s2.avgProdTime = 6.0; s2.yield = 0.9; s2.stock = 20;

    Sample s3;
    s3.sampleId = "S-003"; s3.name = "Gamma";
    s3.avgProdTime = 7.0; s3.yield = 0.85; s3.stock = 30;

    repo->add(s1);
    repo->add(s2);
    repo->add(s3);

    auto results = repo->findByName("Sample");

    EXPECT_EQ(results.size(), 2u);
}

TEST_F(SampleRepositoryTest, SaveAndReload_PersistsSamples) {
    Sample s;
    s.sampleId   = "S-001";
    s.name       = "Persistent";
    s.avgProdTime = 8.0;
    s.yield      = 0.95;
    s.stock      = 200;

    repo->add(s);
    repo->save();

    auto repo2 = std::make_unique<SampleRepository>(kTestFile);
    auto all   = repo2->findAll();

    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].name, "Persistent");
}

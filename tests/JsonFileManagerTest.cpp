#include <gtest/gtest.h>
#include "util/JsonFileManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

static const std::string kTestDir  = "data/test_temp/";
static const std::string kTestFile = kTestDir + "test_data.json";

class JsonFileManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        fs::create_directories(kTestDir);
    }

    void TearDown() override {
        fs::remove_all(kTestDir);
    }
};

// 저장 후 로드 시 동일한 데이터 반환
TEST_F(JsonFileManagerTest, SaveAndLoadRoundTrip) {
    nlohmann::json data = nlohmann::json::array();
    data.push_back({ {"id", "S-001"}, {"name", "TestSample"} });

    JsonFileManager::save(kTestFile, data);
    nlohmann::json loaded = JsonFileManager::load(kTestFile);

    EXPECT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded[0]["id"].get<std::string>(), "S-001");
    EXPECT_EQ(loaded[0]["name"].get<std::string>(), "TestSample");
}

// 파일이 없을 때 빈 배열 반환
TEST_F(JsonFileManagerTest, LoadNonExistentFileReturnsEmptyArray) {
    nlohmann::json result = JsonFileManager::load(kTestDir + "nonexistent.json");
    EXPECT_TRUE(result.is_array());
    EXPECT_EQ(result.size(), 0u);
}

// 여러 항목 저장/로드
TEST_F(JsonFileManagerTest, SaveAndLoadMultipleItems) {
    nlohmann::json data = nlohmann::json::array();
    data.push_back({ {"orderId", "ORD-20260101-0001"} });
    data.push_back({ {"orderId", "ORD-20260101-0002"} });

    JsonFileManager::save(kTestFile, data);
    nlohmann::json loaded = JsonFileManager::load(kTestFile);

    EXPECT_EQ(loaded.size(), 2u);
}

// 파싱 오류 시 빈 배열 반환
TEST_F(JsonFileManagerTest, LoadInvalidJsonReturnsEmptyArray) {
    std::ofstream ofs(kTestFile);
    ofs << "{ invalid json !!!";
    ofs.close();

    nlohmann::json result = JsonFileManager::load(kTestFile);
    EXPECT_TRUE(result.is_array());
    EXPECT_EQ(result.size(), 0u);
}

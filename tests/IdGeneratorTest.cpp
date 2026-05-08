#include <gtest/gtest.h>
#include "util/IdGenerator.h"
#include <regex>
#include <string>

class IdGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        IdGenerator::reset();
    }
};

// 주문 ID 형식: ORD-YYYYMMDD-NNNN
TEST_F(IdGeneratorTest, OrderIdFormatIsCorrect) {
    std::string id = IdGenerator::nextOrderId();
    std::regex pattern(R"(ORD-\d{8}-\d{4})");
    EXPECT_TRUE(std::regex_match(id, pattern)) << "ID: " << id;
}

// 시료 ID 형식: S-NNN
TEST_F(IdGeneratorTest, SampleIdFormatIsCorrect) {
    std::string id = IdGenerator::nextSampleId();
    std::regex pattern(R"(S-\d{3})");
    EXPECT_TRUE(std::regex_match(id, pattern)) << "ID: " << id;
}

// 주문 순번 증가
TEST_F(IdGeneratorTest, OrderIdSequenceIncreases) {
    std::string id1 = IdGenerator::nextOrderId();
    std::string id2 = IdGenerator::nextOrderId();
    std::string id3 = IdGenerator::nextOrderId();

    // 마지막 4자리 시퀀스 번호 추출
    std::string seq1 = id1.substr(id1.size() - 4);
    std::string seq2 = id2.substr(id2.size() - 4);
    std::string seq3 = id3.substr(id3.size() - 4);

    EXPECT_EQ(seq1, "0001");
    EXPECT_EQ(seq2, "0002");
    EXPECT_EQ(seq3, "0003");
}

// 시료 순번 증가
TEST_F(IdGeneratorTest, SampleIdSequenceIncreases) {
    std::string id1 = IdGenerator::nextSampleId();
    std::string id2 = IdGenerator::nextSampleId();
    std::string id3 = IdGenerator::nextSampleId();

    EXPECT_EQ(id1, "S-001");
    EXPECT_EQ(id2, "S-002");
    EXPECT_EQ(id3, "S-003");
}

// reset 후 순번 초기화
TEST_F(IdGeneratorTest, ResetResetsSequence) {
    std::string dummy1 = IdGenerator::nextOrderId();
    std::string dummy2 = IdGenerator::nextOrderId();
    (void)dummy1; (void)dummy2;
    IdGenerator::reset();

    std::string id = IdGenerator::nextOrderId();
    std::string seq = id.substr(id.size() - 4);
    EXPECT_EQ(seq, "0001");
}

// 날짜 포함 (오늘 날짜가 ID에 포함되어 있는지)
TEST_F(IdGeneratorTest, OrderIdContainsDate) {
    std::string id = IdGenerator::nextOrderId();
    // ORD- 다음 8자리가 날짜
    std::string datePart = id.substr(4, 8);
    EXPECT_EQ(datePart.size(), 8u);
    // 모두 숫자인지 확인
    for (char c : datePart) {
        EXPECT_TRUE(std::isdigit(c)) << "Non-digit in date: " << c;
    }
}

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "util/TimeUtil.h"
#include <string>

// addMinutes: "2026-05-08 14:00:00" + 90.0 -> "2026-05-08 15:30:00"
TEST(TimeUtilTest, AddMinutes_90_IncreasesHourAndMinute) {
    std::string base   = "2026-05-08 14:00:00";
    std::string result = TimeUtil::addMinutes(base, 90.0);
    EXPECT_EQ(result, "2026-05-08 15:30:00");
}

// addMinutes: 0분 덧셈 -> 동일 문자열
TEST(TimeUtilTest, AddMinutes_Zero_ReturnsSameTime) {
    std::string base   = "2026-05-08 10:00:00";
    std::string result = TimeUtil::addMinutes(base, 0.0);
    EXPECT_EQ(result, "2026-05-08 10:00:00");
}

// addMinutes: 날짜 경계 넘기
TEST(TimeUtilTest, AddMinutes_CrossMidnight) {
    std::string base   = "2026-05-08 23:50:00";
    std::string result = TimeUtil::addMinutes(base, 20.0);
    EXPECT_EQ(result, "2026-05-09 00:10:00");
}

// isPast(estimatedEndTime, now): past_time -> true
TEST(TimeUtilTest, IsPast_PastTime_ReturnsTrue) {
    std::string past = "2026-01-01 00:00:00";
    std::string now  = "2026-05-08 10:00:00";
    EXPECT_TRUE(TimeUtil::isPast(past, now));
}

// isPast(estimatedEndTime, now): future_time -> false
TEST(TimeUtilTest, IsPast_FutureTime_ReturnsFalse) {
    std::string future = "2099-12-31 23:59:59";
    std::string now    = "2026-05-08 10:00:00";
    EXPECT_FALSE(TimeUtil::isPast(future, now));
}

// isPast(estimatedEndTime, now): same time -> true (same == past)
TEST(TimeUtilTest, IsPast_SameTime_ReturnsTrue) {
    std::string t   = "2026-05-08 10:00:00";
    EXPECT_TRUE(TimeUtil::isPast(t, t));
}

// secondsUntil: future time returns positive
TEST(TimeUtilTest, SecondsUntil_FutureTime_ReturnsPositive) {
    std::string future = TimeUtil::addMinutes(TimeUtil::nowString(), 10.0);
    int secs = TimeUtil::secondsUntil(future);
    EXPECT_GT(secs, 0);
    EXPECT_LE(secs, 600); // within 10 minutes
}

// secondsUntil: past time returns 0
TEST(TimeUtilTest, SecondsUntil_PastTime_ReturnsZero) {
    std::string past = "2020-01-01 00:00:00";
    int secs = TimeUtil::secondsUntil(past);
    EXPECT_EQ(secs, 0);
}

#include "util/TimeUtil.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace {
    constexpr int kSecondsPerMinute = 60;
    constexpr int kTimeStrLen       = 19; // "YYYY-MM-DD HH:MM:SS"
}

std::time_t TimeUtil::parseTime(const std::string& timeStr) {
    std::tm tm = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    tm.tm_isdst = -1;
    return std::mktime(&tm);
}

std::string TimeUtil::formatTime(std::time_t t) {
    std::tm tm = {};
    localtime_s(&tm, &t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string TimeUtil::nowString() {
    std::time_t now = std::time(nullptr);
    return formatTime(now);
}

std::string TimeUtil::addMinutes(const std::string& base, double minutes) {
    std::time_t t = parseTime(base);
    t += static_cast<std::time_t>(minutes * kSecondsPerMinute);
    return formatTime(t);
}

bool TimeUtil::isPast(const std::string& estimatedEndTime) {
    std::time_t end = parseTime(estimatedEndTime);
    std::time_t now = std::time(nullptr);
    return now >= end;
}

bool TimeUtil::isPast(const std::string& estimatedEndTime, const std::string& now) {
    std::time_t end     = parseTime(estimatedEndTime);
    std::time_t nowTime = parseTime(now);
    return nowTime >= end;
}

int TimeUtil::secondsUntil(const std::string& estimatedEndTime) {
    std::time_t end  = parseTime(estimatedEndTime);
    std::time_t now  = std::time(nullptr);
    double diff = std::difftime(end, now);
    return diff > 0.0 ? static_cast<int>(diff) : 0;
}

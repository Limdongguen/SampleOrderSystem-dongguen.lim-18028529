#pragma once
#include <string>
#include <ctime>

class TimeUtil {
public:
    static std::string nowString();
    static std::string addMinutes(const std::string& base, double minutes);
    static bool        isPast(const std::string& estimatedEndTime);
    static bool        isPast(const std::string& estimatedEndTime, const std::string& now);
    static int         secondsUntil(const std::string& estimatedEndTime);

private:
    static std::time_t parseTime(const std::string& timeStr);
    static std::string formatTime(std::time_t t);
};

#include "util/IdGenerator.h"
#include <ctime>
#include <sstream>
#include <iomanip>

namespace {
    constexpr int kTmYearBase = 1900;
    constexpr int kTmMonBase  = 1;
}

int IdGenerator::s_orderSeq  = 0;
int IdGenerator::s_sampleSeq = 0;

std::string IdGenerator::nextOrderId() {
    ++s_orderSeq;
    return "ORD-" + currentDateString() + "-" + zeroPad(s_orderSeq, kOrderSeqWidth);
}

std::string IdGenerator::nextSampleId() {
    ++s_sampleSeq;
    return "S-" + zeroPad(s_sampleSeq, kSampleSeqWidth);
}

void IdGenerator::reset() {
    s_orderSeq  = 0;
    s_sampleSeq = 0;
}

std::string IdGenerator::currentDateString() {
    std::time_t now = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << (tm.tm_year + kTmYearBase)
        << std::setw(2) << (tm.tm_mon  + kTmMonBase)
        << std::setw(2) << tm.tm_mday;
    return oss.str();
}

std::string IdGenerator::zeroPad(int value, int width) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(width) << value;
    return oss.str();
}

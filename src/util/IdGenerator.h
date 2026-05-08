#pragma once
#include <string>

class IdGenerator {
public:
    static std::string nextOrderId();
    static std::string nextSampleId();
    static void        reset();

private:
    static constexpr int kOrderSeqWidth  = 4;
    static constexpr int kSampleSeqWidth = 3;

    static int s_orderSeq;
    static int s_sampleSeq;

    static std::string currentDateString();
    static std::string zeroPad(int value, int width);
};

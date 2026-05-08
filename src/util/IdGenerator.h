#pragma once
#include <string>

class IdGenerator {
public:
    static std::string nextOrderId();
    static std::string nextSampleId();
    static void        reset();

private:
    static int s_orderSeq;
    static int s_sampleSeq;

    static std::string currentDateString();
    static std::string zeroPad(int value, int width);
};

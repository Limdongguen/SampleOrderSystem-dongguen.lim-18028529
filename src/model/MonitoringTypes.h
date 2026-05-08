#pragma once
#include <string>

struct OrderSummary {
    int reserved;
    int confirmed;
    int producing;
    int released;
    int total;
};

struct StockStatus {
    std::string sampleId;
    std::string name;
    int         stock;
    int         activeSum;
    std::string status;      // "여유" | "부족" | "고갈"
    double      remainRatio;
};

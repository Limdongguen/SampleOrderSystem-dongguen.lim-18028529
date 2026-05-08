#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct Sample {
    std::string sampleId;    // S-NNN (3자리 zero-padding)
    std::string name;
    double      avgProdTime; // min/ea
    double      yield;       // 0.0 ~ 1.0
    int         stock;       // ea
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Sample, sampleId, name, avgProdTime, yield, stock)

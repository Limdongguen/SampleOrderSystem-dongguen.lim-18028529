#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "model/Enums.h"

struct ProductionJob {
    std::string jobId;
    std::string orderId;
    std::string sampleId;
    int         shortage;          // 주문 수량 - 재고
    int         actualProduction;  // ceil(shortage / (yield * 0.9))
    double      totalTime;         // avgProdTime * actualProduction
    JobStatus   status;            // WAITING | RUNNING | DONE
    std::string estimatedEndTime;
};

inline void to_json(nlohmann::json& j, const ProductionJob& p) {
    j = nlohmann::json{
        {"jobId",             p.jobId},
        {"orderId",           p.orderId},
        {"sampleId",          p.sampleId},
        {"shortage",          p.shortage},
        {"actualProduction",  p.actualProduction},
        {"totalTime",         p.totalTime},
        {"status",            jobStatusToString(p.status)},
        {"estimatedEndTime",  p.estimatedEndTime}
    };
}

inline void from_json(const nlohmann::json& j, ProductionJob& p) {
    j.at("jobId").get_to(p.jobId);
    j.at("orderId").get_to(p.orderId);
    j.at("sampleId").get_to(p.sampleId);
    j.at("shortage").get_to(p.shortage);
    j.at("actualProduction").get_to(p.actualProduction);
    j.at("totalTime").get_to(p.totalTime);
    p.status = jobStatusFromString(j.at("status").get<std::string>());
    j.at("estimatedEndTime").get_to(p.estimatedEndTime);
}

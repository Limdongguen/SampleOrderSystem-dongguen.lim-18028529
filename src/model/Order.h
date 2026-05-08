#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "model/Enums.h"

struct Order {
    std::string orderId;      // ORD-YYYYMMDD-NNNN
    std::string sampleId;
    std::string customerName;
    int         quantity;
    OrderStatus status;
    std::string createdAt;
    std::string updatedAt;
};

inline void to_json(nlohmann::json& j, const Order& o) {
    j = nlohmann::json{
        {"orderId",      o.orderId},
        {"sampleId",     o.sampleId},
        {"customerName", o.customerName},
        {"quantity",     o.quantity},
        {"status",       orderStatusToString(o.status)},
        {"createdAt",    o.createdAt},
        {"updatedAt",    o.updatedAt}
    };
}

inline void from_json(const nlohmann::json& j, Order& o) {
    j.at("orderId").get_to(o.orderId);
    j.at("sampleId").get_to(o.sampleId);
    j.at("customerName").get_to(o.customerName);
    j.at("quantity").get_to(o.quantity);
    o.status = orderStatusFromString(j.at("status").get<std::string>());
    j.at("createdAt").get_to(o.createdAt);
    j.at("updatedAt").get_to(o.updatedAt);
}

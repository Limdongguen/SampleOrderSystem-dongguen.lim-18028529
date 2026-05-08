#pragma once
#include <string>
#include <stdexcept>

enum class OrderStatus {
    RESERVED,
    REJECTED,
    PRODUCING,
    CONFIRMED,
    RELEASED
};

enum class JobStatus {
    WAITING,
    RUNNING,
    DONE
};

inline std::string orderStatusToString(OrderStatus s) {
    switch (s) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::REJECTED:  return "REJECTED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::RELEASED:  return "RELEASED";
    default:                     return "UNKNOWN";
    }
}

inline OrderStatus orderStatusFromString(const std::string& s) {
    if (s == "RESERVED")  return OrderStatus::RESERVED;
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASED")  return OrderStatus::RELEASED;
    throw std::invalid_argument("Unknown OrderStatus: " + s);
}

inline std::string jobStatusToString(JobStatus s) {
    switch (s) {
    case JobStatus::WAITING: return "WAITING";
    case JobStatus::RUNNING: return "RUNNING";
    case JobStatus::DONE:    return "DONE";
    default:                 return "UNKNOWN";
    }
}

inline JobStatus jobStatusFromString(const std::string& s) {
    if (s == "WAITING") return JobStatus::WAITING;
    if (s == "RUNNING") return JobStatus::RUNNING;
    if (s == "DONE")    return JobStatus::DONE;
    throw std::invalid_argument("Unknown JobStatus: " + s);
}

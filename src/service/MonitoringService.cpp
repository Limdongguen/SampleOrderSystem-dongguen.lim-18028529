#include "service/MonitoringService.h"
#include "model/Enums.h"
#include <algorithm>

MonitoringService::MonitoringService(const std::string& sampleFilePath,
                                     const std::string& orderFilePath)
    : m_sampleRepo(std::make_unique<SampleRepository>(sampleFilePath))
    , m_orderRepo (std::make_unique<OrderRepository>(orderFilePath))
{
}

OrderSummary MonitoringService::getOrderSummary() const {
    OrderSummary summary{};
    for (const auto& order : m_orderRepo->findAll()) {
        switch (order.status) {
        case OrderStatus::RESERVED:  ++summary.reserved;  ++summary.total; break;
        case OrderStatus::CONFIRMED: ++summary.confirmed; ++summary.total; break;
        case OrderStatus::PRODUCING: ++summary.producing; ++summary.total; break;
        case OrderStatus::RELEASED:  ++summary.released;  ++summary.total; break;
        case OrderStatus::REJECTED:
        default:
            break;
        }
    }
    return summary;
}

std::vector<StockStatus> MonitoringService::getStockStatusList() const {
    std::vector<StockStatus> result;
    for (const auto& sample : m_sampleRepo->findAll()) {
        int activeSum = calcActiveSum(sample.sampleId);
        StockStatus ss;
        ss.sampleId   = sample.sampleId;
        ss.name       = sample.name;
        ss.stock      = sample.stock;
        ss.activeSum  = activeSum;
        ss.status     = determineStatus(sample.stock, activeSum);
        ss.remainRatio = calcRemainRatio(sample.stock, activeSum);
        result.push_back(ss);
    }
    return result;
}

int MonitoringService::calcActiveSum(const std::string& sampleId) const {
    int sum = 0;
    for (const auto& order : m_orderRepo->findAll()) {
        if (order.sampleId == sampleId &&
            (order.status == OrderStatus::CONFIRMED ||
             order.status == OrderStatus::PRODUCING))
        {
            sum += order.quantity;
        }
    }
    return sum;
}

std::string MonitoringService::determineStatus(int stock, int activeSum) {
    if (stock == 0) return "고갈";
    if (stock < activeSum) return "부족";
    return "여유";
}

double MonitoringService::calcRemainRatio(int stock, int activeSum) {
    if (activeSum == 0) return 100.0;
    return static_cast<double>(stock) / static_cast<double>(activeSum) * 100.0;
}

#include "service/MonitoringService.h"

MonitoringService::MonitoringService(const std::string& sampleFilePath,
                                     const std::string& orderFilePath)
    : m_sampleRepo(std::make_unique<SampleRepository>(sampleFilePath))
    , m_orderRepo (std::make_unique<OrderRepository>(orderFilePath))
{
}

OrderSummary MonitoringService::getOrderSummary() const {
    // TODO: implement
    return {};
}

std::vector<StockStatus> MonitoringService::getStockStatusList() const {
    // TODO: implement
    return {};
}

std::string MonitoringService::determineStatus(int /*stock*/, int /*activeSum*/) {
    return "";
}

double MonitoringService::calcRemainRatio(int /*stock*/, int /*activeSum*/) {
    return 0.0;
}

#pragma once
#include "model/MonitoringTypes.h"
#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"
#include <vector>
#include <string>
#include <memory>

class MonitoringService {
public:
    explicit MonitoringService(
        const std::string& sampleFilePath = "data/samples.json",
        const std::string& orderFilePath  = "data/orders.json"
    );

    OrderSummary            getOrderSummary()    const;
    std::vector<StockStatus> getStockStatusList() const;

private:
    std::unique_ptr<SampleRepository> m_sampleRepo;
    std::unique_ptr<OrderRepository>  m_orderRepo;

    int                calcActiveSum(const std::string& sampleId) const;
    static std::string determineStatus(int stock, int activeSum);
    static double      calcRemainRatio(int stock, int activeSum);
};

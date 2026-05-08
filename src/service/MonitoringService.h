#pragma once
#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"
#include <vector>
#include <string>
#include <memory>

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

    static std::string determineStatus(int stock, int activeSum);
    static double      calcRemainRatio(int stock, int activeSum);
};

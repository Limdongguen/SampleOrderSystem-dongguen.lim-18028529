#pragma once
#include "service/MonitoringService.h"
#include "service/ProductionService.h"
#include "view/MonitoringView.h"
#include <memory>

class MonitoringController {
public:
    explicit MonitoringController(std::shared_ptr<ProductionService> productionService);
    void run();

private:
    std::unique_ptr<MonitoringService>   m_monitoringService;
    std::shared_ptr<ProductionService>   m_productionService;
    MonitoringView                       m_view;

    void runOrderSummary();
    void runStockStatus();
    void runRealTime(bool isOrder);
    int  readSubMenuChoice() const;
};

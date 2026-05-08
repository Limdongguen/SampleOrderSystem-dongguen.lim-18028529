#pragma once
#include "model/MonitoringTypes.h"
#include <vector>

class MonitoringView {
public:
    void printOrderSummary(const OrderSummary& summary) const;
    void printStockStatusList(const std::vector<StockStatus>& list) const;
    void printSubMenu() const;
    void printCountdown(int seconds) const;
    void clearCountdown() const;
    void printEmptyData() const;
};

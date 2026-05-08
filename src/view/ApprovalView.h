#pragma once
#include "model/Order.h"
#include "model/Sample.h"
#include "model/ProductionJob.h"
#include <vector>
#include <string>

class ApprovalView {
public:
    void printReservedList(const std::vector<Order>& orders) const;
    void printSelectPrompt() const;
    void printStockInfo(const Sample& sample, int quantity) const;
    void printProductionInfo(const ProductionJob& job) const;
    void printApproveRejectPrompt() const;
    void printApproveRejectRetry() const;
    void printApproveSuccess(const std::string& orderId, const std::string& status) const;
    void printRejectSuccess(const std::string& orderId) const;
    void printError(const std::string& message) const;
    void printEmpty() const;
};

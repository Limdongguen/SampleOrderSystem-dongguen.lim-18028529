#pragma once
#include "model/Order.h"
#include "model/Sample.h"
#include <vector>
#include <string>

class OrderView {
public:
    void printSubMenu() const;
    void printSampleIdPrompt() const;
    void printCustomerNamePrompt() const;
    void printQuantityPrompt() const;
    void printOrderList(const std::vector<Order>& orders) const;
    void printOrderConfirm(const Order& order, const std::string& sampleName) const;
    void printReserveSuccess(const std::string& orderId) const;
    void printError(const std::string& message) const;
    void printNotFound() const;
};

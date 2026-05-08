#pragma once
#include "model/Order.h"
#include <vector>

class ReleaseView {
public:
    void printConfirmedList(const std::vector<Order>& orders) const;
    void printReleaseSuccess(const std::string& orderId, int quantity) const;
    void printNoConfirmedOrders() const;
    void printInvalidSelection() const;
    void printSelectionPrompt() const;
};

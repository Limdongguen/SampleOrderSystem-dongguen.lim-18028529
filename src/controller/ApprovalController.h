#pragma once
#include "service/OrderService.h"
#include "service/SampleService.h"
#include "service/ProductionService.h"
#include "view/ApprovalView.h"
#include <memory>

class ApprovalController {
public:
    ApprovalController();
    void run();

private:
    std::unique_ptr<OrderService>      m_orderService;
    std::unique_ptr<SampleService>     m_sampleService;
    ApprovalView                       m_view;

    int  readSelection(int maxNo) const;
    bool readApproveOrReject() const;
    void processOrder(const Order& order);
    void handleApprove(const Order& order);
    void handleReject(const Order& order);
};

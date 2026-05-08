#pragma once
#include "service/OrderService.h"
#include "service/SampleService.h"
#include "view/OrderView.h"
#include <memory>

class OrderController {
public:
    OrderController();
    void reserve();

private:
    std::unique_ptr<OrderService>  m_orderService;
    std::unique_ptr<SampleService> m_sampleService;
    OrderView                      m_view;

    std::string readSampleId() const;
    std::string readCustomerName() const;
    int         readQuantity() const;
};

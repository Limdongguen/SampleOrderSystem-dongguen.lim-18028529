#pragma once
#include "repository/OrderRepository.h"
#include "repository/SampleRepository.h"
#include "model/Order.h"
#include <vector>
#include <string>
#include <memory>

class ReleaseService {
public:
    explicit ReleaseService(
        const std::string& orderFilePath  = "data/orders.json",
        const std::string& sampleFilePath = "data/samples.json"
    );

    std::vector<Order> getConfirmedOrders() const;
    bool               release(const std::string& orderId);

private:
    std::unique_ptr<OrderRepository>  m_orderRepo;
    std::unique_ptr<SampleRepository> m_sampleRepo;
};

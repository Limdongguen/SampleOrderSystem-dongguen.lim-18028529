#pragma once
#include "repository/OrderRepository.h"
#include "repository/SampleRepository.h"
#include "model/Order.h"
#include <vector>
#include <string>
#include <memory>

class OrderService {
public:
    explicit OrderService(
        const std::string& sampleFilePath = "data/samples.json",
        const std::string& orderFilePath  = "data/orders.json"
    );

    bool reserve(const std::string& sampleId,
                 const std::string& customerName,
                 int quantity);

    std::vector<Order> getReservedOrders() const;
    std::vector<Order> getAllOrders() const;

private:
    std::unique_ptr<SampleRepository> m_sampleRepo;
    std::unique_ptr<OrderRepository>  m_orderRepo;

    bool isValidQuantity(int quantity) const;
    static std::string currentTimestamp();
};

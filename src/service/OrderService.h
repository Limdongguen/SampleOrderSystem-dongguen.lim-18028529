#pragma once
#include "repository/OrderRepository.h"
#include "repository/SampleRepository.h"
#include "service/ProductionService.h"
#include "model/Order.h"
#include <vector>
#include <string>
#include <memory>

class OrderService {
public:
    explicit OrderService(
        const std::string& sampleFilePath     = "data/samples.json",
        const std::string& orderFilePath      = "data/orders.json",
        const std::string& productionFilePath = "data/production_jobs.json"
    );

    bool reserve(const std::string& sampleId,
                 const std::string& customerName,
                 int quantity);

    bool approve(const std::string& orderId);
    bool reject(const std::string& orderId);

    std::vector<Order> getReservedOrders() const;
    std::vector<Order> getAllOrders() const;

private:
    std::unique_ptr<SampleRepository>  m_sampleRepo;
    std::unique_ptr<OrderRepository>   m_orderRepo;
    std::unique_ptr<ProductionService> m_productionService;

    bool  isValidQuantity(int quantity) const;
    Order buildOrder(const std::string& sampleId,
                     const std::string& customerName,
                     int quantity) const;
    bool  approveWithSufficientStock(Order& order, Sample& sample);
    bool  approveWithInsufficientStock(Order& order, const Sample& sample);
    static std::string currentTimestamp();
};

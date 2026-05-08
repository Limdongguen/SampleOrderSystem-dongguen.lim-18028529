#pragma once
#include "model/Order.h"
#include "model/Enums.h"
#include <vector>
#include <optional>
#include <string>

class OrderRepository {
public:
    explicit OrderRepository(const std::string& filePath = "data/orders.json");

    void add(const Order& order);
    std::vector<Order> findAll() const;
    std::optional<Order> findById(const std::string& id) const;
    std::vector<Order> findByStatus(OrderStatus status) const;
    void update(const Order& order);
    void save() const;

private:
    std::string        m_filePath;
    std::vector<Order> m_orders;
};

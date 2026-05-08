#include "repository/OrderRepository.h"
#include "util/JsonFileManager.h"
#include <algorithm>

OrderRepository::OrderRepository(const std::string& filePath)
    : m_filePath(filePath)
{
    auto json = JsonFileManager::load(m_filePath);
    for (const auto& item : json) {
        m_orders.push_back(item.get<Order>());
    }
}

void OrderRepository::add(const Order& order) {
    m_orders.push_back(order);
}

std::vector<Order> OrderRepository::findAll() const {
    return m_orders;
}

std::optional<Order> OrderRepository::findById(const std::string& id) const {
    for (const auto& o : m_orders) {
        if (o.orderId == id) {
            return o;
        }
    }
    return std::nullopt;
}

std::vector<Order> OrderRepository::findByStatus(OrderStatus status) const {
    std::vector<Order> result;
    for (const auto& o : m_orders) {
        if (o.status == status) {
            result.push_back(o);
        }
    }
    return result;
}

void OrderRepository::update(const Order& order) {
    for (auto& o : m_orders) {
        if (o.orderId == order.orderId) {
            o = order;
            return;
        }
    }
}

void OrderRepository::save() const {
    nlohmann::json jsonArr = nlohmann::json::array();
    for (const auto& o : m_orders) {
        nlohmann::json j;
        to_json(j, o);
        jsonArr.push_back(j);
    }
    JsonFileManager::save(m_filePath, jsonArr);
}

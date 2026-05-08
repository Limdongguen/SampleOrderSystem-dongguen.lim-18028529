#include "repository/OrderRepository.h"
#include "util/JsonFileManager.h"

OrderRepository::OrderRepository(const std::string& filePath)
    : m_filePath(filePath), m_dirty(false)
{
    reload();
}

void OrderRepository::reload() const {
    m_orders.clear();
    auto json = JsonFileManager::load(m_filePath);
    for (const auto& item : json) m_orders.push_back(item.get<Order>());
    m_dirty = false;
}

void OrderRepository::ensureFresh() const {
    if (!m_dirty) reload();
}

void OrderRepository::add(const Order& order) {
    ensureFresh();
    m_orders.push_back(order);
    m_dirty = true;
}

std::vector<Order> OrderRepository::findAll() const {
    ensureFresh();
    return m_orders;
}

std::optional<Order> OrderRepository::findById(const std::string& id) const {
    ensureFresh();
    for (const auto& o : m_orders) {
        if (o.orderId == id) return o;
    }
    return std::nullopt;
}

std::vector<Order> OrderRepository::findByStatus(OrderStatus status) const {
    ensureFresh();
    std::vector<Order> result;
    for (const auto& o : m_orders) {
        if (o.status == status) result.push_back(o);
    }
    return result;
}

void OrderRepository::update(const Order& order) {
    ensureFresh();
    for (auto& o : m_orders) {
        if (o.orderId == order.orderId) { o = order; m_dirty = true; return; }
    }
}

void OrderRepository::save() const {
    nlohmann::json jsonArr = nlohmann::json::array();
    for (const auto& o : m_orders) {
        nlohmann::json j; to_json(j, o); jsonArr.push_back(std::move(j));
    }
    JsonFileManager::save(m_filePath, jsonArr);
    m_dirty = false;
}

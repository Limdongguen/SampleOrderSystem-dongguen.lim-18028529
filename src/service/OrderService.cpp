#include "service/OrderService.h"
#include "util/IdGenerator.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace {
    constexpr int kQuantityMin = 1;
}

OrderService::OrderService(const std::string& sampleFilePath,
                           const std::string& orderFilePath)
    : m_sampleRepo(std::make_unique<SampleRepository>(sampleFilePath))
    , m_orderRepo(std::make_unique<OrderRepository>(orderFilePath))
{
}

bool OrderService::reserve(const std::string& sampleId,
                           const std::string& customerName,
                           int quantity)
{
    if (!isValidQuantity(quantity)) return false;
    if (!m_sampleRepo->findById(sampleId).has_value()) return false;

    Order order = buildOrder(sampleId, customerName, quantity);
    m_orderRepo->add(order);
    m_orderRepo->save();
    return true;
}

Order OrderService::buildOrder(const std::string& sampleId,
                               const std::string& customerName,
                               int quantity) const
{
    Order order;
    order.orderId      = IdGenerator::nextOrderId();
    order.sampleId     = sampleId;
    order.customerName = customerName;
    order.quantity     = quantity;
    order.status       = OrderStatus::RESERVED;
    order.createdAt    = currentTimestamp();
    order.updatedAt    = order.createdAt;
    return order;
}

std::vector<Order> OrderService::getReservedOrders() const {
    return m_orderRepo->findByStatus(OrderStatus::RESERVED);
}

std::vector<Order> OrderService::getAllOrders() const {
    return m_orderRepo->findAll();
}

bool OrderService::isValidQuantity(int quantity) const {
    return quantity >= kQuantityMin;
}

std::string OrderService::currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_val{};
#ifdef _WIN32
    localtime_s(&tm_val, &t);
#else
    localtime_r(&t, &tm_val);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

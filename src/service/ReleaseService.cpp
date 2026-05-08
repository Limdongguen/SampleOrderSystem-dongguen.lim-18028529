#include "service/ReleaseService.h"
#include "util/TimeUtil.h"

ReleaseService::ReleaseService(
    const std::string& orderFilePath,
    const std::string& sampleFilePath
)
    : m_orderRepo (std::make_unique<OrderRepository>(orderFilePath))
    , m_sampleRepo(std::make_unique<SampleRepository>(sampleFilePath))
{
}

std::vector<Order> ReleaseService::getConfirmedOrders() const {
    return m_orderRepo->findByStatus(OrderStatus::CONFIRMED);
}

bool ReleaseService::release(const std::string& orderId) {
    auto orderOpt = m_orderRepo->findById(orderId);
    if (!orderOpt.has_value()) return false;

    Order order = orderOpt.value();
    if (order.status != OrderStatus::CONFIRMED) return false;

    auto sampleOpt = m_sampleRepo->findById(order.sampleId);
    if (!sampleOpt.has_value()) return false;

    Sample sample = sampleOpt.value();
    sample.stock  -= order.quantity;

    order.status    = OrderStatus::RELEASED;
    order.updatedAt = TimeUtil::nowString();

    m_sampleRepo->update(sample);
    m_sampleRepo->save();

    m_orderRepo->update(order);
    m_orderRepo->save();

    return true;
}

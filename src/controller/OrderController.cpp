#include "controller/OrderController.h"
#include "util/ConsoleHelper.h"

namespace {
    constexpr int kQuantityMin = 1;
    constexpr int kQuantityMax = 100000;
}

OrderController::OrderController()
    : m_orderService(std::make_unique<OrderService>())
    , m_sampleService(std::make_unique<SampleService>())
{
}

void OrderController::reserve() {
    m_view.printSubMenu();

    std::string sampleId = readSampleId();
    auto sample = m_sampleService->getSampleById(sampleId);
    if (!sample.has_value()) {
        m_view.printNotFound();
        return;
    }

    std::string customerName = readCustomerName();
    int quantity = readQuantity();

    Order preview;
    preview.sampleId     = sampleId;
    preview.customerName = customerName;
    preview.quantity     = quantity;
    m_view.printOrderConfirm(preview, sample->name);

    bool ok = m_orderService->reserve(sampleId, customerName, quantity);
    if (ok) {
        auto orders = m_orderService->getAllOrders();
        if (!orders.empty()) {
            m_view.printReserveSuccess(orders.back().orderId);
        }
    } else {
        m_view.printError("주문 접수에 실패했습니다.");
    }
}

std::string OrderController::readSampleId() const {
    m_view.printSampleIdPrompt();
    return ConsoleHelper::readLine();
}

std::string OrderController::readCustomerName() const {
    m_view.printCustomerNamePrompt();
    return ConsoleHelper::readLine();
}

int OrderController::readQuantity() const {
    m_view.printQuantityPrompt();
    return ConsoleHelper::readInt(kQuantityMin, kQuantityMax);
}

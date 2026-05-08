#include "controller/ApprovalController.h"
#include "util/ConsoleHelper.h"
#include <string>

namespace {
    constexpr int kMenuBack = 0;
    constexpr int kMenuMin  = 0;
}

ApprovalController::ApprovalController()
    : m_orderService(std::make_unique<OrderService>())
    , m_sampleService(std::make_unique<SampleService>())
{
}

void ApprovalController::run() {
    auto reserved = m_orderService->getReservedOrders();
    if (reserved.empty()) {
        m_view.printEmpty();
        return;
    }

    m_view.printReservedList(reserved);
    m_view.printSelectPrompt();

    int sel = readSelection(static_cast<int>(reserved.size()));
    if (sel == kMenuBack) return;

    processOrder(reserved[sel - 1]);
}

int ApprovalController::readSelection(int maxNo) const {
    return ConsoleHelper::readInt(kMenuBack, maxNo);
}

void ApprovalController::processOrder(const Order& order) {
    auto sampleOpt = m_sampleService->getSampleById(order.sampleId);
    if (!sampleOpt.has_value()) {
        m_view.printError("시료 정보를 찾을 수 없습니다.");
        return;
    }
    m_view.printStockInfo(sampleOpt.value(), order.quantity);
    m_view.printApproveRejectPrompt();

    bool approve = readApproveOrReject();
    if (approve) {
        handleApprove(order);
    } else {
        handleReject(order);
    }
}

bool ApprovalController::readApproveOrReject() const {
    std::string input;
    while (true) {
        input = ConsoleHelper::readLine();
        if (input == "Y" || input == "y") return true;
        if (input == "N" || input == "n") return false;
        m_view.printApproveRejectRetry();
    }
}

void ApprovalController::handleApprove(const Order& order) {
    bool ok = m_orderService->approve(order.orderId);
    if (!ok) {
        m_view.printError("승인 처리에 실패했습니다.");
        return;
    }
    auto allOrders = m_orderService->getAllOrders();
    for (const auto& o : allOrders) {
        if (o.orderId == order.orderId) {
            m_view.printApproveSuccess(o.orderId, orderStatusToString(o.status));
            return;
        }
    }
}

void ApprovalController::handleReject(const Order& order) {
    bool ok = m_orderService->reject(order.orderId);
    if (ok) {
        m_view.printRejectSuccess(order.orderId);
    } else {
        m_view.printError("거절 처리에 실패했습니다.");
    }
}

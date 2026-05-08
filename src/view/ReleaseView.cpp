#include "view/ReleaseView.h"
#include <iostream>
#include <iomanip>

void ReleaseView::printConfirmedList(const std::vector<Order>& orders) const {
    std::cout << "\n  [출고 가능 주문 목록 - CONFIRMED]\n";
    std::cout << "  " << std::setw(4) << "번호"
              << "  " << std::setw(20) << "주문번호"
              << "  " << std::setw(12) << "고객명"
              << "  " << std::setw(8)  << "시료ID"
              << "  " << std::setw(6)  << "수량" << "\n";
    std::cout << "  " << std::string(54, '-') << "\n";

    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& o = orders[i];
        std::cout << "  " << std::setw(4) << (i + 1)
                  << "  " << std::setw(20) << o.orderId
                  << "  " << std::setw(12) << o.customerName
                  << "  " << std::setw(8)  << o.sampleId
                  << "  " << std::setw(6)  << o.quantity << "\n";
    }
}

void ReleaseView::printReleaseSuccess(const std::string& orderId, int quantity) const {
    std::cout << "\n  출고 완료!\n";
    std::cout << "  주문번호 : " << orderId << "\n";
    std::cout << "  출고 수량 : " << quantity << " ea\n";
}

void ReleaseView::printNoConfirmedOrders() const {
    std::cout << "\n  출고 가능한 주문이 없습니다.\n";
}

void ReleaseView::printInvalidSelection() const {
    std::cout << "  잘못된 선택입니다. 다시 시도하세요.\n";
}

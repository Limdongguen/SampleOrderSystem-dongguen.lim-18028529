#include "view/ReleaseView.h"
#include <iostream>
#include <iomanip>

namespace {
    constexpr int kNoW      =  4;
    constexpr int kOrderW   = 20;
    constexpr int kCustW    = 12;
    constexpr int kSampleW  =  8;
    constexpr int kQtyW     =  6;
    constexpr int kSepLen   = 54;
}

void ReleaseView::printConfirmedList(const std::vector<Order>& orders) const {
    std::cout << "\n  [출고 가능 주문 목록 - CONFIRMED]\n";
    std::cout << "  " << std::setw(kNoW)    << "번호"
              << "  " << std::setw(kOrderW) << "주문번호"
              << "  " << std::setw(kCustW)  << "고객명"
              << "  " << std::setw(kSampleW)<< "시료ID"
              << "  " << std::setw(kQtyW)   << "수량" << "\n";
    std::cout << "  " << std::string(kSepLen, '-') << "\n";

    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& o = orders[i];
        std::cout << "  " << std::setw(kNoW)    << (i + 1)
                  << "  " << std::setw(kOrderW) << o.orderId
                  << "  " << std::setw(kCustW)  << o.customerName
                  << "  " << std::setw(kSampleW)<< o.sampleId
                  << "  " << std::setw(kQtyW)   << o.quantity << "\n";
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

void ReleaseView::printSelectionPrompt() const {
    std::cout << "\n  번호 선택 (0: 뒤로): ";
}

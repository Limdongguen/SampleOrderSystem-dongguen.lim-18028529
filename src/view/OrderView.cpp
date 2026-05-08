#include "view/OrderView.h"
#include <iostream>
#include <iomanip>

namespace {
    constexpr int kOrderIdW  = 20;
    constexpr int kSampleW   = 10;
    constexpr int kCustW     = 15;
    constexpr int kQtyW      =  8;
    constexpr int kStatusW   = 12;
    constexpr int kSepLen    = 70;
}

static void printSep() {
    std::cout << "  " << std::string(kSepLen, '-') << "\n";
}

void OrderView::printSubMenu() const {
    std::cout << "\n";
    std::cout << "  ┌─────────────────────────┐\n";
    std::cout << "  │      주문 접수 메뉴      │\n";
    std::cout << "  └─────────────────────────┘\n";
}

void OrderView::printSampleIdPrompt() const {
    std::cout << "  시료 ID  >> ";
}

void OrderView::printCustomerNamePrompt() const {
    std::cout << "  고객명   >> ";
}

void OrderView::printQuantityPrompt() const {
    std::cout << "  주문 수량 (ea) >> ";
}

void OrderView::printOrderList(const std::vector<Order>& orders) const {
    std::cout << "\n";
    if (orders.empty()) {
        std::cout << "  주문 내역이 없습니다.\n";
        return;
    }
    printSep();
    std::cout << "  주문 목록 (" << orders.size() << "건)\n";
    printSep();
    std::cout << "  " << std::left
              << std::setw(kOrderIdW) << "주문번호"
              << std::setw(kSampleW)  << "시료ID"
              << std::setw(kCustW)    << "고객명"
              << std::setw(kQtyW)     << "수량"
              << std::setw(kStatusW)  << "상태"
              << "접수일시\n";
    printSep();
    for (const auto& o : orders) {
        std::cout << "  " << std::left
                  << std::setw(kOrderIdW) << o.orderId
                  << std::setw(kSampleW)  << o.sampleId
                  << std::setw(kCustW)    << o.customerName
                  << std::setw(kQtyW)     << o.quantity
                  << std::setw(kStatusW)  << orderStatusToString(o.status)
                  << o.createdAt << "\n";
    }
    printSep();
}

void OrderView::printOrderConfirm(const Order& order,
                                  const std::string& sampleName) const {
    std::cout << "\n";
    printSep();
    std::cout << "  주문 접수 확인\n";
    printSep();
    std::cout << "  시료 ID   : " << order.sampleId     << "  (" << sampleName << ")\n";
    std::cout << "  고객명    : " << order.customerName  << "\n";
    std::cout << "  수량      : " << order.quantity      << " ea\n";
    printSep();
}

void OrderView::printReserveSuccess(const std::string& orderId) const {
    std::cout << "\n  ✔ 주문 접수 완료 : " << orderId
              << "  [상태: RESERVED]\n";
}

void OrderView::printError(const std::string& message) const {
    std::cout << "\n  [오류] " << message << "\n";
}

void OrderView::printNotFound() const {
    std::cout << "\n  해당 시료 ID를 찾을 수 없습니다.\n";
}

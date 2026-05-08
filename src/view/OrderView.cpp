#include "view/OrderView.h"
#include <iostream>
#include <iomanip>

namespace {
    constexpr int kOrderIdW = 20;
    constexpr int kSampleW  = 10;
    constexpr int kCustW    = 15;
    constexpr int kQtyW     =  8;
    constexpr int kStatusW  = 12;
    constexpr int kSepLen   = 70;
}

static void printSep() {
    std::cout << "  " << std::string(kSepLen, '-') << "\n";
}

// 한글은 터미널에서 2열 차지 → 우측 테두리 없이 좌측 테두리만 사용
void OrderView::printSubMenu() const {
    std::cout << "\n";
    std::cout << "  ┌──────────────────────────\n";
    std::cout << "  │  주문 접수 메뉴\n";
    std::cout << "  └──────────────────────────\n";
}

void OrderView::printSampleIdPrompt() const {
    std::cout << "  시료 ID  >> ";
}

void OrderView::printCustomerNamePrompt() const {
    std::cout << "  고객명   >> ";
}

void OrderView::printQuantityPrompt() const {
    std::cout << "  주문 수량(ea) >> ";
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
    // 열 display 너비: 주문번호=20, 시료ID=10, 고객명=15, 수량=8, 상태=12
    // 주문번호(8열)+12공백=20, 시료ID(6열)+4공백=10, 고객명(6열)+9공백=15, 수량(4열)+4공백=8, 상태(4열)+8공백=12
    std::cout << "  주문번호            시료ID    고객명         수량    상태        접수일시\n";
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
    std::cout << "  시료 ID  : " << order.sampleId     << "  (" << sampleName << ")\n";
    std::cout << "  고객명   : " << order.customerName  << "\n";
    std::cout << "  수량     : " << order.quantity      << " ea\n";
    printSep();
}

void OrderView::printReserveSuccess(const std::string& orderId) const {
    std::cout << "\n  >> 주문 접수 완료 : " << orderId
              << "  [상태: RESERVED]\n";
}

void OrderView::printError(const std::string& message) const {
    std::cout << "\n  [오류] " << message << "\n";
}

void OrderView::printNotFound() const {
    std::cout << "\n  해당 시료 ID를 찾을 수 없습니다.\n";
}

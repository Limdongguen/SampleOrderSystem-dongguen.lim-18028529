#include "view/ReleaseView.h"
#include <iostream>
#include <iomanip>

namespace {
    constexpr int kNoW    =  4;
    constexpr int kOrderW = 20;
    constexpr int kCustW  = 12;
    constexpr int kSampleW=  8;
    constexpr int kQtyW   =  6;
    constexpr int kSepLen = 58; // 2+4+2+20+2+12+2+8+2+6 = 60 display, prefix 2 포함
}

static void printSep() {
    std::cout << "  " << std::string(kSepLen, '-') << "\n";
}

void ReleaseView::printConfirmedList(const std::vector<Order>& orders) const {
    printSep();
    std::cout << "  출고 가능 주문 목록 (CONFIRMED)\n";
    printSep();
    // 열 display 너비: 번호=4, 주문번호=20, 고객명=12, 시료ID=8, 수량=6
    // 번호(4열), 주문번호(8열)+12공백=20, 고객명(6열)+6공백=12, 시료ID(6열)+2공백=8, 수량(4열)+2공백=6
    std::cout << "  번호  주문번호            고객명      시료ID    수량\n";
    printSep();
    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& o = orders[i];
        std::cout << "  " << std::setw(kNoW)    << (i + 1)
                  << "  " << std::setw(kOrderW) << o.orderId
                  << "  " << std::setw(kCustW)  << o.customerName
                  << "  " << std::setw(kSampleW)<< o.sampleId
                  << "  " << std::setw(kQtyW)   << o.quantity << "\n";
    }
    printSep();
}

void ReleaseView::printReleaseSuccess(const std::string& orderId, int quantity) const {
    std::cout << "\n  >> 출고 완료\n";
    std::cout << "     주문번호 : " << orderId  << "\n";
    std::cout << "     출고 수량 : " << quantity << " ea\n";
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

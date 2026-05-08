#include "view/ApprovalView.h"
#include "model/Enums.h"
#include <iostream>
#include <iomanip>

namespace {
    constexpr int kOrderIdW = 20;
    constexpr int kSampleW  = 10;
    constexpr int kCustW    = 15;
    constexpr int kQtyW     =  8;
    constexpr int kSepLen   = 70;
}

static void printApprovalSep() {
    std::cout << "  " << std::string(kSepLen, '-') << "\n";
}

void ApprovalView::printReservedList(const std::vector<Order>& orders) const {
    std::cout << "\n";
    printApprovalSep();
    std::cout << "  승인 대기 주문 목록 (" << orders.size() << "건)\n";
    printApprovalSep();
    // 열 display 너비: No.=4, 주문번호=20, 시료ID=10, 고객명=15, 수량=8
    // 주문번호(8열)+12공백=20, 시료ID(6열)+4공백=10, 고객명(6열)+9공백=15, 수량(4열)+4공백=8
    std::cout << "  No. 주문번호            시료ID    고객명         수량    접수일시\n";
    printApprovalSep();
    int no = 1;
    for (const auto& o : orders) {
        std::cout << "  " << std::left
                  << std::setw(4)         << no++
                  << std::setw(kOrderIdW) << o.orderId
                  << std::setw(kSampleW)  << o.sampleId
                  << std::setw(kCustW)    << o.customerName
                  << std::setw(kQtyW)     << o.quantity
                  << o.createdAt << "\n";
    }
    printApprovalSep();
}

void ApprovalView::printSelectPrompt() const {
    std::cout << "  처리할 번호 선택 (0=뒤로) >> ";
}

void ApprovalView::printStockInfo(const Sample& sample, int quantity) const {
    std::cout << "\n";
    printApprovalSep();
    std::cout << "  재고 확인\n";
    printApprovalSep();
    std::cout << "  시료명    : " << sample.name  << "\n";
    std::cout << "  현재 재고 : " << sample.stock << " ea\n";
    std::cout << "  주문 수량 : " << quantity      << " ea\n";
    int shortage = quantity - sample.stock;
    if (shortage <= 0) {
        std::cout << "  부족분    : 없음 (재고 충분)\n";
    } else {
        std::cout << "  부족분    : " << shortage << " ea\n";
    }
    printApprovalSep();
}

void ApprovalView::printProductionInfo(const ProductionJob& job) const {
    std::cout << "  실 생산량 : " << job.actualProduction << " ea\n";
    std::cout << "  예상 소요 : " << job.totalTime        << " min\n";
}

void ApprovalView::printApproveRejectPrompt() const {
    std::cout << "  [Y] 승인  [N] 거절 >> ";
}

void ApprovalView::printApproveSuccess(const std::string& orderId,
                                       const std::string& status) const {
    std::cout << "\n  >> 승인 완료 : " << orderId
              << "  [상태: " << status << "]\n";
}

void ApprovalView::printRejectSuccess(const std::string& orderId) const {
    std::cout << "\n  >> 거절 완료 : " << orderId
              << "  [상태: REJECTED]\n";
}

void ApprovalView::printError(const std::string& message) const {
    std::cout << "\n  [오류] " << message << "\n";
}

void ApprovalView::printApproveRejectRetry() const {
    std::cout << "  [Y] 또는 [N]을 입력하세요 >> ";
}

void ApprovalView::printEmpty() const {
    std::cout << "\n  승인 대기 중인 주문이 없습니다.\n";
}

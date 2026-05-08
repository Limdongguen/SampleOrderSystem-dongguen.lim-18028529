#include "view/MonitoringView.h"
#include <iostream>
#include <iomanip>
#include <string>

namespace {
    constexpr int kClearWidth    = 60;
    constexpr int kOrderSepLen   = 38; // 주문량 표 구분선
    constexpr int kStockSepLen   = 60; // 재고량 표 구분선
}

static void printSep(int len) {
    std::cout << "  " << std::string(len, '-') << "\n";
}

void MonitoringView::printSubMenu() const {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════\n";
    std::cout << "  ║  [4] 모니터링\n";
    std::cout << "  ╠══════════════════════════════\n";
    std::cout << "  ║  [1] 주문량 확인\n";
    std::cout << "  ║  [2] 재고량 확인\n";
    std::cout << "  ║  [0] 뒤로\n";
    std::cout << "  ╚══════════════════════════════\n";
    std::cout << "  선택 >> ";
}

void MonitoringView::printOrderSummary(const OrderSummary& summary) const {
    printSep(kOrderSepLen);
    std::cout << "  주문량 현황\n";
    printSep(kOrderSepLen);
    std::cout << "  상태              건수\n";
    printSep(kOrderSepLen);
    std::cout << "  RESERVED      " << std::setw(5) << summary.reserved  << "\n";
    std::cout << "  CONFIRMED     " << std::setw(5) << summary.confirmed << "\n";
    std::cout << "  PRODUCING     " << std::setw(5) << summary.producing
              << "  <- 생산라인 가동 중\n";
    std::cout << "  RELEASED      " << std::setw(5) << summary.released  << "\n";
    printSep(kOrderSepLen);
    std::cout << "  합계          " << std::setw(5) << summary.total     << "\n";
    printSep(kOrderSepLen);
}

void MonitoringView::printStockStatusList(const std::vector<StockStatus>& list) const {
    if (list.empty()) {
        printEmptyData();
        return;
    }
    printSep(kStockSepLen);
    std::cout << "  재고량 현황\n";
    printSep(kStockSepLen);
    // 열 display 너비: sampleId=8, name=14, stock=8, activeSum=10, status=8
    // 시료ID(6열)+2공백=8, 이름(4열)+10공백=14, 재고(4열)+4공백=8, 활성주문(8열)+2공백=10, 상태(4열)+4공백=8
    std::cout << "  시료ID  이름          재고    활성주문  상태    잔여율\n";
    printSep(kStockSepLen);
    for (const auto& ss : list) {
        std::cout << "  " << std::left
                  << std::setw(8)  << ss.sampleId
                  << std::setw(14) << ss.name
                  << std::setw(8)  << ss.stock
                  << std::setw(10) << ss.activeSum
                  << std::setw(8)  << ss.status;
        std::cout << std::fixed << std::setprecision(0) << ss.remainRatio << "%\n";
    }
    printSep(kStockSepLen);
}

void MonitoringView::printCountdown(int seconds) const {
    std::cout << "\r  [ R: 즉시갱신 | 0: 뒤로 | 자동갱신까지 "
              << seconds << "초 ]   " << std::flush;
}

void MonitoringView::clearCountdown() const {
    std::cout << "\r" << std::string(kClearWidth, ' ') << "\r" << std::flush;
}

void MonitoringView::printEmptyData() const {
    std::cout << "\n  데이터 없음\n";
}

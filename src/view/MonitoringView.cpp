#include "view/MonitoringView.h"
#include <iostream>
#include <iomanip>
#include <string>

namespace {
    constexpr int kClearWidth = 60;
    constexpr int kSepLen     = 56;
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
    std::cout << "\n  ── 주문량 현황 ──\n";
    std::cout << "  상태          건수\n";
    std::cout << "  RESERVED    " << std::setw(5) << summary.reserved  << "\n";
    std::cout << "  CONFIRMED   " << std::setw(5) << summary.confirmed << "\n";
    std::cout << "  PRODUCING   " << std::setw(5) << summary.producing << "  ← 생산라인 가동 중\n";
    std::cout << "  RELEASED    " << std::setw(5) << summary.released  << "\n";
    std::cout << "  ─────────────────\n";
    std::cout << "  합계        " << std::setw(5) << summary.total     << "\n";
}

void MonitoringView::printStockStatusList(const std::vector<StockStatus>& list) const {
    if (list.empty()) {
        printEmptyData();
        return;
    }
    std::cout << "\n  ── 재고량 현황 ──\n";
    std::cout << "  " << std::left
              << std::setw(8)  << "시료ID"
              << std::setw(14) << "이름"
              << std::setw(8)  << "재고"
              << std::setw(10) << "활성주문"
              << std::setw(8)  << "상태"
              << "잔여율\n";
    std::cout << "  " << std::string(kSepLen, '-') << "\n";
    for (const auto& ss : list) {
        std::cout << "  " << std::left
                  << std::setw(8)  << ss.sampleId
                  << std::setw(14) << ss.name
                  << std::setw(8)  << ss.stock
                  << std::setw(10) << ss.activeSum
                  << std::setw(8)  << ss.status;
        std::cout << std::fixed << std::setprecision(0) << ss.remainRatio << "%\n";
    }
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

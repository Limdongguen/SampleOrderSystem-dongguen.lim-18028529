#include "view/MainView.h"
#include <iostream>

void MainView::printMenu() const {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════╗\n";
    std::cout << "  ║   S-Semi 시료 생산주문관리 시스템  ║\n";
    std::cout << "  ╠══════════════════════════════════╣\n";
    std::cout << "  ║  [1] 시료 관리                   ║\n";
    std::cout << "  ║  [2] 시료 주문                   ║\n";
    std::cout << "  ║  [3] 주문 승인/거절              ║\n";
    std::cout << "  ║  [4] 모니터링                    ║\n";
    std::cout << "  ║  [5] 생산라인 조회               ║\n";
    std::cout << "  ║  [6] 출고 처리                   ║\n";
    std::cout << "  ║  [0] 종료                        ║\n";
    std::cout << "  ╚══════════════════════════════════╝\n";
    std::cout << "  선택 >> ";
}

void MainView::printNotImplemented(const std::string& feature) const {
    std::cout << "\n  [ 미구현 ] " << feature << "\n";
}

void MainView::printExit() const {
    std::cout << "\n  시스템을 종료합니다. 안녕히 가세요.\n\n";
}

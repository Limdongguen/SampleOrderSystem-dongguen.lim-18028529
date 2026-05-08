#include "controller/MainController.h"
#include "util/ConsoleHelper.h"
#include <iostream>

namespace {
    constexpr int kMenuMin  = 0;
    constexpr int kMenuMax  = 6;
    constexpr int kMenuExit = 0;
}

void MainController::run() {
    int choice = -1;
    while (choice != kMenuExit) {
        printMenu();
        choice = readChoice();
        handleChoice(choice);
    }
}

void MainController::printMenu() const {
    std::cout << "\n==============================\n";
    std::cout << "   S-Semi 시료 생산주문관리\n";
    std::cout << "==============================\n";
    std::cout << " [1] 시료 관리\n";
    std::cout << " [2] 시료 주문\n";
    std::cout << " [3] 주문 승인/거절\n";
    std::cout << " [4] 모니터링\n";
    std::cout << " [5] 생산라인 조회\n";
    std::cout << " [6] 출고 처리\n";
    std::cout << " [0] 종료\n";
    std::cout << "------------------------------\n";
    std::cout << "선택: ";
}

int MainController::readChoice() const {
    return ConsoleHelper::readInt(kMenuMin, kMenuMax);
}

void MainController::handleChoice(int choice) {
    switch (choice) {
    case 1: std::cout << "[1] 시료 관리 - 미구현\n"; break;
    case 2: std::cout << "[2] 시료 주문 - 미구현\n"; break;
    case 3: std::cout << "[3] 주문 승인/거절 - 미구현\n"; break;
    case 4: std::cout << "[4] 모니터링 - 미구현\n"; break;
    case 5: std::cout << "[5] 생산라인 조회 - 미구현\n"; break;
    case 6: std::cout << "[6] 출고 처리 - 미구현\n"; break;
    case 0: std::cout << "시스템을 종료합니다.\n"; break;
    default: break;
    }
}

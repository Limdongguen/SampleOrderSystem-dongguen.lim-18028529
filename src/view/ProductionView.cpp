#include "view/ProductionView.h"
#include "util/TimeUtil.h"
#include <iostream>
#include <iomanip>

namespace {
    constexpr int kSecondsPerMinute = 60;
    constexpr int kNoW    =  4;
    constexpr int kOrderW = 20;
    constexpr int kSampleW=  8;
    constexpr int kProdW  = 10;
    constexpr int kTimeW  = 12;
    constexpr int kSepLen = 62; // 2+4+2+20+2+8+2+10+2+12 = 64 display, prefix 2 포함
}

static void printSep() {
    std::cout << "  " << std::string(kSepLen, '-') << "\n";
}

void ProductionView::printCurrentJob(const ProductionJob& job, int secondsLeft) const {
    printSep();
    std::cout << "  현재 생산 중\n";
    printSep();
    std::cout << "  주문번호  : " << job.orderId          << "\n";
    std::cout << "  시료ID    : " << job.sampleId         << "\n";
    std::cout << "  부족분    : " << job.shortage         << " ea\n";
    std::cout << "  실 생산량 : " << job.actualProduction << " ea\n";
    std::cout << "  완료 예정 : " << job.estimatedEndTime << "\n";
    if (secondsLeft > 0) {
        int mins = secondsLeft / kSecondsPerMinute;
        int secs = secondsLeft % kSecondsPerMinute;
        std::cout << "  남은 시간 : " << mins << "분 " << secs << "초 후\n";
    } else {
        std::cout << "  남은 시간 : 완료 처리 중...\n";
    }
    printSep();
}

void ProductionView::printWaitingQueue(const std::vector<ProductionJob>& waitingJobs) const {
    if (waitingJobs.empty()) return;
    std::cout << "\n";
    printSep();
    std::cout << "  대기 큐\n";
    printSep();
    // 열 display 너비: 순서=4, 주문번호=20, 시료ID=8, 실생산량=10, 예상소요(분)=12
    // 순서(4열)+0=4, 주문번호(8열)+12공백=20, 시료ID(6열)+2공백=8, 실생산량(8열)+2공백=10, 예상소요(분)(12열)=12
    std::cout << "  순서  주문번호            시료ID    실생산량  예상소요(분)\n";
    printSep();
    for (int i = 0; i < static_cast<int>(waitingJobs.size()); ++i) {
        const auto& j = waitingJobs[i];
        std::cout << "  " << std::setw(kNoW)    << (i + 1)
                  << "  " << std::setw(kOrderW) << j.orderId
                  << "  " << std::setw(kSampleW)<< j.sampleId
                  << "  " << std::setw(kProdW)  << j.actualProduction
                  << "  " << std::setw(kTimeW)  << j.totalTime << "\n";
    }
    printSep();
}

void ProductionView::printNoJobRunning() const {
    std::cout << "\n  현재 생산 중인 작업 없음\n";
}

void ProductionView::printBackPrompt() const {
    std::cout << "\n  [0] 뒤로\n  선택: ";
}

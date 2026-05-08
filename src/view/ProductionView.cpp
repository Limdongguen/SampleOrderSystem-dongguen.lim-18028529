#include "view/ProductionView.h"
#include "util/TimeUtil.h"
#include <iostream>
#include <iomanip>

namespace {
    constexpr int kSecondsPerMinute = 60;
    constexpr int kNoW       =  4;
    constexpr int kOrderW    = 20;
    constexpr int kSampleW   =  8;
    constexpr int kProdW     = 10;
    constexpr int kTimeW     = 12;
    constexpr int kSepLen    = 58;
}

void ProductionView::printCurrentJob(const ProductionJob& job, int secondsLeft) const {
    std::cout << "\n  [현재 생산 중]\n";
    std::cout << "  주문번호    : " << job.orderId << "\n";
    std::cout << "  시료ID      : " << job.sampleId << "\n";
    std::cout << "  부족분      : " << job.shortage << " ea\n";
    std::cout << "  실 생산량   : " << job.actualProduction << " ea\n";
    std::cout << "  완료 예정   : " << job.estimatedEndTime << "\n";

    if (secondsLeft > 0) {
        int mins = secondsLeft / kSecondsPerMinute;
        int secs = secondsLeft % kSecondsPerMinute;
        std::cout << "  남은 시간   : " << mins << "분 " << secs << "초 후\n";
    } else {
        std::cout << "  남은 시간   : 완료 처리 중...\n";
    }
}

void ProductionView::printWaitingQueue(const std::vector<ProductionJob>& waitingJobs) const {
    if (waitingJobs.empty()) return;
    std::cout << "\n  [대기 큐]\n";
    std::cout << "  " << std::setw(kNoW)    << "순서"
              << "  " << std::setw(kOrderW) << "주문번호"
              << "  " << std::setw(kSampleW)<< "시료ID"
              << "  " << std::setw(kProdW)  << "실생산량"
              << "  " << std::setw(kTimeW)  << "예상소요(분)" << "\n";
    std::cout << "  " << std::string(kSepLen, '-') << "\n";

    for (int i = 0; i < static_cast<int>(waitingJobs.size()); ++i) {
        const auto& j = waitingJobs[i];
        std::cout << "  " << std::setw(kNoW)    << (i + 1)
                  << "  " << std::setw(kOrderW) << j.orderId
                  << "  " << std::setw(kSampleW)<< j.sampleId
                  << "  " << std::setw(kProdW)  << j.actualProduction
                  << "  " << std::setw(kTimeW)  << j.totalTime << "\n";
    }
}

void ProductionView::printNoJobRunning() const {
    std::cout << "\n  현재 생산 중인 작업 없음\n";
}

void ProductionView::printBackPrompt() const {
    std::cout << "\n  [0] 뒤로\n  선택: ";
}

#include "view/SampleView.h"
#include <iostream>
#include <iomanip>

namespace {
    constexpr int kIdW    =  8;
    constexpr int kNameW  = 20;
    constexpr int kTimeW  = 12;
    constexpr int kYieldW =  8;
    constexpr int kSepLen = 56;
}

static void printSep() {
    std::cout << "  " << std::string(kSepLen, '-') << "\n";
}

// 한글은 터미널에서 2열 차지 → 우측 테두리 없이 좌측 테두리만 사용
void SampleView::printSubMenu() const {
    std::cout << "\n";
    std::cout << "  ┌──────────────────────────\n";
    std::cout << "  │  시료 관리 메뉴\n";
    std::cout << "  ├──────────────────────────\n";
    std::cout << "  │  [1] 시료 등록\n";
    std::cout << "  │  [2] 시료 목록 조회\n";
    std::cout << "  │  [3] 시료 검색\n";
    std::cout << "  │  [0] 뒤로\n";
    std::cout << "  └──────────────────────────\n";
    std::cout << "  선택 >> ";
}

void SampleView::printList(const std::vector<Sample>& samples) const {
    std::cout << "\n";
    if (samples.empty()) {
        std::cout << "  등록된 시료가 없습니다.\n";
        return;
    }
    printSep();
    std::cout << "  시료 목록 (" << samples.size() << "건)\n";
    printSep();
    // setw는 바이트 기준이라 한글 헤더에 맞지 않음 → 열 너비에 맞춰 직접 계산한 리터럴 사용
    // 각 열 display 너비: ID=8, 이름=20, 생산시간=12, 수율=8
    // 이름(4열)+16공백=20, 생산시간(8열)+4공백=12, 수율(4열)+4공백=8
    std::cout << "  ID      이름                생산시간    수율    재고\n";
    printSep();
    for (const auto& s : samples) {
        std::cout << "  " << std::left
                  << std::setw(kIdW)    << s.sampleId
                  << std::setw(kNameW)  << s.name
                  << std::setw(kTimeW)  << s.avgProdTime
                  << std::setw(kYieldW) << s.yield
                  << s.stock << "\n";
    }
    printSep();
}

void SampleView::printSample(const Sample& sample) const {
    std::cout << "\n";
    printSep();
    std::cout << "  시료 상세 정보\n";
    printSep();
    std::cout << "  ID        : " << sample.sampleId    << "\n";
    std::cout << "  이름      : " << sample.name        << "\n";
    std::cout << "  생산시간  : " << sample.avgProdTime << " min/ea\n";
    std::cout << "  수율      : " << sample.yield       << "\n";
    std::cout << "  재고      : " << sample.stock       << " ea\n";
    printSep();
}

void SampleView::printRegisterSuccess(const std::string& sampleId) const {
    std::cout << "\n  >> 시료 등록 완료 : " << sampleId << "\n";
}

void SampleView::printRegisterForm() const {
    std::cout << "\n";
    printSep();
    std::cout << "  신규 시료 등록\n";
    printSep();
}

void SampleView::printNamePrompt() const {
    std::cout << "  이름           : ";
}

void SampleView::printAvgProdTimePrompt() const {
    std::cout << "  생산시간(min)  : ";
}

void SampleView::printYieldPrompt() const {
    std::cout << "  수율(0.0~1.0)  : ";
}

void SampleView::printSearchPrompt() const {
    std::cout << "\n  검색어 >> ";
}

void SampleView::printNotFound() const {
    std::cout << "\n  검색 결과가 없습니다.\n";
}

void SampleView::printError(const std::string& message) const {
    std::cout << "\n  [오류] " << message << "\n";
}

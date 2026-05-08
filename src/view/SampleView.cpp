#include "view/SampleView.h"
#include <iostream>
#include <iomanip>

namespace {
    constexpr int kColWidth = 12;
}

void SampleView::printSubMenu() const {
    std::cout << "\n--- 시료 관리 ---\n";
    std::cout << " [1] 시료 등록\n";
    std::cout << " [2] 시료 목록 조회\n";
    std::cout << " [3] 시료 검색\n";
    std::cout << " [0] 뒤로\n";
    std::cout << "선택: ";
}

void SampleView::printList(const std::vector<Sample>& samples) const {
    if (samples.empty()) {
        std::cout << "등록된 시료가 없습니다.\n";
        return;
    }
    std::cout << "\n"
              << std::left
              << std::setw(kColWidth) << "ID"
              << std::setw(20)        << "이름"
              << std::setw(kColWidth) << "평균생산시간"
              << std::setw(kColWidth) << "수율"
              << std::setw(kColWidth) << "재고"
              << "\n";
    std::cout << std::string(64, '-') << "\n";
    for (const auto& s : samples) {
        std::cout << std::left
                  << std::setw(kColWidth) << s.sampleId
                  << std::setw(20)        << s.name
                  << std::setw(kColWidth) << s.avgProdTime
                  << std::setw(kColWidth) << s.yield
                  << std::setw(kColWidth) << s.stock
                  << "\n";
    }
}

void SampleView::printSample(const Sample& sample) const {
    std::cout << "\n[시료 상세]\n";
    std::cout << "  ID          : " << sample.sampleId    << "\n";
    std::cout << "  이름        : " << sample.name        << "\n";
    std::cout << "  평균생산시간: " << sample.avgProdTime << " min/ea\n";
    std::cout << "  수율        : " << sample.yield       << "\n";
    std::cout << "  재고        : " << sample.stock       << " ea\n";
}

void SampleView::printRegisterSuccess(const std::string& sampleId) const {
    std::cout << "시료 등록 완료: " << sampleId << "\n";
}

void SampleView::printRegisterForm() const {
    std::cout << "\n--- 시료 등록 ---\n";
}

void SampleView::printSearchPrompt() const {
    std::cout << "검색어를 입력하세요: ";
}

void SampleView::printNotFound() const {
    std::cout << "검색 결과가 없습니다.\n";
}

void SampleView::printError(const std::string& message) const {
    std::cout << "[오류] " << message << "\n";
}

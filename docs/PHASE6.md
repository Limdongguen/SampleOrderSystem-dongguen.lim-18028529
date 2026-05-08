# Phase 6 — 모니터링 & UI 완성

> 이전: [Phase 5](PHASE5.md) | 다음: [Phase 7](PHASE7.md)  
> **산출물:** 메인 메뉴 `[4]` 모니터링 완전 동작 + 전체 UI 일관성 완성

## 목표
시스템 현황을 한눈에 파악할 수 있는 모니터링을 완성하고,  
메인 메뉴 요약 정보 및 전체 UI 일관성을 마무리한다.

---

## 체크리스트

### MonitoringService
- [ ] `src/service/MonitoringService.h/.cpp`
  - `getOrderSummary()` — 상태별 건수 집계 (REJECTED 제외)
  - `getStockStatusList()` — 시료별 재고 상태 계산
    ```cpp
    int activeSum = sum(CONFIRMED 수량) + sum(PRODUCING 수량);
    if (stock == 0)             → 고갈
    else if (stock < activeSum) → 부족
    else                        → 여유
    ```

### MonitoringView & MonitoringController
- [ ] `src/view/MonitoringView.h/.cpp`
  - **주문량 확인:** 상태별 건수, PRODUCING 옆 "← 생산라인 대기" 표기
  - **재고량 확인:** 시료명, 재고(ea), 상태(여유/부족/고갈), 잔여율(%)
- [ ] `src/controller/MonitoringController.h/.cpp` — `[1]주문량 [2]재고량 [0]뒤로`
- [ ] `MainController`에서 `[4]` 선택 시 `MonitoringController::run()` 연결

### 메인 메뉴 현황 요약
- [ ] `MainView` 상단에 시스템 현황 출력
  - 현재 시각 (`<ctime>` 사용)
  - 등록 시료 수, 총 재고 합산
  - 전체 주문 건수, 생산라인 대기 수

### UI 일관성 & 예외 처리
- [ ] 모든 메뉴에서 범위 벗어난 입력 시 재요청 (`ConsoleHelper::readInt`)
- [ ] 빈 목록일 때 "데이터 없음" 안내 메시지
- [ ] 목록 5건 초과 시 페이지네이션 (`[N]다음 [P]이전`)

### 단위 테스트
- [ ] `tests/MonitoringServiceTest.cpp`
  - REJECTED 주문이 집계에서 제외됨
  - 재고 여유/부족/고갈 각 경계값 테스트

---

## 완료 기준
- 모니터링에서 상태별 주문 건수와 시료별 재고 상태(여유/부족/고갈) 확인
- 메인 메뉴에 등록 시료 수·총 재고·전체 주문 건수·생산 대기 수 표시
- 잘못된 입력 시 크래시 없이 재요청
- VS Debug x64 — MonitoringService 테스트 통과

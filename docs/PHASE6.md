# Phase 6 — 모니터링 & UI 완성

> 이전: [Phase 5](PHASE5.md) | 다음: [Phase 7](PHASE7.md)  
> **산출물:** 메인 메뉴 `[4]` 모니터링 완전 동작 + 전체 UI 일관성 완성

## 목표
시스템 현황을 한눈에 파악할 수 있는 모니터링을 완성한다.  
모니터링 화면은 **1분마다 자동 갱신**되며, 생산 진행 상황(`tickCheck`)을  
갱신 시마다 반영한다.

---

## 자동 갱신 구현 방식

### 비차단 입력 루프 (`<conio.h>` + `<chrono>`)

```cpp
// MonitoringController::runRealTime() 의사코드
#include <conio.h>
#include <chrono>
#include <thread>

void runRealTime() {
    using Clock = std::chrono::steady_clock;
    constexpr int kRefreshSec = 60;

    while (true) {
        m_productionService->tickCheck();   // 생산 자동 완료 체크
        renderAll();                        // 화면 갱신

        auto deadline = Clock::now() + std::chrono::seconds(kRefreshSec);
        while (Clock::now() < deadline) {
            if (_kbhit()) {
                char ch = static_cast<char>(_getch());
                if (ch == '0') return;          // 뒤로
                if (ch == 'r' || ch == 'R') break; // 즉시 갱신
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            // 카운트다운 갱신: 하단 1줄만 덮어쓰기
            int remaining = static_cast<int>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    deadline - Clock::now()).count());
            m_view.printCountdown(remaining);
        }
        m_view.clearCountdown();            // 카운트다운 줄 지우기 (공백 덮어쓰기)
    }
}
```

- **`_kbhit()`** — 키 입력 여부만 확인 (차단하지 않음)
- **`_getch()`** — Enter 없이 한 문자 즉시 읽기
- **`std::this_thread::sleep_for(500ms)`** — CPU 과부하 방지
- 화면 전체 `cls`는 사용하지 않는다. 카운트다운 줄만 `"\r"` + 공백으로 갱신한다.

### `MonitoringView::printCountdown` 구현 방식

```cpp
void MonitoringView::printCountdown(int seconds) const {
    // \r로 줄 시작으로 이동 후 덮어쓰기 → 줄 바꿈 없이 갱신
    std::cout << "\r  [ R: 즉시갱신 | 0: 뒤로 | 자동갱신까지 "
              << seconds << "초 ]   " << std::flush;
}
void MonitoringView::clearCountdown() const {
    std::cout << "\r" << std::string(60, ' ') << "\r" << std::flush;
}
```

---

## 체크리스트

### MonitoringService

- [x] `src/service/MonitoringService.h/.cpp`
  - `getOrderSummary()` — `OrderSummary` 구조체 반환 (REJECTED 제외)
    ```cpp
    struct OrderSummary {
        int reserved; int confirmed; int producing; int released; int total;
    };
    ```
  - `getStockStatusList()` — `StockStatus` 목록 반환
    ```cpp
    struct StockStatus {
        std::string sampleId; std::string name;
        int stock; int activeSum; std::string status; // "여유"|"부족"|"고갈"
        double remainRatio; // stock / (activeSum == 0 ? stock : activeSum) * 100
    };
    ```
  - `getOrderSummary()` 집계 기준:  
    `activeSum = sum(CONFIRMED 주문량) + sum(PRODUCING 주문량)`  
    `if (stock == 0)` → 고갈 / `else if (stock < activeSum)` → 부족 / `else` → 여유

### MonitoringView & MonitoringController

- [x] `src/view/MonitoringView.h/.cpp`
  - **주문량 확인:** 상태별 건수 표
    ```
    상태         건수
    RESERVED      N
    CONFIRMED     N
    PRODUCING     N  ← 생산라인 가동 중
    RELEASED      N
    합계          N
    ```
  - **재고량 확인:** 시료별 재고 상태 표
    ```
    시료ID   이름      재고   활성주문  상태   잔여율
    S-001   Alpha    100    80      여유   125%
    S-002   Beta       0    30      고갈     0%
    ```
  - `printCountdown(int seconds)` — `\r` 덮어쓰기 카운트다운
  - `printCountdown(-1)` 호출 시 `clearCountdown()` 동작 (줄 지우기)

- [x] `src/controller/MonitoringController.h/.cpp`
  - **멤버**: `MonitoringService`, `ProductionService` (두 서비스 모두 보유)
  - `run()` — 서브메뉴: `[1]주문량 [2]재고량 [0]뒤로`
  - `runOrderSummary()` — `runRealTime()` 패턴으로 1분 자동 갱신
  - `runStockStatus()` — `runRealTime()` 패턴으로 1분 자동 갱신
  - `MainController`에서 `[4]` 선택 시 `MonitoringController::run()` 연결

### 메인 메뉴 현황 요약

- [x] `MainController::run()` 루프 최상단에서 `m_productionService->tickCheck()` 호출
- [x] `MainView::printMenu()` 헤더에 현황 요약 출력 (뷰가 직접 계산하지 않음 — Controller가 데이터 전달)
  ```
  ╔══════════════════════════════════════
  ║  S-Semi 시료 생산주문관리 시스템
  ║  2026-05-08 14:35:22  |  시료 5종  |  총재고 1,200ea
  ║  전체주문 18건  |  생산대기 3건
  ╠══════════════════════════════════════
  ║  [1] 시료 관리
  ...
  ```
- [x] `MainView::printMenu(const MenuSummary& s)` 오버로드 추가
  ```cpp
  struct MenuSummary {
      std::string currentTime;
      int sampleCount; int totalStock;
      int orderCount;  int producingCount;
  };
  ```
- [x] `MainController`가 `MonitoringService`·`SampleService`·`OrderService`에서 요약 데이터 수집 후 `printMenu(summary)` 호출

### UI 일관성 & 예외 처리

- [x] 모든 메뉴에서 범위 벗어난 입력 시 재요청 (`ConsoleHelper::readInt` 이미 처리)
- [x] 빈 목록일 때 "데이터 없음" 안내 메시지 통일
- [x] 목록 5건 초과 시 페이지네이션 (`[N]다음 [P]이전 [0]뒤로`)
  - `ConsoleHelper::paginate(vector, pageSize, printFn)` 헬퍼 추가

### 단위 테스트

- [x] `tests/MonitoringServiceTest.cpp`
  - REJECTED 주문이 `getOrderSummary()` 집계에서 제외됨
  - 재고 여유/부족/고갈 경계값 (stock=activeSum → 여유, stock=activeSum-1 → 부족, stock=0 → 고갈)
  - `remainRatio` 계산 정확성

---

## 완료 기준

- 모니터링 화면 진입 시 60초 자동 갱신 루프 동작, `R`키로 즉시 갱신, `0`으로 종료
- 갱신마다 `tickCheck()` 호출 → 완료된 생산 작업 즉시 반영
- 메인 메뉴에 현재 시각·시료 수·총재고·주문 건수·생산대기 수 표시
- 잘못된 입력 시 크래시 없이 재요청
- VS Debug x64 — MonitoringService 테스트 통과

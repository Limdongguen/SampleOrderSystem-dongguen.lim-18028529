# Phase 5 — 생산라인 & 출고

> 이전: [Phase 4](PHASE4.md) | 다음: [Phase 6](PHASE6.md)  
> **산출물:** 메인 메뉴 `[5]` 생산라인 조회, `[6]` 출고 처리 완전 동작

## 목표
생산 큐(FIFO)를 **실시간 시계 기반**으로 관리한다.  
Job이 RUNNING이 되는 순간 `estimatedEndTime`을 벽시계(wall-clock)로 계산하고,  
`tickCheck()` 호출 시 현재 시각이 `estimatedEndTime`을 지났으면 자동 완료 처리한다.  
출고(RELEASE) 는 CONFIRMED 주문을 대상으로 수동 처리한다.

---

## 시간 표현 규칙

| 항목 | 형식 | 예시 |
|------|------|------|
| `estimatedEndTime` | `"YYYY-MM-DD HH:MM:SS"` (로컬 시각) | `"2026-05-08 14:35:00"` |
| 시각 연산 | `<ctime>` `std::time_t` / `std::mktime` / `std::localtime` | — |
| 시각 비교 | `std::time_t` 수치 비교 (`>=`) | — |

### 시각 유틸리티 — `src/util/TimeUtil.h/.cpp`

```cpp
class TimeUtil {
public:
    // 현재 로컬 시각을 "YYYY-MM-DD HH:MM:SS" 문자열로 반환
    static std::string nowString();

    // 기준 시각 문자열에 minutes 분을 더한 문자열 반환
    static std::string addMinutes(const std::string& base, double minutes);

    // estimatedEndTime 문자열이 현재 시각보다 과거이면 true
    static bool isPast(const std::string& estimatedEndTime);

    // estimatedEndTime까지 남은 초 반환 (음수면 0)
    static int secondsUntil(const std::string& estimatedEndTime);
};
```

- `TimeUtil`은 순수 유틸이므로 **테스트에서 직접 호출 가능**.
- 단위 테스트에서 시간 의존성을 제거하려면 `isPast()`에 현재 시각을 주입하는 overload를 추가한다.  
  `static bool isPast(const std::string& estimatedEndTime, std::time_t now);`

---

## 체크리스트

### TimeUtil 구현
- [x] `src/util/TimeUtil.h/.cpp` — `nowString`, `addMinutes`, `isPast`, `secondsUntil`
- [x] `tests/TimeUtilTest.cpp` — `addMinutes` 분 덧셈, `isPast` 경계값 테스트

### ProductionService — RUNNING 전환 & 자동 완료

- [x] **`startJob(ProductionJob&)`** (private)
  - `job.estimatedEndTime = TimeUtil::addMinutes(TimeUtil::nowString(), job.totalTime)`
  - `job.status = JobStatus::RUNNING`
  - `m_jobRepo->update(job)` + `m_jobRepo->save()`

- [x] **`enqueue()` 수정** — RUNNING 전환 로직 추가  
  큐가 비어 있을 때: `job.status = WAITING` 후 push → 즉시 `startJob(job)` 호출  
  큐가 비어 있지 않을 때: `job.status = WAITING` 후 push (대기)

- [x] **`tickCheck()`** (public)  
  ```
  1. findAll()에서 RUNNING Job 탐색
  2. 없으면 return
  3. TimeUtil::isPast(job.estimatedEndTime) 가 false면 return
  4. completeJob(job) 호출
  ```

- [x] **`completeJob(ProductionJob&)`** (private)
  1. `sample.stock += job.actualProduction`
  2. 연결 주문 상태: PRODUCING → CONFIRMED, `updatedAt` 갱신
  3. `job.status = DONE`
  4. `SampleRepository::update(sample)` + `save()`
  5. `OrderRepository::update(order)` + `save()`
  6. `ProductionJobRepository::update(job)` + `save()`
  7. 큐에서 next WAITING Job 꺼내 `startJob()` 호출

- [x] **`getCurrentJob()`** — RUNNING Job 반환 `std::optional<ProductionJob>`
- [x] **`getWaitingJobs()`** — WAITING Job 목록 반환 `std::vector<ProductionJob>`

> **의존성**: `ProductionService`는 `SampleRepository`·`OrderRepository`·`ProductionJobRepository`를 모두 보유한다.

### ProductionView & ProductionController

- [x] `src/view/ProductionView.h/.cpp`
  - **현재 처리 중:**  
    - 주문번호, 시료ID, 부족분, 실 생산량  
    - 완료 예정: `estimatedEndTime` 문자열  
    - 남은 시간: `TimeUtil::secondsUntil(estimatedEndTime)` → `"X분 Y초 후"` 또는 `"완료 처리 중..."`
  - **대기 큐 (FIFO):** 순서, 주문번호, 시료ID, 실 생산량, 예상 소요시간(분)
  - 생산라인 비어있을 때: `"현재 생산 중인 작업 없음"` 출력

- [x] `src/controller/ProductionController.h/.cpp`
  - `run()` 진입 시 **`m_productionService->tickCheck()` 먼저 호출**
  - 생산 현황 표시 후 `[0] 뒤로` 입력 대기  
    *(완료는 `tickCheck()`가 자동 처리 — 수동 완료 버튼 없음)*
  - `MainController`에서 `[5]` 선택 시 `ProductionController::run()` 연결

### ReleaseService

- [x] `src/service/ReleaseService.h/.cpp`
  - `getConfirmedOrders()` — CONFIRMED 주문 목록 반환
  - `release(orderId)` — 주문 검증(CONFIRMED 여부) → `stock -= quantity` → 상태 RELEASED → 저장

### ReleaseView & ReleaseController

- [x] `src/view/ReleaseView.h/.cpp`
  - CONFIRMED 목록 테이블 (번호, 주문번호, 고객명, 시료ID, 수량)
  - 출고 완료 메시지 (주문번호, 출고 수량)
- [x] `src/controller/ReleaseController.h/.cpp`
  - `run()` 진입 시 `m_productionService->tickCheck()` 호출
  - CONFIRMED 목록 → 번호 선택 → `ReleaseService::release()`
  - `MainController`에서 `[6]` 선택 시 `ReleaseController::run()` 연결

### 단위 테스트

- [x] `tests/TimeUtilTest.cpp`
  - `addMinutes("2026-05-08 14:00:00", 90.0)` → `"2026-05-08 15:30:00"`
  - `isPast(now, past_time)` → true / `isPast(now, future_time)` → false

- [x] `tests/ProductionServiceTest.cpp` (완료 처리 케이스 추가)
  - `tickCheck()` — `isPast` 반환값 오버로드를 이용해 시간 주입:  
    과거 `estimatedEndTime` 설정 후 `tickCheck()` 호출 → 재고 증가 + CONFIRMED 전환 확인
  - 큐에 Job 2개: 첫 번째 완료 후 두 번째 자동 RUNNING 전환 확인

- [x] `tests/ReleaseServiceTest.cpp`
  - CONFIRMED 주문 출고 → RELEASED, `stock -= quantity` 확인
  - CONFIRMED 아닌 주문 출고 시도 → `false` 반환

---

## vcxproj 등록 필수 파일

신규 `.cpp` 파일 추가 시 `SampleOrderSystem.vcxproj` `<ItemGroup>` 에 등록:
- `src/util/TimeUtil.cpp`
- `src/service/ReleaseService.cpp`
- `src/view/ProductionView.cpp`, `src/view/ReleaseView.cpp`
- `src/controller/ProductionController.cpp`, `src/controller/ReleaseController.cpp`
- `tests/TimeUtilTest.cpp`, `tests/ReleaseServiceTest.cpp` (Debug|x64 전용)

---

## 완료 기준

- RUNNING Job의 `estimatedEndTime`이 `"YYYY-MM-DD HH:MM:SS"` 형식으로 저장됨
- `tickCheck()` 호출 시 과거 `estimatedEndTime` → 재고 증가 + CONFIRMED 전환 + JSON 저장 확인
- 출고 처리 시 재고 차감 + RELEASED 전환 확인
- VS Debug x64 — TimeUtil, ProductionService(완료), Release 테스트 통과

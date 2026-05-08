# Phase 5 — 생산라인 & 출고

> 이전: [Phase 4](PHASE4.md) | 다음: [Phase 6](PHASE6.md)  
> **산출물:** 메인 메뉴 `[5]` 생산라인 조회, `[6]` 출고 처리 완전 동작

## 목표
생산 큐(FIFO)를 관리하고, 생산 완료 처리 및 출고를 구현한다.  
생산 완료 시 재고가 증가하고 주문이 PRODUCING → CONFIRMED으로 전환된다.

---

## 체크리스트

### ProductionService — completeCurrentJob
- [ ] `completeCurrentJob()` 구현
  1. 큐 front의 Job을 완료 처리
  2. `sample.stock += actualProduction`
  3. 연결된 주문 상태: PRODUCING → CONFIRMED
  4. Job 상태: RUNNING → DONE
  5. `SampleRepository::save()`, `OrderRepository::save()`, `ProductionJobRepository::save()`
  6. 큐에서 pop, 다음 Job이 있으면 RUNNING으로 전환
- [ ] `getCurrentJob()` — 현재 RUNNING Job 반환 (`std::optional`)
- [ ] `getWaitingQueue()` — WAITING Job 목록 반환

### ProductionView & ProductionController
- [ ] `src/view/ProductionView.h/.cpp`
  - **현재 처리 중:** 주문번호, 시료명, 주문량, 재고(승인 시점), 부족분, 실 생산량, 완료 예정
  - **대기 큐 (FIFO):** 순서, 주문번호, 시료명, 부족분, 실 생산량
  - 생산라인 비어있을 때 "현재 생산 중인 작업 없음" 출력
- [ ] `src/controller/ProductionController.h/.cpp`
  - 생산 현황 표시
  - `[C]완료 처리 [0]뒤로` 옵션
- [ ] `MainController`에서 `[5]` 선택 시 `ProductionController::run()` 연결

### ReleaseService
- [ ] `src/service/ReleaseService.h/.cpp`
  - `getConfirmedOrders()` — CONFIRMED 목록 반환
  - `release(orderId)` — 재고 차감(`stock -= quantity`), 상태 RELEASED → 저장

### ReleaseView & ReleaseController
- [ ] `src/view/ReleaseView.h/.cpp`
  - CONFIRMED 목록 테이블 (번호, 주문번호, 고객명, 시료명, 수량)
  - 출고 완료 메시지 (주문번호, 출고 수량, 처리 일시)
- [ ] `src/controller/ReleaseController.h/.cpp` — CONFIRMED 목록 → 번호 선택 → `ReleaseService::release()`
- [ ] `MainController`에서 `[6]` 선택 시 `ReleaseController::run()` 연결

### 단위 테스트
- [ ] `tests/ProductionServiceTest.cpp` (완료 처리 케이스 추가)
  - 완료 처리 후 재고 증가, 주문 CONFIRMED 전환, 큐 다음 Job RUNNING 전환
- [ ] `tests/ReleaseServiceTest.cpp`
  - CONFIRMED 주문 출고 → RELEASED, 재고 차감
  - CONFIRMED 아닌 주문 출고 시도 → 오류 반환

---

## 완료 기준
- 생산 완료 처리 시 재고 증가 + 주문 CONFIRMED 전환 + JSON 저장 확인
- 출고 처리 시 재고 차감 + 주문 RELEASED 전환 확인
- VS Debug x64 — Production, Release 테스트 통과

# Phase 4 — 주문 승인/거절

> 이전: [Phase 3](PHASE3.md) | 다음: [Phase 5](PHASE5.md)  
> **산출물:** 메인 메뉴 `[3]` 승인/거절 완전 동작, 재고 분기 처리

## 목표
RESERVED 주문을 승인 또는 거절한다.  
승인 시 재고 상황에 따라 CONFIRMED 또는 PRODUCING으로 자동 분기되며,  
재고 부족 시 ProductionJob이 생산 큐에 등록된다.

---

## 체크리스트

### ProductionJobRepository
- [ ] `src/repository/ProductionJobRepository.h/.cpp`
  - `JsonFileManager::load("data/production_jobs.json")` 로드
  - `add(ProductionJob)`, `findAll()`, `findByOrderId(orderId)`, `update(ProductionJob)`, `save()`

### ProductionService — enqueue
- [ ] `src/service/ProductionService.h/.cpp`
  - `std::queue<ProductionJob>` 멤버 (앱 시작 시 WAITING 상태 Job으로 초기화)
  - `enqueue(order, sample)` — 생산량 계산 후 큐에 추가 + 저장
    ```cpp
    int shortage         = order.quantity - sample.stock;
    int actualProduction = (int)std::ceil(shortage / (sample.yield * 0.9));
    double totalTime     = sample.avgProdTime * actualProduction;
    ```
  - `getQueue()` — 현재 대기 큐 snapshot 반환

### OrderService — approve / reject
- [ ] `approve(orderId)` 구현
  - 재고 충분(`stock >= quantity`): 재고 차감 → 상태 CONFIRMED → 저장
  - 재고 부족(`stock < quantity`): `ProductionService::enqueue()` → 상태 PRODUCING → 저장
- [ ] `reject(orderId)` — 상태 즉시 REJECTED → 저장

### ApprovalView & ApprovalController
- [ ] `src/view/ApprovalView.h/.cpp`
  - RESERVED 목록 테이블 출력
  - 재고 확인 결과 출력 (현재 재고, 부족분, 실 생산량, 예상 소요 시간)
  - 승인/거절 선택 프롬프트 `[Y]승인 [N]거절`
  - 처리 결과 (상태 전환 메시지)
- [ ] `src/controller/ApprovalController.h/.cpp` — RESERVED 목록 → 번호 선택 → 승인/거절
- [ ] `MainController`에서 `[3]` 선택 시 `ApprovalController::run()` 연결

### 단위 테스트
- [ ] `tests/OrderServiceTest.cpp` (승인/거절 케이스 추가)
  - 재고 충분 승인 → 상태 CONFIRMED, 재고 차감
  - 재고 부족 승인 → 상태 PRODUCING, ProductionJob 생성됨
  - 거절 → 상태 REJECTED
  - REJECTED 주문이 `getReservedOrders()` 에 미포함
- [ ] `tests/ProductionServiceTest.cpp`
  - `shortage=170, yield=0.92` → `actualProduction = ceil(170/(0.92*0.9)) = 206`
  - `shortage=0` 이하일 때 enqueue 호출 불가

---

## 완료 기준
- 재고 충분 주문 승인 → CONFIRMED, `samples.json` 재고 차감 확인
- 재고 부족 주문 승인 → PRODUCING, `production_jobs.json`에 작업 저장 확인
- 거절 → REJECTED, 이후 RESERVED 목록에서 미노출
- VS Debug x64 — approve/reject/enqueue 테스트 통과

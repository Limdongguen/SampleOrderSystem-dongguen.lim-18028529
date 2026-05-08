# Phase 3 — 주문 접수

> 이전: [Phase 2](PHASE2.md) | 다음: [Phase 4](PHASE4.md)  
> **산출물:** 메인 메뉴 `[2]` 주문 접수 완전 동작, `orders.json` RESERVED 저장

## 목표
고객 주문을 접수하여 RESERVED 상태로 저장한다.  
유효한 시료 ID만 주문 가능하며, 주문번호가 자동 생성된다.

---

## 체크리스트

### OrderRepository
- [ ] `src/repository/OrderRepository.h/.cpp`
  - 생성자에서 `JsonFileManager::load("data/orders.json")` 호출
  - `add(Order)`, `findAll()`, `findById(id)`, `findByStatus(status)`, `update(Order)`, `save()` 구현

### OrderService — reserve
- [ ] `src/service/OrderService.h/.cpp`
  - `reserve(sampleId, customerName, quantity)` 구현
    1. `SampleRepository`에서 시료 존재 확인 → 없으면 오류 반환
    2. `quantity > 0` 검증
    3. `IdGenerator::nextOrderId()` 로 주문번호 생성
    4. `OrderStatus::RESERVED`, `createdAt` 현재 시각 설정
    5. `OrderRepository::add()` + `save()` 호출
  - `getReservedOrders()` — RESERVED 목록 반환

### OrderView & OrderController
- [ ] `src/view/OrderView.h/.cpp`
  - 주문 입력 프롬프트 (시료 ID, 고객명, 수량)
  - 입력 확인 화면 (시료명 조회 후 표시)
  - 완료 메시지 (주문번호, 현재 상태 RESERVED)
- [ ] `src/controller/OrderController.h/.cpp` — 입력 수집 → `OrderService::reserve()` → View 출력
- [ ] `MainController`에서 `[2]` 선택 시 `OrderController::reserve()` 연결

### 단위 테스트
- [ ] `tests/OrderServiceTest.cpp`
  - 유효한 입력으로 주문 접수 → RESERVED 상태, 주문번호 형식 `ORD-YYYYMMDD-NNNN`
  - 존재하지 않는 시료 ID → 오류 반환
  - 수량 0 이하 → 오류 반환

---

## 완료 기준
- 주문 접수 후 `data/orders.json`에 RESERVED 상태로 저장
- 존재하지 않는 시료 ID 입력 시 오류 메시지 후 재입력 유도
- VS Debug x64 — OrderService 테스트 통과

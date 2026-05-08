# Phase 7 — 더미 데이터 & 최종 품질

> 이전: [Phase 6](PHASE6.md) | 다음: 완료  
> **산출물:** 전체 시나리오 시연 가능, 제출 품질 달성

## 목표
테스트용 더미 데이터를 생성하여 전체 시나리오를 검증하고,  
CleanCode 최종 리팩토링으로 제출 품질을 완성한다.

---

## 체크리스트

### 더미 데이터 생성 Tool

- [x] `SampleOrderSystem/DummyGen.cpp` 또는 별도 실행 파일로 구현
- [x] 시료 10종 생성 (실제 반도체 소재명 사용)
  ```
  실리콘 웨이퍼-8인치, GaN 에피택셜-4인치, SiC 파워기판-6인치,
  포토레지스트-PR7, 산화막 웨이퍼-SiO2, HfO2 게이트유전체,
  Cu 배선재료, Low-k 절연막, Si3N4 질화막, Al2O3 ALD막
  ```
  - **`avgProdTime`을 0.05~0.2 (분) 으로 설정** → 생산 완료가 수 분 내에 일어나도록 (시연용)
  - `yield`: 0.85 ~ 0.98 범위 랜덤

- [x] 주문 20건 생성 (상태 혼합: RESERVED 3, CONFIRMED 5, PRODUCING 3, RELEASED 8, REJECTED 1)
- [x] 생산 작업 3건 생성 (PRODUCING 주문과 1:1 매핑, 1개 RUNNING + 2개 WAITING)
  - **RUNNING Job의 `estimatedEndTime`**: 더미 생성 시각 기준 `+2분` 으로 설정  
    → 앱 실행 후 약 2분 내에 `tickCheck()`가 자동 완료 처리

- [x] 실행 시 `data/*.json` 생성 및 결과 요약 출력

### 전체 시나리오 통합 검증 (기존 A~E: RegressionTest.cpp에 구현됨)

- [x] **시나리오 A**: DummyGen → 시료 추가 → 주문 → 승인 → 모니터링 → 출고/생산 확인
- [x] **시나리오 B**: REJECTED 주문이 모니터링 집계에서 제외
- [x] **시나리오 C**: DummyGen 후 RESERVED 3건 조회
- [x] **시나리오 D**: 생산 RUNNING Job 및 estimatedEndTime 확인
- [x] **시나리오 E**: 크로스 서비스 — 한 인스턴스가 저장한 시료를 다른 인스턴스가 주문에 사용

### Regression 테스트 확장 (`tests/RegressionTest.cpp` 추가 시나리오)

#### F. 시료 입력 유효성 검증
- [x] `F1` — 빈 이름 시료 등록 실패 (`registerSample("", 0.1, 0.9)` → false)
- [x] `F2` — 수율 0.0 시료 등록 실패
- [x] `F3` — 수율 1.0 초과 시료 등록 실패 (`yield=1.1`)
- [x] `F4` — 생산시간 0 이하 시료 등록 실패 (`avgProdTime=0.0`)
- [x] `F5` — 중복 이름 시료 등록 실패 (동일 이름 두 번 등록)

#### G. 주문 입력 유효성 검증
- [x] `G1` — 존재하지 않는 sampleId로 주문 접수 실패
- [x] `G2` — 수량 0 주문 접수 실패
- [x] `G3` — 수량 음수 주문 접수 실패

#### H. 주문 상태 전이 전 경로 검증
- [x] `H1` — RESERVED → CONFIRMED (재고 충분: stock > quantity)
- [x] `H2` — RESERVED → PRODUCING (재고 부족: stock < quantity)
- [x] `H3` — RESERVED → REJECTED
- [x] `H4` — REJECTED 주문은 `getReservedOrders()`에서 제외됨
- [x] `H5` — CONFIRMED → RELEASED (출고 후 재고 차감 확인)

#### I. 생산 공식 정확성
- [x] `I1` — `shortage=100, yield=1.0` → `actualProduction = ceil(100/(1.0*0.9)) = 112`
- [x] `I2` — `shortage=170, yield=0.92` → `actualProduction = 206`
- [x] `I3` — `shortage=1, yield=0.85` → `actualProduction = ceil(1/(0.85*0.9)) = 2`
- [x] `I4` — totalTime = avgProdTime × actualProduction 정확성

#### J. tickCheck 자동 완료
- [x] `J1` — 과거 `estimatedEndTime` → `tickCheck()=true`, 재고 증가, PRODUCING→CONFIRMED
- [x] `J2` — 미래 `estimatedEndTime` → `tickCheck()=false`, 상태 변화 없음
- [x] `J3` — WAITING Job 2개: 첫 번째 완료 → 두 번째 자동 RUNNING 전환 확인

#### K. 모니터링 경계값
- [x] `K1` — `stock=0` → 고갈
- [x] `K2` — `stock < activeSum` → 부족
- [x] `K3` — `stock == activeSum` → 여유 (경계: stock이 정확히 activeSum과 같을 때)
- [x] `K4` — `stock > activeSum` → 여유 (RESERVED는 activeSum에 미포함 확인)

#### L. 출고 오류 케이스
- [x] `L1` — RESERVED 상태 주문 출고 시도 → false 반환
- [x] `L2` — REJECTED 상태 주문 출고 시도 → false 반환
- [x] `L3` — 출고 후 재고가 `stock -= quantity` 만큼 차감됨

#### M. ID 연속성
- [x] `M1` — DummyGen(10종) 후 `syncIdGenerator()` → 다음 시료 ID가 `S-011`
- [x] `M2` — `nextSampleId()` 연속 호출 시 S-011, S-012 순서 보장

---

### CleanCode 최종 리팩토링

- [x] 함수 길이 30줄 초과 함수 탐지 및 분리
- [x] 매직 넘버 전수 조사 → `constexpr` 상수화
- [x] `#pragma once` 누락 헤더 확인
- [x] `const`/`const&` 파라미터 전달 일관성 확인
- [x] 중복 코드 제거

### 최종 테스트 전체 통과

- [x] VS Debug x64 빌드 → `RUN_ALL_TESTS()` 0 failures (73개 통과)
- [x] Regression 확장 테스트 추가 후 전체 통과

---

## 완료 기준

- 더미 데이터 실행 후 전체 시나리오 수동 검증 완료
- VS Debug x64 전체 테스트 통과 (0 failures)
- 모든 소스 파일에 `#pragma once`, 매직 넘버 없음, 함수 30줄 이내

# Phase 7 — 더미 데이터 & 최종 품질

> 이전: [Phase 6](PHASE6.md) | 다음: 완료  
> **산출물:** 전체 시나리오 시연 가능, 제출 품질 달성

## 목표
테스트용 더미 데이터를 생성하여 전체 시나리오를 검증하고,  
CleanCode 최종 리팩토링으로 제출 품질을 완성한다.

---

## 체크리스트

### 더미 데이터 생성 Tool

- [ ] `SampleOrderSystem/DummyGen.cpp` 또는 별도 실행 파일로 구현
- [ ] 시료 10종 생성 (실제 반도체 소재명 사용)
  ```
  실리콘 웨이퍼-8인치, GaN 에피택셜-4인치, SiC 파워기판-6인치,
  포토레지스트-PR7, 산화막 웨이퍼-SiO2, HfO2 게이트유전체,
  Cu 배선재료, Low-k 절연막, Si3N4 질화막, Al2O3 ALD막
  ```
  - **`avgProdTime`을 0.05~0.2 (분) 으로 설정** → 생산 완료가 수 분 내에 일어나도록 (시연용)
  - `yield`: 0.85 ~ 0.98 범위 랜덤

- [ ] 주문 20건 생성 (상태 혼합: RESERVED 3, CONFIRMED 5, PRODUCING 3, RELEASED 8, REJECTED 1)
- [ ] 생산 작업 3건 생성 (PRODUCING 주문과 1:1 매핑, 1개 RUNNING + 2개 WAITING)
  - **RUNNING Job의 `estimatedEndTime`**: 더미 생성 시각 기준 `+2분` 으로 설정  
    → 앱 실행 후 약 2분 내에 `tickCheck()`가 자동 완료 처리

- [ ] 실행 시 `data/*.json` 생성 및 결과 요약 출력

### 전체 시나리오 통합 검증

- [ ] **시나리오 A**: 시료 등록 → 주문 접수 → 재고 충분 승인 → 출고
- [ ] **시나리오 B**: 주문 접수 → 재고 부족 승인 → 생산 큐 진입 → `tickCheck()` 자동 완료 → 출고
- [ ] **시나리오 C**: 주문 접수 → 거절 → 모니터링 주문량 확인에서 미집계 확인
- [ ] **시나리오 D**: 모니터링 재고량에서 고갈 시료 확인
- [ ] **시나리오 E**: 모니터링 화면 진입 → 60초 대기 → 자동 갱신 확인  
  (더미 RUNNING Job이 2분 후 완료 → 갱신 후 CONFIRMED 반영 확인)

### CleanCode 최종 리팩토링

- [ ] 함수 길이 30줄 초과 함수 탐지 및 분리
- [ ] 매직 넘버 전수 조사 → `constexpr` 상수화
  ```cpp
  constexpr double kYieldBuffer  = 0.9;
  constexpr int    kPageSize     = 5;
  constexpr int    kRefreshSec   = 60;    // 모니터링 자동 갱신 주기
  constexpr int    kPollMs       = 500;   // _kbhit() 폴링 간격
  ```
- [ ] `#pragma once` 누락 헤더 확인
- [ ] `const`/`const&` 파라미터 전달 일관성 확인
- [ ] 중복 코드 제거

### 최종 테스트 전체 통과

- [ ] VS Debug x64 빌드 → `RUN_ALL_TESTS()` 0 failures
- [ ] 테스트 커버리지 확인:
  - `TimeUtil::addMinutes` / `isPast` 경계값
  - `ProductionService::tickCheck()` 과거·미래 `estimatedEndTime`
  - `ProductionJob` 생산 공식 (`ceil(shortage / (yield * 0.9))`)
  - `OrderStatus` 전이 (모든 경로: CONFIRMED / PRODUCING / REJECTED)
  - 재고 상태 판별 경계값 (여유 / 부족 / 고갈)
  - `SampleService` 입력 검증 (빈 이름, yield 범위, avgProdTime 범위)
  - `IdGenerator` 형식 및 순번

---

## 완료 기준

- 더미 데이터 실행 후 전체 5개 시나리오 수동 검증 완료
- VS Debug x64 전체 테스트 통과 (0 failures)
- 모든 소스 파일에 `#pragma once`, 매직 넘버 없음, 함수 30줄 이내

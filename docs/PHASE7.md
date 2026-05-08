# Phase 7 — 더미 데이터 & 최종 품질

> 이전: [Phase 6](PHASE6.md) | 다음: 완료  
> **산출물:** 전체 시나리오 시연 가능, 제출 품질 달성

## 목표
테스트용 더미 데이터를 생성하여 전체 시나리오를 검증하고,  
CleanCode 최종 리팩토링으로 제출 품질을 완성한다.

---

## 체크리스트

### 더미 데이터 생성 Tool
- [ ] `SampleOrderSystem/DummyGen.cpp` 또는 별도 프로젝트로 구현
- [ ] 시료 10종 생성 (실제 반도체 소재명 사용)
  ```
  실리콘 웨이퍼-8인치, GaN 에피택셜-4인치, SiC 파워기판-6인치,
  포토레지스트-PR7, 산화막 웨이퍼-SiO2, HfO2 게이트유전체,
  Cu 배선재료, Low-k 절연막, Si3N4 질화막, Al2O3 ALD막
  ```
- [ ] 주문 20건 생성 (상태 혼합: RESERVED 3, CONFIRMED 5, PRODUCING 3, RELEASED 8, REJECTED 1)
- [ ] 생산 작업 3건 생성 (PRODUCING 주문과 1:1 매핑, WAITING/RUNNING 혼합)
- [ ] 실행 시 `data/*.json` 생성 및 결과 요약 출력

### 전체 시나리오 통합 검증
- [ ] 시나리오 A: 시료 등록 → 주문 접수 → 재고 충분 승인 → 출고
- [ ] 시나리오 B: 주문 접수 → 재고 부족 승인 → 생산 완료 → 출고
- [ ] 시나리오 C: 주문 접수 → 거절 → 모니터링에서 미노출 확인
- [ ] 시나리오 D: 모니터링에서 재고 고갈 시료 확인

### CleanCode 최종 리팩토링
- [ ] 함수 길이 30줄 초과 함수 탐지 및 분리
- [ ] 매직 넘버 전수 조사 → `constexpr` 상수화
  - `constexpr double kYieldBuffer = 0.9;`
  - `constexpr int kPageSize = 5;`
- [ ] `#pragma once` 누락 헤더 확인
- [ ] `const`/`const&` 파라미터 전달 일관성 확인
- [ ] 중복 코드 제거

### 최종 테스트 전체 통과
- [ ] VS Debug x64 빌드 → `RUN_ALL_TESTS()` 0 failures
- [ ] 테스트 커버리지 확인:
  - ProductionJob 생산 공식
  - OrderStatus 전이 (모든 경로)
  - 재고 상태 판별 (경계값)
  - SampleService 입력 검증
  - IdGenerator 형식 및 순번

---

## 완료 기준
- 더미 데이터 실행 후 전체 4개 시나리오 수동 검증 완료
- VS Debug x64 전체 테스트 통과 (0 failures)
- 모든 소스 파일에 `#pragma once`, 매직 넘버 없음, 함수 30줄 이내

# PRD — 반도체 시료 생산주문관리 시스템

> 제출자: dongguen.lim (사번: 18028529)

## 1. 배경

가상 반도체 회사 S-Semi에서 시료(Sample) 주문이 급증하여 엑셀/메모장 방식의 한계를 극복하기 위해 체계적인 콘솔 기반 시스템 개발이 필요하다.

---

## 2. 역할별 흐름

```
고객 ──시료 요청──▶ 주문 담당자 ──주문서 전달──▶ 생산 담당자
                        ▲                          │
                        └────── 승인 / 거절 ────────┘
```

| 역할 | 책임 |
|------|------|
| 고객 | 필요한 시료를 이메일로 요청 |
| 주문 담당자 | 요청에 맞게 주문서 작성 및 관리 |
| 생산 담당자 | 시료 등록, 주문 수신 후 승인 또는 거절 |

---

## 3. 주문 상태 흐름

```
RESERVED ──(승인, 재고 충분)──▶ CONFIRMED ──▶ RELEASED
RESERVED ──(승인, 재고 부족)──▶ PRODUCING ──▶ CONFIRMED ──▶ RELEASED
RESERVED ──(거절)────────────▶ REJECTED   (모든 조회에서 제외)
```

| 상태 | 의미 |
|------|------|
| RESERVED | 주문 접수 |
| REJECTED | 주문 거절 (모니터링 제외) |
| PRODUCING | 승인 완료, 재고 부족으로 생산 중 |
| CONFIRMED | 승인 완료, 출고 대기 중 |
| RELEASED | 출고 완료 |

---

## 4. 데이터 모델

### Sample
```cpp
struct Sample {
    std::string sampleId;    // S-NNN (3자리 zero-padding)
    std::string name;
    double      avgProdTime; // min/ea
    double      yield;       // 0.0 ~ 1.0
    int         stock;       // ea
};
```

### Order
```cpp
struct Order {
    std::string orderId;      // ORD-YYYYMMDD-NNNN
    std::string sampleId;
    std::string customerName;
    int         quantity;
    OrderStatus status;
    std::string createdAt;
    std::string updatedAt;
};
```

### ProductionJob
```cpp
struct ProductionJob {
    std::string jobId;
    std::string orderId;
    std::string sampleId;
    int         shortage;          // 주문 수량 - 재고
    int         actualProduction;  // ceil(shortage / (yield * 0.9))
    double      totalTime;         // avgProdTime * actualProduction
    JobStatus   status;            // WAITING | RUNNING | DONE
    std::string estimatedEndTime;
};
```

---

## 5. 기능 명세

### 메인 메뉴
- 시스템 현재 시각, 등록 시료 수, 총 재고, 전체 주문 건수, 생산라인 대기 수 표시
- `[1]` 시료 관리 / `[2]` 시료 주문 / `[3]` 주문 승인·거절 / `[4]` 모니터링 / `[5]` 생산라인 조회 / `[6]` 출고 처리 / `[0]` 종료

### 시료 관리
| 서브메뉴 | 기능 |
|----------|------|
| 시료 등록 | sampleId(자동), 이름, avgProdTime, yield 입력 후 저장 |
| 시료 조회 | 전체 목록 + 현재 재고 표시 |
| 시료 검색 | 이름 또는 ID로 검색 |

### 시료 주문
- 입력: 시료 ID, 고객명, 주문 수량
- 주문번호 자동 생성(`ORD-YYYYMMDD-NNNN`), 상태 RESERVED로 저장

### 주문 승인/거절
- RESERVED 목록 표시 → 번호 선택
- 승인: 재고 충분 → CONFIRMED / 재고 부족 → PRODUCING + 생산 큐 등록
- 거절: 즉시 REJECTED

### 모니터링
- 상태별 주문 건수 (REJECTED 제외)
- 시료별 재고 및 재고 상태 (여유 / 부족 / 고갈)

### 생산라인 조회
- 현재 처리 중인 작업 (주문번호, 시료, 주문량, 부족분, 실 생산량, 진행률, 완료 예정)
- 대기 큐 목록 (FIFO 순서)
- 생산 완료 시 재고 반영 → 주문 상태 PRODUCING → CONFIRMED

### 출고 처리
- CONFIRMED 목록 표시 → 번호 선택 → 재고 차감 → 상태 RELEASED

---

## 6. 핵심 비즈니스 규칙

1. 등록된 시료만 주문 가능
2. 생산 큐 전략: FIFO (`std::queue<ProductionJob>`)
3. 실 생산량: `ceil(shortage / (yield * 0.9))`
4. REJECTED 주문은 모니터링 및 모든 목록 조회에서 제외
5. 출고는 CONFIRMED 상태만 가능
6. 모든 상태 변경 직후 JSON 파일에 즉시 저장

---

## 7. 기술 스택

| 항목 | 선택 |
|------|------|
| 언어 | C++20 |
| 빌드 | Visual Studio 2026 (MSBuild) |
| JSON | nlohmann/json (`third_party/nlohmann/json.hpp`) |
| 테스트 | Google Mock/Test (NuGet `gmock.1.11.0`) |
| 콘솔 I/O | std::cin / std::cout |
| IDE | Visual Studio 2026 |

---

## 8. 제출 Repository

| 항목 | Repository |
|------|-----------|
| MVC 스켈레톤 | `ConsoleMVC-dongguen.lim-18028529` |
| 데이터 영속성 | `DataPersistence-dongguen.lim-18028529` |
| 데이터 모니터링 Tool | `DataMonitor-dongguen.lim-18028529` |
| Dummy 데이터 생성 Tool | `DummyDataGenerator-dongguen.lim-18028529` |
| 반도체 시료 생산주문관리 시스템 | `SampleOrderSystem-dongguen.lim-18028529` |

> 모든 Repository는 **Public** 으로 생성

---

## 9. 개발 프로세스 규칙

### 규칙 1 — TDD: Red → Green → Refactor

| 단계 | 행동 | 커밋 메시지 형식 |
|------|------|----------------|
| 🔴 RED | 실패하는 테스트를 먼저 작성 | `red: [테스트명] - [검증 내용]` |
| 🟢 GREEN | 테스트를 통과하는 최소 구현 | `green: [구현 내용]` |
| 🔵 REFACTOR | 동작 유지하며 코드 품질 개선 | `refactor: [개선 내용]` |

### 규칙 2 — 4-SubAgent 파이프라인 필수 사용

```
/pipeline
  ├── SubAgent1: doc-verify        → 문서 정합성 PASS 확인
  ├── SubAgent2: ai-action         → TDD 사이클 준수 + 단계별 커밋
  ├── SubAgent3: test-verify       → 전체 테스트 통과 확인    (병렬)
  └── SubAgent4: compliance-verify → 아키텍처·규칙 준수 확인 (병렬)
```

## 10. 개발 Phase 로드맵

| Phase | 목표 | 산출물 |
|-------|------|--------|
| 1 | 프로젝트 기반 구성 | 모델 + 유틸 + 메뉴 골격 |
| 2 | 시료 관리 | 등록·조회·검색 |
| 3 | 주문 접수 | RESERVED 저장 |
| 4 | 주문 승인/거절 | CONFIRMED/PRODUCING/REJECTED 분기 |
| 5 | 생산라인 & 출고 | 생산 완료 처리 + RELEASED |
| 6 | 모니터링 & UI 완성 | 재고 상태·주문 현황 + UI 완성 |
| 7 | 더미 데이터 & 최종 품질 | 전체 시나리오 검증 + CleanCode |

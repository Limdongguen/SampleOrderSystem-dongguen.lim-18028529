# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 프로젝트 개요

**S-Semi 반도체 시료 생산주문관리 시스템** — 가상 반도체 회사 S-Semi의 시료(Sample) 주문·생산·출고를 관리하는 콘솔 기반 C++ 애플리케이션.

- 언어: C++17
- 빌드: Visual Studio 2026 (MSBuild) — 주 빌드 시스템
- JSON 영속성: nlohmann/json (`third_party/nlohmann/json.hpp`)
- 테스트: Google Mock/Test (NuGet `gmock.1.11.0`)
- 솔루션: `SampleOrderSystem.slnx` / 프로젝트: `SampleOrderSystem/SampleOrderSystem.vcxproj`

---

## 빌드 및 실행 명령

```powershell
# VS 개발자 환경 포함 MSBuild (Debug x64 - 테스트 실행)
$vs = "C:\Program Files\Microsoft Visual Studio\18\Community"
cmd /c "`"$vs\VC\Auxiliary\Build\vcvars64.bat`" && `"$vs\MSBuild\Current\Bin\MSBuild.exe`" SampleOrderSystem\SampleOrderSystem.vcxproj /p:Configuration=Debug /p:Platform=x64"

# Release 빌드 (콘솔 앱 실행)
cmd /c "`"$vs\VC\Auxiliary\Build\vcvars64.bat`" && `"$vs\MSBuild\Current\Bin\MSBuild.exe`" SampleOrderSystem\SampleOrderSystem.vcxproj /p:Configuration=Release /p:Platform=x64"
```

### Debug vs Release 동작
| 빌드 | 동작 |
|------|------|
| **Debug** | `InitGoogleMock()` + `RUN_ALL_TESTS()` — 테스트 실행 |
| **Release** | `MainController::run()` — 콘솔 앱 실행 |

---

## 프로젝트 구조

```
SampleOrderSystem/          ← 솔루션 루트
├── SampleOrderSystem.slnx  ← VS 솔루션
├── SampleOrderSystem/      ← VS 프로젝트 디렉토리
│   ├── SampleOrderSystem.vcxproj
│   └── main.cpp
├── src/                    ← 소스 코드
│   ├── controller/
│   ├── model/
│   ├── repository/
│   ├── service/
│   ├── util/
│   └── view/
├── tests/                  ← 테스트 파일 (gmock, Debug 전용)
├── data/                   ← JSON 데이터 파일 (runtime)
├── third_party/
│   └── nlohmann/json.hpp
└── packages/
    └── gmock.1.11.0/       ← NuGet gmock (이미 설치됨)
```

---

## 아키텍처

MVC 패턴으로 계층을 분리. 데이터는 `data/` 디렉토리의 JSON 파일에 영속 저장된다.

```
main.cpp (Debug: 테스트 | Release: 앱)
  └─▶ MainController        # 메뉴 루프, 화면 전환
        ├─▶ SampleController
        ├─▶ OrderController
        ├─▶ ApprovalController
        ├─▶ MonitoringController
        ├─▶ ProductionController
        └─▶ ReleaseController

각 Controller
  ├─▶ *View      (콘솔 출력 — std::cout만 사용)
  └─▶ *Service   (비즈니스 로직)
        └─▶ *Repository  (JSON 파일 R/W — JsonFileManager 경유)
```

**계층 간 의존 방향:** Controller → Service → Repository → JsonFileManager  
**View는 Service를 직접 호출하지 않는다.** Controller가 중간에서 조율한다.

---

## 개발 프로세스 규칙

> **이 규칙은 모든 개발 작업에 예외 없이 적용된다.**

### 규칙 1 — TDD 사이클 (Red → Green → Refactor)

모든 기능 구현은 반드시 아래 3단계 순서를 따르며, **각 단계마다 즉시 커밋**한다.

```
① RED      : 실패하는 테스트를 먼저 작성한다
              → commit: "red: [테스트명] - [검증 내용]"

② GREEN    : 테스트를 통과하는 최소한의 코드를 작성한다
              → commit: "green: [구현 내용]"

③ REFACTOR : 동작을 바꾸지 않고 코드 품질을 개선한다 (절대 생략 금지)
              → commit: "refactor: [개선 내용]"
```

**Refactor 단계 체크리스트**
- 함수 길이 30줄 이내 확인
- 매직 넘버 → named constant 교체
- 중복 코드 제거
- `const`/`const&` 전달 일관성 확인
- include guard(`#pragma once`) 누락 여부 확인

### 규칙 2 — 4-SubAgent 파이프라인 필수 사용

각 개발 작업은 반드시 `/pipeline` 명령을 통해 4개의 SubAgent 안에서 수행한다.

```
SubAgent1 (doc-verify)      : 문서 정합성 검증 → PASS 확인 후 진행
SubAgent2 (ai-action)       : TDD 사이클 준수하며 구현 + 단계별 커밋
SubAgent3 (test-verify)     : 전체 테스트 통과 확인        ─┐ 병렬
SubAgent4 (compliance-verify): 아키텍처·비즈니스 규칙 준수 확인 ─┘
```

임의로 `/ai-action`만 단독 실행하거나 SubAgent를 건너뛰는 것은 금지된다.
단독 실행은 디버깅 목적으로만 허용된다.

---

## 핵심 도메인 규칙

### 주문 상태 전이
```
RESERVED ──(승인, 재고 충분)──▶ CONFIRMED ──▶ RELEASED
RESERVED ──(승인, 재고 부족)──▶ PRODUCING ──▶ CONFIRMED ──▶ RELEASED
RESERVED ──(거절)────────────▶ REJECTED   (이후 모든 조회에서 제외)
```

### 생산 수량 공식
```cpp
int shortage         = order.quantity - sample.stock;  // 주문 수량 - 현재 재고
int actualProduction = (int)std::ceil(shortage / (yield * 0.9));
double totalTime     = avgProdTime * actualProduction;
```
- `shortage` = 주문 수량 − 현재 재고
- 수율(yield) 오차 10% 버퍼를 포함하여 실 생산량을 산정

### 재고 상태 판별 (모니터링)
- **여유**: `stock >= 활성 주문(CONFIRMED+PRODUCING) 합산 수량`
- **부족**: `0 < stock < 합산 수량`
- **고갈**: `stock == 0`

### 생산 큐 전략
- FIFO (`std::queue<ProductionJob>`)
- 생산 완료 시 재고에 실 생산량을 반영하고 주문 상태를 PRODUCING → CONFIRMED으로 전환

---

## 데이터 영속성

`JsonFileManager`가 `data/*.json` 파일을 읽고 쓴다. 모든 상태 변경 직후 즉시 저장한다.

```
data/
├── samples.json
├── orders.json
└── production_jobs.json
```

애플리케이션 시작 시 각 Repository가 해당 JSON 파일을 로드한다. 파일이 없으면 빈 컬렉션으로 초기화한다.

---

## 주문번호 / ID 생성 규칙

- 주문번호: `ORD-YYYYMMDD-NNNN` (당일 순번, 4자리 zero-padding)
- 시료 ID: `S-NNN` (3자리 zero-padding)
- `IdGenerator` 유틸리티에서 일괄 관리

---

## VS 프로젝트 설정 (vcxproj)

테스트 파일 추가 시 vcxproj에:
- `Debug|x64` 전용으로 `<ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'!='Debug|x64'">true</ExcludedFromBuild>` 설정
- `AdditionalIncludeDirectories`: `$(ProjectDir)..\src;$(ProjectDir)..\third_party;%(AdditionalIncludeDirectories)`
- `PrecompiledHeader`: `NotUsing`
- `LocalDebuggerWorkingDirectory`: `$(ProjectDir)..\` (data/ 경로 해결)

## gmock 테스트 작성 주의사항 (MSVC)

`std::unique_ptr<T>::operator->()` 를 standalone 표현식으로 사용하면 MSVC 파서가 오류를 냅니다.

```cpp
// ❌ MSVC 파싱 오류
service->method();

// ✅ 올바른 방법
bool r = service->method();           // 할당 사용
T& ref = *service; ref.method();      // 레퍼런스 경유
EXPECT_TRUE(service->method());       // 매크로 인자 내부
```

---

## 문서 구조

```
docs/
├── PRD.md      # 제품 요구사항 (기능 명세, 데이터 모델, 제출 정보)
├── PHASE1.md   # Phase 1: 프로젝트 기반 구성
├── PHASE2.md   # Phase 2: 시료 관리
├── PHASE3.md   # Phase 3: 주문 접수
├── PHASE4.md   # Phase 4: 주문 승인/거절
├── PHASE5.md   # Phase 5: 생산라인 & 출고
├── PHASE6.md   # Phase 6: 모니터링 & UI 완성
└── PHASE7.md   # Phase 7: 더미 데이터 & 최종 품질
```

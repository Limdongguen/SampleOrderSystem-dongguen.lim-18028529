# Phase 1 — 프로젝트 기반 구성

> 이전: 없음 | 다음: [Phase 2](PHASE2.md)  
> **산출물:** VS Debug 빌드 성공 + 메인 메뉴 골격 실행

## 목표
이후 모든 Phase의 기반이 되는 도메인 모델, 유틸리티, 콘솔 입출력 구조를 완성한다.  
이 Phase만으로도 VS Debug 빌드(테스트 실행) + Release 빌드(앱 실행)가 가능한 상태를 만든다.

---

## 체크리스트

### vcxproj 설정
- [x] `AdditionalIncludeDirectories`: `$(ProjectDir)..\src;$(ProjectDir)..\third_party`
- [x] `PrecompiledHeader`: `NotUsing` (4개 구성 모두)
- [x] `LocalDebuggerWorkingDirectory`: `$(ProjectDir)..\` (data/ 경로 해결)
- [x] 테스트 파일 추가 섹션 준비 (Debug|x64 전용 `<ExcludedFromBuild>`)

### 도메인 모델 & Enum
- [x] `src/model/Enums.h` — `OrderStatus`, `JobStatus` enum class + to/from string 헬퍼
- [x] `src/model/Sample.h` — 구조체 + `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE`
- [x] `src/model/Order.h` — 구조체 + JSON 직렬화
- [x] `src/model/ProductionJob.h` — 구조체 + JSON 직렬화

### 유틸리티
- [x] `src/util/JsonFileManager.h/.cpp` — `load(path)`, `save(path, json)`; 파일 없으면 빈 배열 반환
- [x] `src/util/IdGenerator.h/.cpp` — `nextOrderId()` → `ORD-YYYYMMDD-NNNN`, `nextSampleId()` → `S-NNN`, `reset()`
- [x] `src/util/ConsoleHelper.h/.cpp` — `readInt(min, max)`, `readLine()`, `readDouble()`

### 진입점 & MainController 골격
- [x] `SampleOrderSystem/main.cpp` 수정 — Release 분기에 `MainController::run()` 호출
- [x] `src/controller/MainController.h/.cpp` — 메뉴 루프 (`[1]~[6] [0]종료`), 각 항목 "미구현" 출력

### 단위 테스트 (gmock, Debug|x64 전용)
- [x] `tests/JsonFileManagerTest.cpp` — 저장/로드 왕복, 파일 없을 때 빈 배열
- [x] `tests/IdGeneratorTest.cpp` — 형식, 순번 증가, 날짜 포함
- [x] vcxproj에 테스트 파일 2개 추가 (Debug|x64 전용)

---

## gmock 테스트 작성 규칙 (MSVC 주의사항)

`std::unique_ptr<T>::operator->()` 를 standalone 표현식으로 사용하면 MSVC 파서 오류 발생.

```cpp
// ❌ standalone unique_ptr-> 금지
service->method();

// ✅ 올바른 방법
bool r = service->method();           // 할당
T& ref = *service; ref.method();      // 레퍼런스 경유
EXPECT_TRUE(service->method());       // 매크로 인자 내부
```

---

## 완료 기준
- VS Debug x64 빌드 성공 → `RUN_ALL_TESTS()` 실행 (JsonFileManager, IdGenerator 테스트 통과)
- VS Release x64 빌드 성공 → 메인 메뉴 출력, `0` 입력으로 종료

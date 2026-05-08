# Phase 2 — 시료 관리

> 이전: [Phase 1](PHASE1.md) | 다음: [Phase 3](PHASE3.md)  
> **산출물:** 메인 메뉴 `[1]` 시료 등록·조회·검색 완전 동작

## 목표
시스템의 가장 기본 단위인 시료(Sample)를 등록·조회·검색하는 기능을 완성한다.  
이 Phase 완료 후 `data/samples.json`이 영속적으로 유지된다.

---

## 체크리스트

### SampleRepository
- [ ] `src/repository/SampleRepository.h/.cpp`
  - 생성자에서 `JsonFileManager::load("data/samples.json")` 호출
  - `add(Sample)`, `findAll()`, `findById(id)`, `findByName(keyword)`, `save()` 구현
  - `save()` 는 `JsonFileManager::save()` 호출

### SampleService
- [ ] `src/service/SampleService.h/.cpp`
  - `registerSample(name, avgProdTime, yield)` — ID 자동 생성, 중복 이름 검증
  - `getAllSamples()` — 전체 목록 반환
  - `searchByName(keyword)` — 부분 일치 검색
  - `getSampleById(id)` — 없으면 `std::nullopt` 반환

### SampleView & SampleController
- [ ] `src/view/SampleView.h/.cpp` — `printList()`, `printSample()`, `printRegisterSuccess()`, 오류 메시지 출력
- [ ] `src/controller/SampleController.h/.cpp` — 서브메뉴 `[1]등록 [2]목록 [3]검색 [0]뒤로`
- [ ] `MainController`에서 `[1]` 선택 시 `SampleController::run()` 연결

### 입력 검증 규칙
- 수율: `0.0 < yield <= 1.0`
- 평균 생산시간: `avgProdTime > 0`
- 이름: 빈 문자열 불허

### 단위 테스트 (gmock, Debug|x64 전용)
- [ ] `tests/SampleRepositoryTest.cpp`
  - add 후 findAll에 포함, findById 성공/실패, findByName 부분일치, save 후 재로드
- [ ] `tests/SampleServiceTest.cpp`
  - 정상 등록, 중복 이름 거절, yield 범위 위반, avgProdTime 0 이하, 부분일치 검색
- [ ] vcxproj에 테스트 파일 2개 추가

---

## 완료 기준
- 시료 등록 후 `data/samples.json` 저장 확인
- 앱 재시작 후 등록한 시료가 목록에 유지됨
- VS Debug x64 — SampleRepository, SampleService 테스트 통과

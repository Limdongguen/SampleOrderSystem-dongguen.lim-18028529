# SubAgent3 — Test Verify (테스트 검증)

당신은 테스트 검증 전문 에이전트입니다.
`C:\reviewer\SampleOrderSystem` 프로젝트를 VS MSBuild로 빌드하고 테스트를 실행합니다.

## 빌드 환경

```powershell
$vs = "C:\Program Files\Microsoft Visual Studio\18\Community"
$vcvars = "$vs\VC\Auxiliary\Build\vcvars64.bat"
$msbuild = "$vs\MSBuild\Current\Bin\MSBuild.exe"
$proj = "C:\reviewer\SampleOrderSystem\SampleOrderSystem\SampleOrderSystem.vcxproj"
```

## 실행 순서

### Step 1: Debug x64 빌드
```powershell
cmd /c "`"$vcvars`" && `"$msbuild`" `"$proj`" /p:Configuration=Debug /p:Platform=x64 /t:Rebuild /v:minimal"
```
빌드 실패 시 오류 메시지 캡처 후 중단.

### Step 2: 테스트 실행
빌드된 Debug exe 직접 실행:
```
C:\reviewer\SampleOrderSystem\SampleOrderSystem\x64\Debug\SampleOrderSystem.exe
```
(Debug 빌드는 `RUN_ALL_TESTS()` 실행)

### Step 3: 테스트 커버리지 확인
`docs/PHASE*.md`의 테스트 체크리스트와 실제 `tests/` 디렉토리 비교.

### Step 4: 핵심 케이스 확인
| 케이스 | 파일 | 검증 |
|--------|------|------|
| 생산 공식 | ProductionServiceTest | shortage=170, yield=0.92 → 206 |
| 재고 충분 승인 | OrderServiceTest | RESERVED → CONFIRMED |
| 재고 부족 승인 | OrderServiceTest | RESERVED → PRODUCING |
| 거절 | OrderServiceTest | RESERVED → REJECTED |
| 재고 경계값 | MonitoringServiceTest | 여유/부족/고갈 |
| ID 형식 | IdGeneratorTest | ORD-YYYYMMDD-NNNN, S-NNN |

## 보고 형식

```
## Test Verify 결과

### 빌드: SUCCESS / FAIL
### 테스트: N개 실행 / N개 통과 / N개 실패
### 실패 목록: (있을 경우)
### 최종 판정: PASS / FAIL
```

FAIL이 있으면 원인을 분석하고 수정 가능하면 직접 고친 뒤 재빌드 후 PASS 확인.

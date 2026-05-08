# Pipeline — 4-SubAgent 오케스트레이터

SampleOrderSystem 프로젝트의 4단계 SubAgent 파이프라인을 실행합니다.

> **모든 개발 작업은 이 파이프라인을 통해서만 수행한다.**  
> SubAgent를 건너뛰거나 단독 실행하는 것은 디버깅 목적 외 금지된다.

```
SubAgent1 (doc-verify)          ← 문서 정합성 PASS 확인
       │
       ▼ PASS 시에만
SubAgent2 (ai-action)           ← TDD: Red→Green→Refactor + 단계별 커밋
       │  (각 항목마다 3커밋: red / green / refactor)
       ▼ 완료 후 병렬
SubAgent3 (test-verify)    ──┐  ← VS Debug x64 빌드 + 테스트 통과 확인
SubAgent4 (compliance-verify)──┘  ← 아키텍처·규칙 준수 확인
       │
       ▼
최종 보고
```

### TDD 커밋 흐름 (SubAgent2 내부)
```
체크리스트 항목 1개 시작
  ├── 🔴 red: [테스트명] - [검증 내용]       → git commit
  ├── 🟢 green: [구현 내용]                  → git commit
  └── 🔵 refactor: [개선 내용]               → git commit  ← 절대 생략 금지
체크리스트 항목 1개 완료 → 다음 항목으로
```

---

## 실행 지침

### Step 1: SubAgent1 — 문서 정합성 검증

**Agent 설명:** 문서 정합성 검증  
**Agent 프롬프트:**
`C:\reviewer\SampleOrderSystem\.claude\commands\doc-verify.md` 파일의 지침을 따라 문서 정합성 검증을 수행하세요.
`CLAUDE.md`, `docs/PRD.md`, `docs/PHASE1.md`~`docs/PHASE7.md` 파일을 읽고 모든 검증 항목을 점검합니다.
FAIL 항목이 있으면 직접 수정하고 최종 PASS 판정을 받으세요.

**SubAgent1 결과에 따른 분기:**
- **PASS** → Step 2로 진행
- **FAIL (수정 불가)** → 파이프라인 중단 후 사용자에게 보고

---

### Step 2: SubAgent2 — AI Action

SubAgent1 PASS 확인 후 SubAgent2를 실행하세요.

**Agent 설명:** AI 코드 구현  
**Agent 프롬프트:**
`C:\reviewer\SampleOrderSystem\.claude\commands\ai-action.md` 파일의 지침을 따라 코드를 구현하세요.
`docs/PHASE1.md`~`docs/PHASE7.md`에서 미완료(`- [ ]`) 항목이 있는 가장 낮은 Phase를 선택하고,
`CLAUDE.md`와 `docs/PRD.md`를 기준으로 구현합니다.
구현 완료 항목은 `- [x]`로 체크리스트를 업데이트하세요.

**SubAgent2 완료 후** → Step 3으로 진행

---

### Step 3: SubAgent3 + SubAgent4 병렬 실행

SubAgent3과 SubAgent4를 **동시에** 실행하세요.

**SubAgent3 — Test Verify**
- 설명: VS 빌드 및 테스트 검증
- 프롬프트: `C:\reviewer\SampleOrderSystem\.claude\commands\test-verify.md` 파일의 지침을 따라 VS Debug x64 빌드 후 테스트를 실행하세요. FAIL이 있으면 직접 수정 후 재실행하여 PASS를 확인하세요.

**SubAgent4 — Compliance Verify**
- 설명: 아키텍처 및 비즈니스 규칙 준수 검증
- 프롬프트: `C:\reviewer\SampleOrderSystem\.claude\commands\compliance-verify.md` 파일의 지침을 따라 `src/` 코드의 규정 준수 여부를 검증하세요. FAIL 항목은 직접 수정하고 PASS를 확인하세요.

---

### Step 4: 최종 파이프라인 보고

```
╔══════════════════════════════════════════════╗
║     SampleOrderSystem Pipeline 실행 결과     ║
╚══════════════════════════════════════════════╝

SubAgent1 (문서 정합성)   : ✅ PASS / ❌ FAIL
SubAgent2 (AI Action)     : ✅ 완료 / ⚠️  일부 미구현
SubAgent3 (Test Verify)   : ✅ PASS / ❌ FAIL  [N/N 통과]
SubAgent4 (Compliance)    : ✅ PASS / ❌ FAIL  [N건 위반]

──────────────────────────────────────────────
SubAgent2 TDD 커밋:
  red: / green: / refactor: 목록

SubAgent3 실패 테스트: (있을 경우)
SubAgent4 위반 항목: (있을 경우)

──────────────────────────────────────────────
전체 결과: ✅ ALL PASS / ❌ 조치 필요
다음 실행 권장 Phase: Phase N
```

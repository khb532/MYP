# Claude Code 명령어 가이드

Claude Code는 Anthropic의 공식 CLI 도구로, 터미널에서 직접 Claude AI와 상호작용하며 소프트웨어 개발 작업을 수행할 수 있게 해줍니다.

## 목차
- [기본 CLI 명령어](#기본-cli-명령어)
- [슬래시 커맨드 (Slash Commands)](#슬래시-커맨드-slash-commands)
- [도구 (Tools)](#도구-tools)
- [커스텀 명령어](#커스텀-명령어)
- [Git 관련 작업](#git-관련-작업)

---

## 기본 CLI 명령어

### `/help`
Claude Code 사용에 대한 도움말을 표시합니다.
```
/help
```

### `/clear`
현재 대화 내용을 지웁니다.
```
/clear
```

### `/login`
Claude Code에 로그인하여 인증합니다. WebFetch 등의 기능을 사용하려면 로그인이 필요합니다.
```
/login
```

### `/bashes`
실행 중인 백그라운드 bash 셸의 ID를 확인합니다.
```
/bashes
```

---

## 슬래시 커맨드 (Slash Commands)

슬래시 커맨드는 `.claude/commands/` 디렉토리에 마크다운 파일로 저장된 사용자 정의 명령어입니다.

### 커스텀 슬래시 커맨드 생성

1. `.claude/commands/` 디렉토리에 마크다운 파일 생성
2. 파일명이 명령어 이름이 됩니다 (예: `review-pr.md` → `/review-pr`)
3. 파일 내용은 Claude에게 전달될 프롬프트입니다

### 사용 예시
```
/review-pr 123
```
이 명령어는 `.claude/commands/review-pr.md` 파일의 내용을 프롬프트로 사용하여 실행됩니다.

---

## 도구 (Tools)

Claude Code는 다음과 같은 강력한 도구들을 제공합니다:

### 파일 작업

#### `Read`
파일의 내용을 읽습니다.
- 절대 경로 사용 필수
- 이미지, PDF, Jupyter Notebook도 읽을 수 있음
- 기본적으로 최대 2000줄 읽기

#### `Write`
새 파일을 작성하거나 기존 파일을 덮어씁니다.
- 기존 파일을 덮어쓰기 전 반드시 Read 도구로 먼저 읽어야 함
- 절대 경로 사용 필수

#### `Edit`
파일의 특정 부분을 수정합니다.
- 정확한 문자열 매칭 필요
- `replace_all` 옵션으로 모든 발생 항목 치환 가능
- 파일을 편집하기 전 반드시 Read로 먼저 읽어야 함

### 검색 작업

#### `Glob`
파일 패턴 매칭을 통한 빠른 파일 검색
```
패턴 예시:
- **/*.js
- src/**/*.ts
- *.md
```

#### `Grep`
ripgrep 기반의 강력한 코드 검색 도구
- 정규 표현식 지원
- 파일 타입 필터링
- 컨텍스트 라인 표시 (-A, -B, -C)
- 대소문자 무시 검색 (-i)

### 명령 실행

#### `Bash`
지속적인 셸 세션에서 bash 명령어를 실행합니다.
- 최대 10분 타임아웃
- 백그라운드 실행 가능 (`run_in_background`)
- 파일 작업은 전용 도구 사용 권장 (Read, Write, Edit)

#### `BashOutput`
백그라운드에서 실행 중인 bash 셸의 출력을 확인합니다.

#### `KillShell`
실행 중인 백그라운드 bash 셸을 종료합니다.

### 웹 작업

#### `WebSearch`
웹 검색을 수행하여 최신 정보를 가져옵니다.
- 도메인 필터링 지원
- 미국에서만 사용 가능

#### `WebFetch`
특정 URL의 콘텐츠를 가져와서 분석합니다.
- HTML을 마크다운으로 변환
- 15분 캐시 지원
- 인증 필요 시 `/login` 실행

### 작업 관리

#### `TodoWrite`
작업 목록을 생성하고 관리합니다.
- 상태: `pending`, `in_progress`, `completed`
- 복잡한 다단계 작업에 유용
- 진행 상황 추적

#### `Task`
복잡한 작업을 자율적으로 처리하는 에이전트를 실행합니다.

**사용 가능한 에이전트 타입:**
- `general-purpose`: 복잡한 질문 연구, 코드 검색, 다단계 작업
- `statusline-setup`: 상태 라인 설정 구성
- `output-style-setup`: 출력 스타일 생성
- `Explore`: 코드베이스 탐색 전문 (빠른 파일 검색, 코드 검색)

### 사용자 상호작용

#### `AskUserQuestion`
실행 중 사용자에게 질문합니다.
- 사용자 선호도 수집
- 모호한 지시사항 명확화
- 구현 선택사항 결정
- 최대 4개 질문, 각 질문당 2-4개 옵션

### Jupyter Notebook

#### `NotebookEdit`
Jupyter Notebook 셀의 내용을 편집합니다.
- 셀 교체, 삽입, 삭제 가능
- 셀 타입: `code`, `markdown`

---

## Git 관련 작업

Claude Code는 Git 작업을 안전하게 수행합니다.

### Git 커밋 생성

사용자가 명시적으로 요청할 때만 커밋을 생성합니다:

1. `git status`로 추적되지 않은 파일 확인
2. `git diff`로 변경사항 확인
3. `git log`로 최근 커밋 메시지 스타일 확인
4. 변경사항 분석 및 커밋 메시지 초안 작성
5. 관련 파일을 스테이징 영역에 추가
6. 커밋 생성 (자동으로 Claude Code 서명 추가)

**커밋 메시지 형식:**
```
커밋 메시지 내용

🤖 Generated with [Claude Code](https://claude.com/claude-code)

Co-Authored-By: Claude <noreply@anthropic.com>
```

### Git 안전 프로토콜

- Git config 절대 수정 안 함
- 파괴적/비가역적 명령 금지 (예: push --force, hard reset)
- Hook 건너뛰기 금지 (--no-verify, --no-gpg-sign)
- main/master로 강제 푸시 경고
- 커밋 수정 시 authorship 확인

### Pull Request 생성

GitHub CLI (`gh`)를 사용하여 PR을 생성합니다:

1. 분기 이후 모든 변경사항 확인
2. PR 요약 작성 (모든 관련 커밋 분석)
3. 필요 시 새 브랜치 생성 및 푸시
4. `gh pr create`로 PR 생성

**PR 본문 형식:**
```markdown
## Summary
- 주요 변경사항 1
- 주요 변경사항 2

## Test plan
[ ] 테스트 항목 1
[ ] 테스트 항목 2

🤖 Generated with [Claude Code](https://claude.com/claude-code)
```

---

## 커스텀 명령어

### Hooks

사용자는 도구 호출과 같은 이벤트에 반응하여 실행되는 셸 명령인 'hooks'를 설정 파일에 구성할 수 있습니다.

예시:
- `user-prompt-submit-hook`: 사용자 프롬프트 제출 시 실행
- Hook의 피드백은 사용자로부터 온 것처럼 처리됨

### Skills

스킬은 특수한 기능과 도메인 지식을 제공하는 전문화된 워크플로우입니다.

사용 가능한 스킬은 `<available_skills>` 태그에 나열됩니다.

---

## 사용 팁

### 병렬 도구 호출
독립적인 작업들은 단일 메시지에서 여러 도구를 동시에 호출하여 효율성을 극대화합니다.

### 파일 작업 우선순위
1. 가능한 한 기존 파일 편집 (Edit)
2. 새 파일 생성은 반드시 필요할 때만 (Write)
3. 문서 파일은 명시적 요청이 있을 때만 생성

### 검색 전략
- 특정 파일 경로 알 때: `Read` 또는 `Glob`
- 특정 클래스 정의 찾을 때: `Glob`
- 특정 파일 내 검색: `Read`
- 코드베이스 탐색: `Task` (Explore 에이전트)

### 코드 참조
파일 경로와 라인 번호를 포함하여 코드를 참조합니다:
```
src/services/process.ts:712
```

---

## 주의사항

### 보안
- 방어적 보안 작업만 지원
- 악의적으로 사용될 수 있는 코드 생성/수정 거부
- 자격 증명 수집/크롤링 금지
- 보안 분석, 탐지 규칙, 취약점 설명, 방어 도구는 허용

### Git 작업
- 명시적 요청 없이 커밋/푸시 금지
- 대화형 Git 명령어 금지 (예: `git rebase -i`, `git add -i`)
- 비밀 정보가 포함된 파일 커밋 경고

### 파일 작업
- bash를 통한 파일 읽기/쓰기 금지 (전용 도구 사용)
- 상대 경로가 아닌 절대 경로 사용
- 공백이 포함된 경로는 반드시 따옴표로 감싸기

---

## 피드백 및 도움말

- 도움말: `/help`
- 피드백 및 이슈 보고: https://github.com/anthropics/anthropic-code/issues
- 공식 문서: https://docs.claude.com/claude-code

---

**마지막 업데이트:** 2025-10-20
**버전:** Claude Sonnet 4.5

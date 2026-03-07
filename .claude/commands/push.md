# /push - Git 푸시 도우미

**Critical Language Requirement**: You MUST respond in Korean at all times.

---

## 트리거 조건

**TRIGGER when**: 사용자가 다음과 같은 요청을 할 때:
- "푸시", "push", "푸시해줘", "올려줘", "올려"
- "커밋하고 푸시", "커밋하고 올려줘" → 커밋 먼저 실행 후 푸시
- "반영해줘", "원격에 올려줘"

**DO NOT TRIGGER when**:
- 푸시 없이 커밋만 요청할 때 → `/commit` skill 사용
- git 개념을 설명해달라고 할 때

---

## 실행 프로세스

### 1. 현재 상태 확인
```bash
git status
git log origin/{브랜치}..HEAD --oneline
```
- 커밋되지 않은 변경사항이 있으면 먼저 `/commit` 실행 안내
- 푸시할 커밋 목록을 사용자에게 보여줌

### 2. 사용자 확인
```
브랜치: {현재 브랜치}
푸시할 커밋:
  - [PREFIX] 설명1
  - [PREFIX] 설명2

origin/{브랜치}에 푸시하겠습니다. 진행할까요?
```
- `master` 브랜치라면 추가 경고 표시

### 3. 푸시 실행
사용자 확인 후 실행:
```bash
git push origin {현재 브랜치}
```

---

## 주의사항

- 푸시는 반드시 사용자 확인 후 실행 (되돌리기 어려움)
- `--force` / `--force-with-lease` 는 절대 사용하지 않음
- `master` 브랜치 직접 푸시 시 경고 후 재확인
- 커밋이 없으면 푸시하지 않고 안내

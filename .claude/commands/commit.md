# /commit - Git 커밋 도우미

**Critical Language Requirement**: You MUST respond in Korean at all times.

---

## 트리거 조건

**TRIGGER when**: 사용자가 다음과 같은 요청을 할 때:
- "커밋", "commit", "커밋해줘", "커밋하자", "커밋해"
- "저장해줘", "변경사항 저장"
- 푸시 없이 커밋만 요청할 때

**DO NOT TRIGGER when**:
- 커밋 메시지 형식에 대해 단순히 질문할 때
- git 개념을 설명해달라고 할 때
- 푸시까지 요청할 때 → `/push` skill 참고

---

## 커밋 PREFIX 규칙

| PREFIX | 사용 조건 |
|--------|-----------|
| `[FEAT]` | 완성된 새 기능 (게임 플레이 흐름 통합) |
| `[WIP]` | 구현 중인 기능 (빌드 가능 여부 무관) |
| `[DOCS]` | 문서만 수정 (코드 변경 없음) |
| `[FIX]` | 기존 `[FEAT]` 기능 수정/보완 |

**커밋 메시지 형식**:
- **Summary**: `[PREFIX] 한국어 설명 한 줄`
- **Description**: 해당 작업의 상세 내역을 항목별로 요약 (매 커밋 필수)

---

## 실행 프로세스

### 1. 변경사항 파악
```bash
git status
git diff
git diff --staged
```

### 2. 커밋 메시지 생성
변경 내용을 분석해 PREFIX 자동 추천 후 사용자 확인:
```
Summary    : [PREFIX] {설명}
Description:
- {변경 항목 1}
- {변경 항목 2}
- ...

진행할까요? (수정이 필요하면 말씀해주세요)
```

### 3. 스테이징 & 커밋
사용자 확인 후 실행:
```bash
git add {파일 또는 -A}
git commit -m "[PREFIX] 설명" -m "- 변경 항목 1\n- 변경 항목 2\n- ..."
```

---

## 주의사항

- 커밋 메시지는 반드시 한국어로 작성
- Summary와 Description 모두 매 커밋마다 작성 (Description 생략 불가)
- 스테이징되지 않은 파일이 있으면 포함 여부 확인
- 커밋 후 푸시가 필요하면 `/push` 를 별도로 사용

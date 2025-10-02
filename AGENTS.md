
# Repository Guidelines

Unreal Engine 5 + C++ 프로젝트입니다. Rider IDE 기준으로 개발·빌드·테스트와 컨벤션을 명확히 합니다.

## 변수 정의 (범용 템플릿)
- `<PROJECT_NAME>`: Unreal 프로젝트명(예: `MyProject`). 보통 주 모듈명과 동일.
- `<MODULE_NAME>`: 주 게임 모듈명(기본값은 `<PROJECT_NAME>`).
- `<PROJECT_ROOT>`: 프로젝트 루트 디렉터리 경로(예: `C:\UEProject\<PROJECT_NAME>`).
- `<UPROJECT>`: `<PROJECT_NAME>.uproject` 파일명.
- `<UPROJECT_PATH>`: `<PROJECT_ROOT>\<PROJECT_NAME>.uproject`의 전체 경로.
- `<EDITOR_TARGET>`: `<PROJECT_NAME>Editor` 타깃 이름.
- `<PROJECT_LOG_CATEGORY>`: 기본 로그 카테고리(권장: `Log<Module>` 또는 `<PROJECT_NAME>Log`).

## 프로젝트 구조
- `Source/<MODULE_NAME>/{Public,Private}`는 `Source/<MODULE_NAME>/<Domain>` 구조(예: `Components`, `Ai`, `UI`)를 따르며 클래스당 `Public/<Domain>/<ClassName>.h`와 `Private/<Domain>/<ClassName>.cpp` 한 쌍(`.h/.cpp`)으로, 파일명은 클래스명과 동일하게 배치합니다(예: `UserWidget.h` → `Source/<MODULE_NAME>/Public/UI/UserWidget.h`, `UserWidget.cpp` → `Source/<MODULE_NAME>/Private/UI/UserWidget.cpp`). 엔트리 파일은 `<MODULE_NAME>.Build.cs`, `<PROJECT_NAME>.Target.cs`, `<PROJECT_NAME>Editor.Target.cs`, `<PROJECT_NAME>.uproject`입니다.
- `Content/Blueprints`, `Content/Animation`, `Content/UI`: 블루프린트/애님/위젯 분리. `ABP_Player` 경로: `/Game/Blueprints/ABP_Player.ABP_Player_C`.
- `Config/Default*.ini`: 설정 관리. `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`는 생성물이므로 커밋 금지.

## 빌드 · 실행 · 개발
- Rider: 구성 `<EDITOR_TARGET> | Win64 | Development`(또는 `DebugGame`) 선택 → F5(디버그/PIE).
- 프로젝트 파일 재생성: `Tools → Unreal → Generate Project Files` 또는 `<UE5>\Engine\Build\BatchFiles\GenerateProjectFiles.bat -project="<UPROJECT_PATH>"`.
- 커맨드라인 빌드: `<UE5>\Engine\Build\BatchFiles\Build.bat <EDITOR_TARGET> Win64 Development -Project="<UPROJECT_PATH>"`.
- 쿠킹/패키징(예): `<UE5>\Engine\Build\BatchFiles\RunUAT.bat BuildCookRun -project="<UPROJECT_PATH>" -platform=Win64 -clientconfig=Shipping -cook -allmaps -build -stage -pak`.

## 코딩 스타일 · 네이밍
- UE 스타일: `UCLASS`/`USTRUCT`/`UFUNCTION` 등 매크로 사용 및 Unreal 리플렉션 규칙 준수.
- PascalCase: 타입/함수/대부분의 변수는 PascalCase 사용.
- 헝가리안 접두사(예외):
  - 불리언: `b` 접두사 사용 — 예) `bIsAlive`, `bCanAttack`.
  - 포인터: `p` 접두사 사용 — 예) `pActor`, `pData` (그 외 타입은 접두사 없이 PascalCase).
- 클래스/타입 접두사: `U/A/F/I/T/E/S` 사용.
- 헤더 최소 포함·전방 선언, `#pragma once`. 공개 API만 `Public/`, 나머지는 `Private/`.
- 로그: 모듈별 `DECLARE_LOG_CATEGORY_EXTERN`/`DEFINE_LOG_CATEGORY`로 카테고리 정의. 기본 표준명: `<PROJECT_NAME>Log` 또는 `Log<Module>`. 사용: `UE_LOG(<PROJECT_LOG_CATEGORY>, Log, TEXT("..."));`.
- 편의 매크로: 호출 파일/라인/함수 자동 출력. 권장 위치: `Source/<MODULE_NAME>/<PROJECT_NAME>.h`.
  - 정보: `SHOWLOG()`, `SHOWLOGF(Format, ...)`
  - 경고: `SHOWWARN()`, `SHOWWARNF(Format, ...)`
  - 오류: `SHOWERROR()`, `SHOWERRORF(Format, ...)`
- 인클루드 경로는 모듈 상대. 애셋/소켓 네이밍 일관: `ABP_*`, `BP_*`, `WBP_*`, `SK_/SM_`, `M_/MI_`, `HandGrip_R` 등.

## 테스트 지침
- 권장: Unreal Automation Tests(C++), Functional Tests(게임플레이). 별도 모듈 `Source/<PROJECT_NAME>Tests` 구성, `WITH_AUTOMATION_TESTS` 가드.
- 실행: Session Frontend → Automation 또는 `<UE5>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "<UPROJECT_PATH>" -ExecCmds="Automation RunTests <TEST_FILTER>" -unattended -testexit`.
  - `<TEST_FILTER>`: 보통 `<MODULE_NAME>` 또는 테스트 카테고리 프리픽스 사용.

## 커밋 · PR 가이드
- 커밋 메시지 규칙
  - 제목은 대문자 대괄호 접두사(`[FEAT]`, `[FIX]`, `[REFACTOR]`, `[DOCS]`, `[TEST]`, `[BUILD]`, `[CI]`, `[PERF]`, `[STYLE]`, `[CHORE]`, `[REVERT]`)를 붙인 명령형 요약 한 줄(72자 이내)로 작성하며, 본 프로젝트는 Conventional Commits의 콜론 표기(`feat:`) 대신 대괄호 표기(`[FEAT]`)를 사용합니다(예: `[FEAT] 플레이어 대시 기능 추가`, `[FIX] 넉백 계산 오버플로우 수정`).
  - 본문(선택): 변경 이유/배경, 영향 범위, 마이그레이션/리스크, 테스트 방법을 단락으로 기술. 한 줄 72자 내 줄바꿈 권장.
  - 이슈 연계: 필요 시 `Refs: #123` 또는 `Fixes: #123` 표기.
- PR: 목적/변경점/리스크/테스트 절차 명시, 시각적 변경은 스크린샷·GIF 첨부. PR은 작고 집중적으로 유지.

## 보안 · 설정
- 비밀정보·머신별 설정 커밋 금지. `Default*.ini`만 버전관리. `.gitignore`에 생성물 제외 항목 유지.

## 에이전트 지침
 - 코드 우선, 생성물·에셋 직접 수정 금지. UE 모듈 레이아웃과 본 지침 준수. AnimBP 경로 변경 시 이 문서 동기화.
 - 응답 언어: 모든 에이전트 메시지는 항상 한국어로 작성합니다.
 - 범위 고정: 모든 질문과 응답은 항상 Unreal Engine 5와 C++ 컨텍스트를 전제로 합니다.
 - Codex CLI 에이전트는 본 AGENTS.md를 준수합니다. 리포지터리 전역에 적용되며, 더 깊은 경로에 별도 AGENTS.md가 있으면 해당 지침이 우선합니다.
  - 변경 금지 파일: `Source/<PROJECT_NAME>.Target.cs`, `Source/<PROJECT_NAME>Editor.Target.cs` 파일은 절대 수정하지 않습니다. 빌드 관련 변경은 `Source/<MODULE_NAME>/<MODULE_NAME>.Build.cs` 또는 `<PROJECT_NAME>.uproject`로 한정합니다.
  - 엔진 버전: 절대 임의로 변경하지 않습니다. (`.uproject`의 `EngineAssociation` 포함)
  - 예외: 편의 매크로 정의 파일 `Source/<MODULE_NAME>/<PROJECT_NAME>.h` 및 구현 `Source/<MODULE_NAME>/<PROJECT_NAME>.cpp`가 레거시로 존재할 수 있습니다. 신규 파일에는 예외를 적용하지 않습니다.
 - 파일 수정은 필요한 부분만 최소로 편집하며, 기존 내용을 삭제했다가 동일하게 다시 삽입하는 방식은 금지합니다.
 - 널 가드는 null 발생 가능성이 필연적인 위치에만 작성하고, 잠재적 위험 지점에는 주석으로 널 가드 주의를 명시합니다.
 - Codex는 작업 시작 전에 작업계획·작업과정·작업결과검토 순서의 체크리스트를 작성하고 전 과정을 통해 체크리스트를 지속적으로 점검합니다. 체크리스트를 사용자에게 확인받을 때에는 체크리스트와 함께 삽입·삭제·수정 예정 문구를 구체적으로 제시합니다.
 - 코드 수정은 한 번에 하나의 함수만 다루고, 계획한 절차에 따라 해당 함수 구현부에 TODO와 로직을 한 줄씩 주석으로 작성해 사용자 확인을 받은 뒤 다음 단계로 진행합니다. 주석에는 함수가 호출될 흐름과 필요한 연계 코드, 각 매개변수가 왜 필요한지와 사용 방식을 구조적으로 서술합니다.
 - 헤더에서 함수 또는 변수 선언을 변경할 때도 동일한 계획과 TODO/주석 작성 절차를 선행하고 사용자 확인 후에 구현을 진행합니다. 주석에는 해당 선언이 전체 코드에서 담당할 동작과 역할을 간결히 요약합니다.

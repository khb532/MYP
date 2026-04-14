# CLAUDE.md

이 문서는 Claude Code(claude.ai/code)가 이 저장소의 코드를 작업할 때 참고하는 가이드입니다.

## 언어 요구사항
**이 프로젝트에서는 항상 한국어로 응답해야 합니다.**

## 프로젝트 개요

Unreal Engine 5 기반의 C++ 게임 프로젝트

**엔진 버전**: Unreal Engine 5.x
**모듈 타입**: Runtime (단일 또는 다중 모듈)
**주요 언어**: C++ (블루프린트 통합)

## 아키텍처 개요

### 모듈 의존성

**일반적인 필수 모듈** (`{ProjectName}.Build.cs`):
```
Core, CoreUObject, Engine, InputCore, EnhancedInput
```

**선택적 모듈 (필요에 따라)**:
```
AIModule, StateTreeModule, GameplayStateTreeModule,
UMG, Slate, SlateCore
```

**Include 경로 구조**:
- Private 코드는 `Private/` 폴더, public 코드는 `Public/` 폴더

### 로깅 시스템

프로젝트에서 커스텀 로그 카테고리와 편의 매크로를 사용하는 경우:

**중앙 로그 카테고리**: 프로젝트의 메인 헤더 파일 (`{ProjectName}.h`)에 선언

**로그 카테고리**: `MYPLog` (Warning 기본, All 컴파일)

**편의 매크로** (`Source/MYP/MYP.h`):
```cpp
LOGMSG()                   // 함수명[라인] — Warning 레벨
LOGMSGF(Format, ...)       // 함수명[라인] | 메시지 — Warning 레벨
LOGWARN()                  // 함수명[라인] — Warning 레벨
LOGWARNF(Format, ...)      // 함수명[라인] | 메시지 — Warning 레벨
LOGERROR()                 // 함수명[라인] — Error 레벨
LOGERRORF(Format, ...)     // 함수명[라인] | 메시지 — Error 레벨
```

**사용법**: 프로젝트에 정의된 로깅 매크로가 있다면 일관성을 위해 `UE_LOG` 대신 해당 매크로를 선호합니다.
- 포맷 없이 위치만 남길 때: `LOGMSG()`, `LOGWARN()`, `LOGERROR()`
- 값을 함께 출력할 때: `LOGMSGF(TEXT("Speed=%f"), Speed)`

## 개발 가이드라인

### 코드 규칙

**네이밍**:
- 클래스: Actor는 `A` 접두사, UObject 파생은 `U` 접두사, 구조체는 `F` 접두사, 인터페이스는 `I` 접두사

**인터페이스 구현**:
- 인터페이스 메서드는 항상 헤더 선언과 C++ 정의 모두 구현
- `UINTERFACE`/`IInterface` 패턴 사용 (UE4/5 표준)


## 산출물 저장 경로

레포트, 문서, 설계 초안 등 모든 산출물은 아래 경로에 저장합니다:

```
D:\DoveObs\Reports\MYP\{BranchName}\
```

- `{BranchName}`은 산출물 생성 시점에 `git branch --show-current`로 확인한 현재 브랜치명을 사용
- 일일 레포트: `Daily_Report_{YYYY-MM-DD}.md`
- 설계 문서, 기획 초안 등: 내용에 맞는 파일명으로 동일 폴더에 저장

## 주요 파일 레퍼런스

| 파일 | 목적 |
|------|---------|
| `Source/{ProjectName}/{ProjectName}.Build.cs` | 모듈 의존성 및 include 경로 |
| `Source/{ProjectName}/{ProjectName}.h` | 로그 카테고리 정의 및 편의 매크로 (선택) |
| `Config/DefaultEngine.ini` | 엔진 설정 (렌더러, 입력, 맵) |
| `Config/DefaultInput.ini` | 입력 액션 매핑 |

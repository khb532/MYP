# CLAUDE.md

이 문서는 Claude Code(claude.ai/code)가 이 저장소의 코드를 작업할 때 참고하는 가이드입니다.

## 언어 요구사항
**이 프로젝트에서는 항상 한국어로 응답해야 합니다.**

## 프로젝트 개요

**MYP**는 Unreal Engine 5.6 기반의 게임 프로젝트

**엔진 버전**: Unreal Engine 5.6
**모듈 타입**: Runtime (단일 모듈)
**주요 언어**: C++ (블루프린트 통합)

## 빌드 및 개발 명령어

### 프로젝트 빌드
```bash
# 프로젝트 파일 생성 (.sln 파일이 없거나 오래된 경우)
# MYP.uproject 우클릭 → "Generate Visual Studio project files"

# Rider에서 빌드
# MYP.sln 열기 → Build → Build Solution (Ctrl+Shift+B)
# Configuration: Development Editor
# Platform: Win64
```

### 프로젝트 실행
```bash
# Unreal Editor에서 실행
# MYP.uproject 더블클릭

# 또는 Rider에서 빌드 후 실행
# MYP를 시작 프로젝트로 설정 → Debug → Start Without Debugging (Ctrl+F5)
```

### 일반적인 개발 작업
- **C++ 변경사항 컴파일**: 에디터에서 Hot Reload (Ctrl+Alt+F11) 또는 솔루션 리빌드
- **.sln 생성**: .uproject 우클릭 → "Generate Visual Studio project files"
- **클린 빌드**: `Binaries/`, `Intermediate/`, `Saved/` 폴더 삭제 후 프로젝트 재생성

## 아키텍처 개요

### 모듈 의존성

**필수 모듈** (MYP.Build.cs):
```
Core, CoreUObject, Engine, InputCore, EnhancedInput,
AIModule, StateTreeModule, GameplayStateTreeModule,
UMG, Slate
```

**Include 경로 구조**:
- Private 코드는 `Private/` 폴더, public 코드는 `Public/` 폴더

### 로깅 시스템

**중앙 로그 카테고리**: `MYPLOG` (`MYP.h`에 선언)

**편의 매크로**:
```cpp
SHOWLOG(Format, ...)     // 파일:라인:함수 포함 Info 레벨
SHOWLOGF(Format, ...)    // 포맷 인자 포함 Info
SHOWWARN(Format, ...)    // Warning 레벨
SHOWERROR(Format, ...)   // Error 레벨
```

**사용법**: 일관성을 위해 `UE_LOG` 대신 이 매크로들을 선호—자동으로 소스 위치 포함.

## 개발 가이드라인

### 코드 규칙

**네이밍**:
- 클래스: Actor는 `A` 접두사, UObject 파생은 `U` 접두사, 구조체는 `F` 접두사, 인터페이스는 `I` 접두사

**인터페이스 구현**:
- 인터페이스 메서드는 항상 헤더 선언과 C++ 정의 모두 구현
- `UINTERFACE`/`IInterface` 패턴 사용 (UE4/5 표준)

## Git 워크플로우

**브랜치**:
- `master`: 메인 안정 브랜치 (PR용)
- `Dev`: 활성 개발 브랜치

## 커밋 가이드라인

**형식**: `[PREFIX] 한국어 설명 한 줄`

**접두사**:
- `[FEAT]` - 완성된 새 기능 (게임 플레이 흐름 통합)
- `[WIP]` - 구현 중인 기능 (빌드 가능 여부 무관)
- `[DOCS]` - 문서만 수정 (코드 변경 없음)
- `[FIX]` - 기존 `[FEAT]` 기능 수정/보완

## 주요 파일 레퍼런스

| 파일 | 목적 |
|------|---------|
| `MYP.Build.cs` | 모듈 의존성 및 include 경로 |
| `MYP.h` | 로그 카테고리 정의 및 편의 매크로 |
| `Config/DefaultEngine.ini` | 엔진 설정 (렌더러, 입력, 맵) |
| `Config/DefaultInput.ini` | 입력 액션 매핑 |

## 문제 해결

**Hot Reload 실패**:
- `Binaries/`, `Intermediate/` 폴더 삭제
- 프로젝트 파일 재생성 (.uproject 우클릭)
- Rider에서 솔루션 리빌드

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Main log category used across the project */
DECLARE_LOG_CATEGORY_EXTERN(MYPLog, Log, All);

/*
- DECLARE_LOG_CATEGORY_EXTERN(Name, DefaultVerbosity, CompileTimeVerbosity):
  UE 로그 카테고리를 다른 모듈/파일에서 참조 가능하도록 외부 선언합니다. 구현 파일에서 DEFINE_LOG_CATEGORY로 정의합니다.
  
- DEFINE_LOG_CATEGORY(Name): 선언된 로그 카테고리를 실제로 정의합니다.

- UE_LOG(Category, Verbosity, Format, ...): Unreal의 표준 로그 매크로입니다. printf 스타일 포맷을 사용합니다.

- TEXT("..."): 리터럴을 UE의 `TCHAR`(플랫폼별 와이드/좁은)로 변환합니다.

- ANSI_TO_TCHAR(AnsiStr): `char*`(ANSI) → `TCHAR*`로 변환합니다. `__FILE__`, `__FUNCTION__`은 보통 ANSI이므로 변환이 필요합니다.

- __FILE__ / __LINE__ / __FUNCTION__:
  컴파일러가 제공하는 사전정의 매크로로, 각각 현재 소스 파일 경로, 라인 번호, 함수 이름으로 확장됩니다.
  
- 가변 인자 매크로: `#define MACRO(fmt, ...)` 형태로 정의하며, 본문에서 `__VA_ARGS__`로 전달 인자를 참조합니다.
  `##__VA_ARGS__`는 인자가 비어 있을 때 선행 콤마를 제거하는 MSVC/GCC 확장 문법입니다.
  
- #ifndef / #define / #endif: 매크로가 이미 정의된 경우 중복 정의를 방지합니다(조건부 컴파일 가드).

- 로그 레벨(Verbosity): `Log`(정보), `Warning`(경고), `Error`(오류)를 사용합니다.
주의: Shipping 빌드에서는 일부 로그가 무시되거나 제거됩니다. 민감정보 로그 금지.
*/

#ifndef SHOWLOG
#define SHOWLOG() \
    UE_LOG(MYPLog, Log, TEXT("[%s:%d] %s"), \
        ANSI_TO_TCHAR(__FILE__), \
        __LINE__, \
        ANSI_TO_TCHAR(__FUNCTION__))
#endif

// 메시지를 추가로 남기고 싶을 때 사용
// 사용 예) SHOWLOGF(TEXT("Speed=%f"), Speed);
#ifndef SHOWLOGF
#define SHOWLOGF(Format, ...) \
    UE_LOG(MYPLog, Log, TEXT("[%s:%d] %s | " Format), \
        ANSI_TO_TCHAR(__FILE__), \
        __LINE__, \
        ANSI_TO_TCHAR(__FUNCTION__), \
        ##__VA_ARGS__)
#endif

// 경고 레벨 로그
#ifndef SHOWWARN
#define SHOWWARN() \
    UE_LOG(MYPLog, Warning, TEXT("[%s:%d] %s"), \
        ANSI_TO_TCHAR(__FILE__), \
        __LINE__, \
        ANSI_TO_TCHAR(__FUNCTION__))
#endif

#ifndef SHOWWARNF
#define SHOWWARNF(Format, ...) \
    UE_LOG(MYPLog, Warning, TEXT("[%s:%d] %s | " Format), \
        ANSI_TO_TCHAR(__FILE__), \
        __LINE__, \
        ANSI_TO_TCHAR(__FUNCTION__), \
        ##__VA_ARGS__)
#endif

// 오류 레벨 로그
#ifndef SHOWERROR
#define SHOWERROR() \
    UE_LOG(MYPLog, Error, TEXT("[%s:%d] %s"), \
        ANSI_TO_TCHAR(__FILE__), \
        __LINE__, \
        ANSI_TO_TCHAR(__FUNCTION__))
#endif

#ifndef SHOWERRORF
#define SHOWERRORF(Format, ...) \
    UE_LOG(MYPLog, Error, TEXT("[%s:%d] %s | " Format), \
        ANSI_TO_TCHAR(__FILE__), \
        __LINE__, \
        ANSI_TO_TCHAR(__FUNCTION__), \
        ##__VA_ARGS__)
#endif

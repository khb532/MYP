# Dynamic Particle System - Beta 개발 계획

**프로젝트**: Dynamic Particle System (사용자 정의 Lua 스크립트로 파티클 제어)
**단계**: Beta - 게임플레이 통합 및 Lua 런타임 시스템
**작성일**: 2025년 12월 14일
**브랜치**: DynamicNiagara

---

## 📋 Beta 목표

**Alpha-1 완성 사항**:
- ✅ OpCode 시스템 설계
- ✅ HLSL Compute Shader (스택 기반 VM)
- ✅ RDG 기반 GPU 버퍼 관리
- ✅ PlateActor + ParticleComputeComponent
- ✅ 하드코딩된 바이트코드로 GPU 실행 검증 완료

**Beta 목표**:
- 🎯 **Niagara 파티클 시각화**
- 🎯 **Lua 스크립트 런타임 입력 시스템**
- 🎯 **게임플레이 통합** (Input → Spawn → UI → Effect)
- 🎯 **사용자 경험 완성**

---

## 🎮 게임플레이 플로우

```
1. 사용자 입력
   └─ 특정 키 입력 또는 버튼 UI 클릭

2. PlateActor 스폰
   └─ Player 전방 200cm 위치에 스폰

3. Lua 코드 입력 UI 팝업
   └─ MultiLineEditableTextBox + 저장/취소 버튼

4. 코드 입력 및 저장
   └─ 사용자가 Lua 스크립트 작성 후 "Save & Run" 클릭

5. 컴파일 및 실행
   └─ Lua → 바이트코드 → GPU Compute Shader

6. 파티클 이펙트 재생
   └─ Niagara가 RenderTarget 읽어서 파티클 표시
```

---

## 🏗️ 구현 단계

### **Step 1: Niagara 연동** ⭐ 우선순위 1

**목표**: GPU에서 계산한 데이터를 Niagara로 시각화

**작업 내용**:
- [ ] PlateActor에 `TSubclassOf<UNiagaraSystem>` 추가
- [ ] `UNiagaraComponent` 생성 및 설정
- [ ] Niagara System Asset 생성
  - User Parameter로 PositionRT, ColorRT 받기
  - Sample Texture로 파티클 데이터 읽기
  - Sprite Renderer 설정
- [ ] BP_PlateActor 생성 및 Niagara 할당
- [ ] 파티클 시각화 테스트 (100개 빨간 파티클, X축 일렬)

**예상 소요 시간**: 1-2일

**완료 조건**:
- 화면에서 100개 파티클이 X축으로 일렬로 표시됨
- 파티클 색상이 빨간색으로 표시됨

---

### **Step 2: Input System 및 PlateActor 스폰** ⭐ 우선순위 2

**목표**: 사용자 입력으로 PlateActor 스폰

**작업 내용**:
- [ ] Enhanced Input 설정 (또는 기존 Input 확장)
  - SpawnPlate 액션 추가 (예: 'P' 키)
- [ ] PlayerController 또는 Pawn에 스폰 함수 구현
  - Player 전방 200cm 위치 계산
  - `GetWorld()->SpawnActor<APlateActor>()`
- [ ] 버튼 UI 추가 (선택사항)
  - UMG 버튼 위젯
  - OnClicked → SpawnPlate 호출

**예상 소요 시간**: 0.5-1일

**완료 조건**:
- 키 입력 또는 버튼 클릭 시 PlateActor 스폰
- Player 전방 200cm 위치에 정확히 배치
- 파티클 이펙트 자동 재생

---

### **Step 3: Lua 코드 입력 UI (UMG)** ⭐ 우선순위 3

**목표**: 사용자가 Lua 스크립트를 입력할 수 있는 UI

**작업 내용**:
- [ ] WBP_LuaCodeEditor 위젯 생성
  - Canvas Panel 레이아웃
  - 배경 (반투명 검정)
  - 타이틀 Text (Particle Code Editor)
- [ ] MultiLineEditableTextBox 추가
  - 기본 텍스트: 예제 Lua 코드
  - 폰트: Monospace (Courier New 등)
  - 힌트 텍스트: "-- Write Lua code here..."
- [ ] 버튼 2개 추가
  - Cancel 버튼 → UI 닫기
  - Save & Run 버튼 → 코드 저장 및 실행
- [ ] 팝업/팝다운 애니메이션
  - Fade In/Out
  - Scale 애니메이션 (선택사항)

**예상 소요 시간**: 1-2일

**완료 조건**:
- UI가 깔끔하게 표시됨
- 텍스트 입력 및 편집 가능
- 저장/취소 버튼 동작 확인

---

### **Step 4: LuaCompiler 구현** ⭐ 우선순위 4 (핵심)

**목표**: Lua 스크립트를 OpCode 바이트코드로 변환

**작업 내용**:

#### **4-1. LuaJIT 통합**
- [ ] MYP.Build.cs에 LuaJIT 라이브러리 추가
  - ThirdParty/LuaJIT 폴더 생성
  - Include/Lib 파일 배치
  - PublicIncludePaths, PublicAdditionalLibraries 설정
- [ ] Lua 초기화 테스트
  - lua_State 생성/해제
  - 간단한 스크립트 실행 테스트

#### **4-2. Lua DSL 설계**
- [ ] 파티클 스크립트 문법 정의
  ```lua
  -- 변수: particleID, time, deltaTime
  -- 함수: sin, cos, sqrt, pow

  position.x = particleID * 10.0
  position.y = sin(time + particleID) * 50.0
  position.z = 0.0

  color.r = 1.0
  color.g = sin(time) * 0.5 + 0.5
  color.b = 0.0
  color.a = 1.0
  ```
- [ ] 지원 기능 명세
  - 변수: particleID, time, deltaTime
  - 연산자: +, -, *, /
  - 함수: sin, cos, sqrt, pow
  - 출력: position.x/y/z, color.r/g/b/a

#### **4-3. Lua → AST 파싱**
- [ ] 재귀 하강 파서 구현
  - 표현식 파싱 (Expression)
  - 할당문 파싱 (Assignment)
  - 함수 호출 파싱 (FunctionCall)
- [ ] AST 노드 정의
  ```cpp
  enum class EASTNodeType { Literal, Variable, BinaryOp, FunctionCall, Assignment };
  struct FASTNode { EASTNodeType Type; ... };
  ```

#### **4-4. AST → OpCode 변환**
- [ ] 코드 생성기 구현
  - AST 순회 (DFS)
  - OpCode 생성 (PUSH, ADD, MUL, SIN, STORE 등)
  - 상수 풀 관리
- [ ] 최적화 (선택사항)
  - 상수 폴딩 (Constant Folding)
  - 불필요한 PUSH/POP 제거

#### **4-5. ULuaCompiler 클래스 작성**
- [ ] ULuaCompiler.h/cpp 생성
  ```cpp
  class ULuaCompiler : public UObject
  {
      bool CompileScript(const FString& LuaCode,
                        TArray<uint32>& OutBytecode,
                        TArray<float>& OutConstants,
                        FString& OutError);
  };
  ```
- [ ] 에러 핸들링
  - 구문 오류 메시지
  - 라인 번호 표시

**예상 소요 시간**: 3-4일

**완료 조건**:
- Lua 스크립트 → 바이트코드 변환 성공
- 기존 하드코딩 바이트코드와 동일한 결과 생성
- 에러 처리 완료

---

### **Step 5: 통합 및 연동** ⭐ 우선순위 5

**목표**: UI, Compiler, PlateActor 연결

**작업 내용**:
- [ ] PlateActor에 LuaCompiler 추가
  ```cpp
  UPROPERTY()
  ULuaCompiler* LuaCompiler;
  ```
- [ ] UI 저장 버튼 이벤트
  - WBP_LuaCodeEditor → PlateActor 참조
  - OnSaveClicked → PlateActor->SetLuaScript() 호출
- [ ] PlateActor::SetLuaScript() 구현
  ```cpp
  void SetLuaScript(const FString& LuaCode)
  {
      TArray<uint32> Bytecode;
      TArray<float> Constants;

      if (LuaCompiler->CompileScript(LuaCode, Bytecode, Constants, Error))
      {
          ComputeComponent->UploadBytecode(Bytecode, Constants);
          // UI 닫기
      }
      else
      {
          // 에러 메시지 표시
      }
  }
  ```
- [ ] 전체 플로우 테스트

**예상 소요 시간**: 1일

**완료 조건**:
- 키 입력 → 스폰 → UI 팝업 → 코드 입력 → 저장 → 파티클 재생
- 전체 플로우가 매끄럽게 연결됨

---

## 📊 개발 일정 (예상)

| 단계 | 작업 | 소요 시간 | 누적 |
|------|------|-----------|------|
| Step 1 | Niagara 연동 | 1-2일 | 2일 |
| Step 2 | Input System & Spawn | 0.5-1일 | 3일 |
| Step 3 | Lua 입력 UI | 1-2일 | 5일 |
| Step 4 | LuaCompiler 구현 | 3-4일 | 9일 |
| Step 5 | 통합 및 연동 | 1일 | 10일 |
| **합계** | | **약 2주** | |

---

## 🎯 Beta 완료 기준

**필수 기능**:
- ✅ 사용자가 키/버튼으로 PlateActor 스폰 가능
- ✅ Lua 코드 입력 UI 정상 작동
- ✅ Lua 스크립트 → 바이트코드 컴파일 성공
- ✅ GPU Compute Shader 실행 및 파티클 시각화
- ✅ 전체 플로우가 끊김 없이 작동

**예시 시나리오**:
```
1. 플레이어가 'P' 키 입력
2. 전방 200cm에 PlateActor 스폰
3. Lua 코드 입력 UI 팝업
4. 사용자 코드 입력:
   position.x = particleID * 10
   position.y = sin(time + particleID) * 50
   position.z = 0
   color = vec4(1, 0.5, 0, 1)  // 주황색
5. "Save & Run" 클릭
6. UI 닫힘, 파티클이 물결치며 주황색으로 표시됨 ✨
```

---

## 📝 기술 스택

**언어/라이브러리**:
- C++ (Unreal Engine 5)
- HLSL (Compute Shader)
- Lua 5.1 (LuaJIT)
- UMG (UI)

**주요 시스템**:
- RDG (Render Dependency Graph)
- Global Shader System
- Enhanced Input (또는 Legacy Input)
- Niagara Particle System

---

## 🚧 리스크 및 대응

**리스크 1: Lua 파싱 복잡도**
- **대응**: 단순한 문법부터 시작 (변수, 연산자, 할당문만)
- **대안**: Blueprint 노드로 먼저 프로토타입

**리스크 2: Niagara 연동 어려움**
- **대응**: Unreal 공식 문서/샘플 참고
- **대안**: 디버그 DrawDebugPoint로 먼저 검증

**리스크 3: 성능 문제**
- **대응**: Lua 컴파일은 1회만 (캐싱)
- **모니터링**: GPU 프레임타임 측정

---

## 🎨 UI 모의도

```
┌────────────────────────────────────┐
│  Particle Code Editor              │
├────────────────────────────────────┤
│ ┌────────────────────────────────┐ │
│ │ -- Lua Code Here               │ │
│ │ position.x = particleID * 10   │ │
│ │ position.y = sin(time) * 50    │ │
│ │ position.z = 0                 │ │
│ │                                │ │
│ │ color.r = 1.0                  │ │
│ │ color.g = 0.5                  │ │
│ │ color.b = 0.0                  │ │
│ │ color.a = 1.0                  │ │
│ └────────────────────────────────┘ │
│                                    │
│  [Cancel]            [Save & Run]  │
└────────────────────────────────────┘
```

---

## 📚 참고 자료

**Unreal 문서**:
- Niagara System User Guide
- UMG UI Designer Guide
- Enhanced Input System

**LuaJIT**:
- LuaJIT Official Documentation
- Lua 5.1 Reference Manual

**예제 프로젝트**:
- Unreal Niagara Samples
- GPU Particle Systems

---

**작성자**: Claude Code
**최종 수정**: 2025년 12월 14일

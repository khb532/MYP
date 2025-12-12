# 파티클 시스템 Alpha 버전 - 신속 시연용 개발 계획

## 프로젝트 목표

**핵심 질문에 대한 답 얻기**: "Lua → 바이트코드 → GPU Compute Shader → 나이아가라 파이프라인이 실제로 작동하는가?"

**시연 목표**:
- 하드코딩된 Lua 코드 1개를 GPU에서 실행
- 파티클이 나이아가라로 렌더링
- 싱글플레이 환경
- 크래시 없이 안정적으로 작동

**제외 사항**:
- UI 시스템 (런타임 코드 입력 불가, C++에 Lua 스크립트 하드코딩)
- 기즈모 시스템 (위치/회전/스케일 조작 불가, 고정 위치만)
- 멀티플레이
- 에러 핸들링/보안/샌드박스
- 고급 기능 (Flocking, 라이브러리 등)
- 최적화 및 폴리싱

**유지 사항**:
- ✅ 플레이트 스폰 및 배치 (고정 위치)
- ✅ Lua → 바이트코드 파이프라인 (개발자가 하드코딩한 스크립트)
- ✅ GPU Compute Shader 실행
- ✅ 나이아가라 렌더링

---

## Alpha 개발 계획 (1-2주)

### Phase Alpha-1: 최소 GPU 파이프라인 (3-4일)

#### 목표
바이트코드를 GPU에서 실행하고 결과를 **RenderTarget2D에 유지** (Zero-Copy)

> **⚠️ 중요 아키텍처 변경 (Gemini 검토 반영)**:
> - ❌ 기존: GPU → CPU TArray 변환 → 나이아가라 (PCIe 병목 발생)
> - ✅ 변경: GPU → RenderTarget2D → 나이아가라 직접 읽기 (Zero-Copy)

#### 구현 항목

**1. 바이트코드 정의** (1일)
- [ ] 최소 OpCode 집합 정의 (10-15개)
  ```cpp
  enum class EOpCode : uint8
  {
      PUSH_CONST,      // 상수 스택에 푸시
      PUSH_VAR,        // 변수 스택에 푸시 (particleId, time, etc)
      ADD, SUB, MUL, DIV,
      SIN, COS,
      STORE_POS_X,     // 결과 위치 X 저장
      STORE_POS_Y,
      STORE_POS_Z,
      STORE_COLOR_R,   // 결과 색상 R 저장
      STORE_COLOR_G,
      STORE_COLOR_B,
      HALT             // 실행 종료
  };
  ```

- [ ] 하드코딩된 바이트코드 예제 작성
  ```cpp
  // 예제: 원형 모션
  // x = cos(time + particleId) * 100
  // y = sin(time + particleId) * 100
  // z = 0
  TArray<uint32> Bytecode = {
      PUSH_VAR, 1,        // time
      PUSH_VAR, 0,        // particleId
      ADD,
      COS,
      PUSH_CONST, 0,      // 100.0f (상수 인덱스 0)
      MUL,
      STORE_POS_X,

      // ... (y, z 동일 패턴)
  };
  ```

**2. Compute Shader 구현** (1-2일)
- [ ] `ParticleSimulation.usf` 생성
  ```hlsl
  // ⚠️ 수정: RenderTarget2D에 직접 기록
  RWTexture2D<float4> PositionTexture;  // RGB = Position, A = unused
  RWTexture2D<float4> ColorTexture;     // RGB = Color, A = unused
  StructuredBuffer<uint> BytecodeBuffer;
  StructuredBuffer<float> ConstantBuffer;

  float Time;
  uint ParticleCount;
  uint TextureWidth;  // 예: 32x32 = 1024 파티클

  [numthreads(8, 8, 1)]
  void MainCS(uint3 ThreadId : SV_DispatchThreadID)
  {
      uint2 TexCoord = ThreadId.xy;
      uint ParticleId = TexCoord.y * TextureWidth + TexCoord.x;

      if (ParticleId >= ParticleCount) return;

      // 스택 기반 인터프리터 구현
      float Stack[32];
      int StackPtr = 0;
      float3 ResultPos = float3(0, 0, 0);
      float3 ResultColor = float3(1, 1, 1);

      // 바이트코드 실행 루프
      uint PC = 0;
      while (PC < BytecodeLength)
      {
          uint OpCode = BytecodeBuffer[PC++];

          switch (OpCode)
          {
              case OP_PUSH_CONST:
                  Stack[StackPtr++] = ConstantBuffer[BytecodeBuffer[PC++]];
                  break;
              case OP_ADD:
                  Stack[StackPtr-2] = Stack[StackPtr-2] + Stack[StackPtr-1];
                  StackPtr--;
                  break;
              case OP_STORE_POS_X:
                  ResultPos.x = Stack[--StackPtr];
                  break;
              // ... 나머지 OpCode 구현
              case OP_HALT:
                  break;
          }
      }

      // ✅ GPU 메모리에 직접 저장 (CPU로 내려가지 않음!)
      PositionTexture[TexCoord] = float4(ResultPos, 1.0);
      ColorTexture[TexCoord] = float4(ResultColor, 1.0);
  }
  ```

**3. C++ GPU 리소스 관리** (1일)
- [ ] `UParticleComputeComponent` 클래스 생성
  ```cpp
  UCLASS()
  class UParticleComputeComponent : public UActorComponent
  {
      GENERATED_BODY()

  public:
      void InitializeBuffers(int32 NumParticles);
      void UploadBytecode(const TArray<uint32>& Bytecode);
      void ExecuteSimulation(float DeltaTime);

      // ✅ 변경: CPU 다운로드 함수 제거!
      // ❌ void DownloadResults(...) 삭제

      // ✅ 추가: 나이아가라가 읽을 RenderTarget 반환
      UTextureRenderTarget2D* GetPositionTexture() const { return PositionRT; }
      UTextureRenderTarget2D* GetColorTexture() const { return ColorRT; }

  private:
      // ✅ 변경: Buffer 대신 RenderTarget 사용
      UPROPERTY()
      UTextureRenderTarget2D* PositionRT;  // 32x32 RGBA16F

      UPROPERTY()
      UTextureRenderTarget2D* ColorRT;     // 32x32 RGBA16F

      FRWBuffer BytecodeBuffer;
      FRWBuffer ConstantBuffer;

      int32 ParticleCount;
      int32 TextureSize;  // sqrt(ParticleCount) 반올림
  };
  ```

- [ ] RHI 커맨드로 Compute Shader 디스패치
  ```cpp
  void UParticleComputeComponent::ExecuteSimulation(float DeltaTime)
  {
      ENQUEUE_RENDER_COMMAND(ParticleSimCommand)(
          [this, DeltaTime](FRHICommandListImmediate& RHICmdList)
          {
              // PositionRT, ColorRT의 UAV 가져오기
              FRHIUnorderedAccessView* PositionUAV = PositionRT->GetRenderTargetResource()->GetTextureUAV();
              FRHIUnorderedAccessView* ColorUAV = ColorRT->GetRenderTargetResource()->GetTextureUAV();

              // Shader 파라미터 바인딩
              // DispatchComputeShader(TextureSize, TextureSize, 1)
          });
  }
  ```

**산출물**:
- `ParticleSimulation.usf` (RenderTarget 출력)
- `ParticleComputeComponent.h/cpp` (RenderTarget 관리)
- 하드코딩된 테스트 바이트코드

**⚠️ Phase Alpha-1 핵심 검증 사항**:
- [ ] RenderTarget2D가 Compute Shader에서 정상적으로 쓰기 가능한가?
- [ ] 1000개 파티클 (32x32 텍스처) 처리 시간 측정 (목표: <1ms)
- [ ] GPU Profiler로 Compute Shader 성능 확인

---

### Phase Alpha-2: 플레이트 + 나이아가라 통합 (3-4일)

#### 목표
플레이트 위에서 Compute Shader 결과를 나이아가라로 시각화

#### 구현 항목

**1. 플레이트 시스템** (1일)
- [ ] `APlateActor` 클래스 생성
  ```cpp
  UCLASS()
  class APlateActor : public AActor
  {
      GENERATED_BODY()

  public:
      APlateActor();

  private:
      UPROPERTY(VisibleAnywhere)
      UStaticMeshComponent* PlateMesh;  // 500×350 평면 (RootComponent)

      UPROPERTY(VisibleAnywhere)
      UParticleComputeComponent* ComputeComponent;

      UPROPERTY(VisibleAnywhere)
      UNiagaraComponent* ParticleSystem;  // PlateMesh에 상대 위치로 부착

      UPROPERTY(EditAnywhere, Category = "Particle")
      float ParticleHeightOffset = 50.0f;  // 플레이트 중심에서 Z축 오프셋(cm)
  };
  ```

- [ ] 배치 및 위치 설정
  ```cpp
  // APlateActor 생성자
  APlateActor::APlateActor()
  {
      PlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateMesh"));
      RootComponent = PlateMesh;

      ParticleSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ParticleSystem"));
      ParticleSystem->SetupAttachment(PlateMesh);
      // 플레이트 중심에서 Z축 위로 오프셋
      ParticleSystem->SetRelativeLocation(FVector(0, 0, ParticleHeightOffset));
  }
  ```
  - 맵에서 월드 고정 위치에 배치
  - 나이아가라는 플레이트 로컬 공간에서 **+Z 방향**으로 오프셋
  - 기즈모 없음 (위치 조작 불가)

**2. 나이아가라 시스템 설정** (1-2일)
- [ ] `NS_TestParticles` 생성
  - 단일 이미터
  - Sprite 파티클
  - Emitter State: Infinite (계속 유지)

- [ ] ✅ **Grid 2D Collection 방식으로 변경**
  - User Parameters 설정:
    - `User.PositionTexture` (Texture2D) ← Compute Shader 출력
    - `User.ColorTexture` (Texture2D) ← Compute Shader 출력
    - `User.TextureSize` (Int, 예: 32)

- [ ] 파티클 Spawn
  - Spawn Rate: 0
  - Spawn Burst: ParticleCount (한 번만, 1024개)

- [ ] 파티클 Update (Texture Sample)
  ```
  // Grid 2D Collection 사용
  float2 UV = float2(Particle.ID % TextureSize, Particle.ID / TextureSize) / TextureSize;
  Particle.Position = SampleTexture2D(User.PositionTexture, UV).xyz;
  Particle.Color = SampleTexture2D(User.ColorTexture, UV).rgb;
  ```

> **💡 Gemini 제안 핵심**:
> - ❌ 기존: `SetNiagaraArrayVector()` 사용 (CPU 경유)
> - ✅ 변경: Texture Sample로 GPU에서 직접 읽기

**2. 홀로그램 머티리얼** (1일)
- [ ] `M_SimpleHologram` 생성
  ```
  - Base Color: Particle Color * 2.0 (밝게)
  - Emissive: Particle Color * 5.0
  - Opacity: 0.7
  - Blend Mode: Translucent
  - Shading Model: Unlit
  ```

**3. C++ 통합** (1일)
- [ ] `APlateActor`에 파이프라인 구현
  ```cpp
  void APlateActor::BeginPlay()
  {
      Super::BeginPlay();

      // 바이트코드 생성 (Lua 컴파일러 없이 직접 작성)
      TArray<uint32> Bytecode = GenerateCircleMotionBytecode();

      ComputeComponent->InitializeBuffers(1000);
      ComputeComponent->UploadBytecode(Bytecode);

      // ✅ 변경: RenderTarget을 나이아가라에 전달 (BeginPlay에서 한 번만)
      ParticleSystem->SetTextureObject(TEXT("User.PositionTexture"),
                                       ComputeComponent->GetPositionTexture());
      ParticleSystem->SetTextureObject(TEXT("User.ColorTexture"),
                                       ComputeComponent->GetColorTexture());
      ParticleSystem->SetNiagaraVariableInt(TEXT("User.TextureSize"), 32);

      ParticleSystem->Activate();
  }

  void APlateActor::Tick(float DeltaTime)
  {
      Super::Tick(DeltaTime);

      // ✅ 변경: Compute Shader만 실행 (CPU 다운로드/업로드 없음!)
      ComputeComponent->ExecuteSimulation(DeltaTime);

      // ❌ 삭제: DownloadResults, SetNiagaraArrayVector 호출 제거
      // 나이아가라가 자동으로 RenderTarget을 읽음
  }
  ```

> **🚀 성능 개선 핵심**:
> - 기존: 매 프레임 GPU→CPU→GPU 왕복 (심각한 병목)
> - 변경: GPU에서 모든 작업 완료 (Zero-Copy)

**산출물**:
- `PlateActor.h/cpp`
- `NS_TestParticles.uasset`
- `M_SimpleHologram.uasset`

---

### Phase Alpha-3: Lua → 바이트코드 컴파일러 (선택, 2-3일)

> **중요**: 이 Phase는 "Lua 스크립트 → 바이트코드" 변환 파이프라인 검증용입니다.
>
> - ❌ UI에서 사용자가 코드를 입력하는 기능 없음
> - ✅ C++ 코드에 하드코딩된 Lua 스크립트를 바이트코드로 컴파일
> - ✅ 시연이 급하면 이 Phase를 건너뛰고 바이트코드를 직접 작성 가능

#### 목표
개발자가 작성한 Lua 코드를 바이트코드로 변환하는 컴파일러 구현

#### 구현 항목

**1. Lua 라이브러리 선택** (0.5일)
- [ ] 옵션 1: **LuaJIT 직접 통합** (추천)
  - `ThirdParty/LuaJIT` 폴더에 복사
  - `Build.cs`에 include/lib 경로 추가

- [ ] 옵션 2: LuaMachine 플러그인
  - 더 빠르지만 커스텀 제어 어려움

**2. 최소 컴파일러 구현** (1-2일)
- [ ] `FLuaBytecodeCompiler` 클래스
  ```cpp
  class FLuaBytecodeCompiler
  {
  public:
      bool Compile(const FString& LuaCode,
                   TArray<uint32>& OutBytecode,
                   TArray<float>& OutConstants);

  private:
      void ParseAST(lua_State* L);
      void EmitOpCode(EOpCode Op);
      void EmitConstant(float Value);
  };
  ```

- [ ] 지원할 Lua 코드 패턴
  ```lua
  -- 매우 단순한 형태만
  function update(id, time)
      local x = math.cos(time + id) * 100
      local y = math.sin(time + id) * 100
      local z = 0
      return x, y, z
  end
  ```

- [ ] AST → 바이트코드 변환
  - Lua C API로 AST 순회
  - 각 노드를 OpCode로 변환
  - 상수 풀 생성

**3. 통합 테스트** (0.5일)
- [ ] 하드코딩된 Lua 문자열을 컴파일
- [ ] 바이트코드가 Alpha-1과 동일한 결과 생성하는지 확인

**산출물**:
- `LuaBytecodeCompiler.h/cpp`
- Lua 통합 테스트 코드

---

## 시연 시나리오

### 최종 결과물
1. 맵에 `APlateActor` 배치 (월드 고정 위치)
2. 플레이 시작
3. **플레이트 중심 위쪽(+Z)에서** 1000개의 파티클이 원형으로 회전
4. 홀로그램 스타일로 렌더링
5. 색상이 시간에 따라 변화

### 데모 코드 (하드코딩)

**옵션 1: Lua 스크립트 하드코딩** (Phase Alpha-3 완료 시)
```cpp
// PlateActor.cpp의 BeginPlay()
void APlateActor::BeginPlay()
{
    Super::BeginPlay();

    // 하드코딩된 Lua 스크립트
    FString LuaCode = TEXT(R"(
        function update(id, time)
            local angle = time + id * 0.1
            local radius = 100
            local x = math.cos(angle) * radius
            local y = math.sin(angle) * radius
            local z = math.sin(time) * 20

            local r = (math.sin(time + id) + 1) * 0.5
            local g = (math.cos(time + id) + 1) * 0.5
            local b = 0.8

            return x, y, z, r, g, b
        end
    )");

    // Lua → 바이트코드 컴파일
    TArray<uint32> Bytecode;
    TArray<float> Constants;
    LuaCompiler->Compile(LuaCode, Bytecode, Constants);

    ComputeComponent->InitializeBuffers(1000);
    ComputeComponent->UploadBytecode(Bytecode, Constants);
    ParticleSystem->Activate();
}
```

**옵션 2: 바이트코드 직접 작성** (Phase Alpha-3 건너뛸 경우)
```cpp
void APlateActor::BeginPlay()
{
    Super::BeginPlay();

    // 직접 작성한 바이트코드 (원형 모션)
    TArray<uint32> Bytecode = {
        // x = cos(time + id) * 100
        OP_PUSH_VAR, VAR_TIME,
        OP_PUSH_VAR, VAR_ID,
        OP_ADD,
        OP_COS,
        OP_PUSH_CONST, 0,  // 100.0f
        OP_MUL,
        OP_STORE_POS_X,

        // y = sin(time + id) * 100
        OP_PUSH_VAR, VAR_TIME,
        OP_PUSH_VAR, VAR_ID,
        OP_ADD,
        OP_SIN,
        OP_PUSH_CONST, 0,  // 100.0f
        OP_MUL,
        OP_STORE_POS_Y,

        // z = 0
        OP_PUSH_CONST, 1,  // 0.0f
        OP_STORE_POS_Z,

        OP_HALT
    };

    TArray<float> Constants = { 100.0f, 0.0f };

    ComputeComponent->InitializeBuffers(1000);
    ComputeComponent->UploadBytecode(Bytecode, Constants);
    ParticleSystem->Activate();
}
```

---

## 기술 스택 (최소)

### 필수
1. **Unreal Engine 5**
   - C++ (Actor, Component)
   - RHI (Compute Shader 디스패치)

2. **HLSL**
   - Compute Shader
   - 스택 기반 인터프리터

3. **나이아가라**
   - Array Data Interface
   - 기본 Sprite 파티클

### 선택 (시간 있을 경우)
4. **Lua**
   - LuaJIT
   - C API 기본 사용법

---

## 개발 일정

| Day | 작업 내용 | 산출물 |
|-----|----------|--------|
| 1 | OpCode 정의, 하드코딩 바이트코드 | OpCode enum, 테스트 바이트코드 |
| 2-3 | Compute Shader 구현 | ParticleSimulation.usf |
| 4 | C++ GPU 리소스 관리 | ParticleComputeComponent |
| 5 | 플레이트 Actor 생성 | PlateActor.h/cpp |
| 6 | 나이아가라 시스템 | NS_TestParticles |
| 7 | 홀로그램 머티리얼 | M_SimpleHologram |
| 8 | 통합 및 테스트 | 시연 가능 ✅ |
| 9-11 (선택) | Lua 컴파일러 | LuaBytecodeCompiler |

**총 기간**:
- **최소 8일** (Phase 1-2만, 바이트코드 직접 작성)
- **최대 11일** (Phase 1-3, Lua 컴파일러 포함)

---

## 성공 기준

### 필수 (Alpha 통과)
- [x] 1000개 파티클이 GPU에서 계산됨
- [x] 나이아가라로 정상 렌더링
- [x] 크래시 없이 60fps 유지
- [x] 원형 모션 정상 작동

### 보너스 (시간 남으면)
- [ ] Lua 코드 → 바이트코드 변환 작동
- [ ] 2-3개 다른 모션 패턴 (나선, 웨이브 등)
- [ ] 색상 애니메이션

---

## 위험 요소 및 대응

### 1. Compute Shader 디버깅 어려움
**위험**: GPU 코드는 디버깅이 어려움
**대응**:
- CPU 버전 인터프리터 먼저 작성
- 결과를 CPU/GPU 비교
- RenderDoc 사용

### 2. RHI API 복잡도
**위험**: Unreal RHI API가 낯설 수 있음
**대응**:
- 기존 Compute Shader 예제 참고
- 엔진 소스 코드 읽기 (FComputeShaderUtils)
- 커뮤니티 예제 활용

### 3. ~~나이아가라 Array Data Interface 이슈~~ (해결됨)
**위험**: ~~대량 데이터 전송 시 퍼포먼스 문제~~ → **RenderTarget 방식으로 해결**
**대응**:
- ✅ RenderTarget2D 사용으로 CPU 경유 제거
- ✅ GPU-to-GPU 직접 전송으로 병목 해소
- ✅ 5,000개 이상 파티클도 처리 가능 예상

---

## 다음 단계 (Alpha 완료 후)

Alpha 시연 성공 시:
1. **Beta 단계**: UI 추가, 플레이트 시스템
2. **RC 단계**: 멀티플레이, 보안
3. **v1.0**: 폴리싱, 최적화

Alpha 실패 시:
- 병목 지점 분석
- 아키텍처 재설계
- 대안 접근 (Blueprint 노드 기반 등)

---

## 참고 자료

### Compute Shader in UE5
- [Unreal Engine Compute Shader 예제](https://github.com/search?q=unreal+compute+shader)
- UE5 소스: `Engine/Shaders/Private/` 예제들
- `FComputeShaderUtils::Dispatch()` 사용법

### 나이아가라 Array Data Interface
- [공식 문서](https://docs.unrealengine.com/5.0/en-US/data-interfaces-in-niagara-for-unreal-engine/)
- `UNiagaraComponent::SetNiagaraArrayVector()` API

### 바이트코드 인터프리터
- [Crafting Interpreters - Bytecode VM](https://craftinginterpreters.com/a-bytecode-virtual-machine.html)

---

## 즉시 시작 가능한 작업 (수정본)

1. **지금 당장**:
   - `Source/MYP/Particle/` 폴더 생성
   - `ParticleOpCodes.h` (OpCode enum 정의)
   - `ParticleComputeComponent.h/cpp` 뼈대 작성
   - `PlateActor.h/cpp` 뼈대 작성

2. **30분 이내**:
   - ✅ **RenderTarget2D 생성 코드 작성** (32x32 RGBA16F)
   - 하드코딩 바이트코드 배열 작성
   - 원형 모션 바이트코드 완성

3. **오늘 내**:
   - Compute Shader 파일 생성 (`ParticleSimulation.usf`)
   - ✅ **RWTexture2D 출력 구조로 작성**
   - 기본 스택 인터프리터 구조 작성
   - PlateActor에 StaticMesh + Niagara 컴포넌트 추가

---

## 요약

**Alpha 버전 핵심**:
- ✅ 플레이트 위에서 파티클 시스템 작동
- ✅ Lua/바이트코드 → GPU Compute Shader → 나이아가라 파이프라인 검증
- ✅ 8-11일 내 시연 가능
- ❌ UI, 기즈모, 멀티플레이, 보안 등은 추후

**다음 단계**: Alpha 성공 시 원본 계획서(particle_system_dev_plan.md)의 Beta 단계로 진행

---

**문서 버전**: Alpha 2.0 (Gemini 검토 반영)
**작성일**: 2024-12-11 (수정: 2024-12-12)
**최종 수정**: 2024-12-12 (아키텍처 개선 - Zero-Copy GPU 파이프라인)
**목표**: 8-11일 내 시연 가능한 프로토타입 완성

---

## 🔄 주요 변경 사항 (v2.0)

### ⚠️ 치명적 성능 병목 해결
**기존 (v1.1)**:
```
Compute Shader → GPU→CPU 복사 → TArray 변환 → CPU→GPU 업로드 → Niagara
                  ^^^^^^^^^^^^^^^^   (PCIe 병목, 프레임 드랍 확정)
```

**개선 (v2.0)**:
```
Compute Shader → RenderTarget2D (GPU 메모리 유지) → Niagara 직접 읽기
                  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                  Zero-Copy, 60fps 달성 가능
```

### 핵심 API 변경
- ❌ `DownloadResults()` 제거
- ❌ `SetNiagaraArrayVector()` 제거
- ✅ `RWTexture2D<float4>` 사용
- ✅ `SetTextureObject()` 사용 (BeginPlay에서 한 번만)

### 검증 근거
- Gemini AI Technical Consultant 검토 (ParticleSys_Plan_Review.md)
- Tech_Feasibility_Analysis.md의 "데이터 파이프라인 병목" 섹션 참조

---

**Let's build it! 🚀**

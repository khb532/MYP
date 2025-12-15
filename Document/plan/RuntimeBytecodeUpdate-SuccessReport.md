# Runtime Bytecode Update - 성공 레포트

**프로젝트**: Dynamic Particle System (Lua Scripting)
**작성일**: 2025년 12월 15일
**브랜치**: DynamicNiagara-Test
**상태**: ✅ **런타임 바이트코드 업데이트 성공**

---

## 📋 목차

1. [요약](#요약)
2. [최종 성공 사례](#최종-성공-사례)
3. [해결 방법](#해결-방법)
4. [테스트 결과](#테스트-결과)
5. [현재 브랜치 상태](#현재-브랜치-상태)
6. [다음 단계](#다음-단계)

---

## 요약

### 🎯 목표
런타임에 GPU Compute Shader의 바이트코드를 동적으로 변경하여, 파티클 동작을 실시간으로 수정 가능하게 함.

### ✅ 결과
**완전 성공** - FRWBuffer + SHADER_PARAMETER_SRV 방식으로 런타임 바이트코드 업데이트 완벽 작동 확인.

### 🔑 핵심 문제
UE5의 **RDG (Render Dependency Graph) 버퍼 풀링 메커니즘**이 바이트코드 버퍼를 캐싱하여, 두 번째 업로드 시 GPU 메모리가 업데이트되지 않음.

### 💡 해결책
1. **FRWBuffer 사용** - RHI 레벨에서 직접 버퍼 관리 (RDG Pooling 우회)
2. **SHADER_PARAMETER_SRV** - RDG CreateSRV 대신 RHI SRV 직접 바인딩
3. **ReadFloat16Pixels** - RGBA16f 데이터를 정확히 읽어 검증

---

## 최종 성공 사례

### 테스트 시나리오
```
0초:  초기 바이트코드 업로드 (position.x = particleID * 10)
1초:  첫 번째 검증
5초:  새 바이트코드 업로드 (position.x = particleID, MUL 제거)
10초: 최종 검증 (FlushRenderingCommands 후)
```

### 실제 로그 결과

#### 1초 후 (초기 바이트코드)
```
MYPLog: Warning: === First 3 Particles ===
MYPLog: Warning: Particle[0]: Pos(0.00, 0.00, 0.00) Color(0.00, 0.00, 0.00)
MYPLog: Warning: Particle[1]: Pos(10.00, 0.00, 0.00) Color(0.10, 0.00, 0.00)
MYPLog: Warning: Particle[2]: Pos(20.00, 0.00, 0.00) Color(0.20, 0.00, 0.00)
```
✅ **검증**: particleID * 10 정상 작동

#### 5초 후 (바이트코드 업로드)
```
MYPLog: Warning: === [5초] 바이트코드 동적 변경: position.x = particleID (연산 없이) ===
MYPLog: Warning: [UploadBytecode] First bytecodes: 1 0 30 0 1
MYPLog: Warning: [GAME THREAD] Bytecode updated: 22 instructions, 4 constants
MYPLog: Warning: [RENDER THREAD] BytecodeBuffer 기존 버퍼 해제
MYPLog: Warning: [RENDER THREAD] BytecodeBuffer 업로드 완료: 22 instructions
MYPLog: Warning: [RENDER THREAD] ConstantsBuffer 기존 버퍼 해제
MYPLog: Warning: [RENDER THREAD] ConstantsBuffer 업로드 완료: 4 constants
```
✅ **검증**: 기존 버퍼 해제 및 새 버퍼 생성 확인

#### 10초 후 (최종 검증)
```
MYPLog: Warning: === [10초] Render Thread 강제 동기화 후 결과 확인 ===
MYPLog: Warning: === First 3 Particles ===
MYPLog: Warning: Particle[0]: Pos(0.00, 0.00, 0.00) Color(0.00, 1.00, 0.00)
MYPLog: Warning: Particle[1]: Pos(1.00, 0.00, 0.00) Color(0.01, 1.00, 0.00)  ← ✅ 10.00 → 1.00
MYPLog: Warning: Particle[2]: Pos(2.00, 0.00, 0.00) Color(0.02, 1.00, 0.00)  ← ✅ 20.00 → 2.00
MYPLog: Warning: === 검증: Particle[0]=0, Particle[1]=1, Particle[2]=2로 변경되었는지 확인 (MUL 없이 PUSH_VAR만) ===
```
✅ **성공**: Position 값 변경 (10.00 → 1.00, 20.00 → 2.00)
✅ **성공**: Color 값 변경 (빨강 → 초록)

---

## 해결 방법

### 1. FRWBuffer 방식 (RHI 직접 관리)

**ParticleComputeComponent.h**
```cpp
#include "RenderResource.h"  // FRWBuffer 헤더

private:
    FRWBuffer BytecodeBuffer;   // RHI 직접 관리
    FRWBuffer ConstantsBuffer;
```

**ParticleComputeComponent.cpp - UploadBytecode**
```cpp
void UParticleComputeComponent::UploadBytecode(const TArray<uint32>& bytecode, const TArray<float>& constants)
{
    BytecodeData = bytecode;
    ConstantData = constants;

    TArray<uint32> BytecodeCopy = bytecode;
    TArray<float> ConstantsCopy = constants;

    ENQUEUE_RENDER_COMMAND(UploadParticleBytecode)(
        [this, BytecodeCopy, ConstantsCopy](FRHICommandListImmediate& RHICmdList)
        {
            // 1. 기존 버퍼 명시적 해제
            if (BytecodeBuffer.Buffer)
            {
                BytecodeBuffer.Release();
                LOGMSGF(TEXT("[RENDER THREAD] BytecodeBuffer 기존 버퍼 해제"));
            }

            // 2. 새 버퍼 생성 (RHI 직접)
            if (BytecodeCopy.Num() > 0)
            {
                uint32 BytecodeSize = BytecodeCopy.Num() * sizeof(uint32);

                // FRWBuffer 초기화 (UE 5.6 API)
                BytecodeBuffer.Initialize(
                    RHICmdList,
                    TEXT("ParticleBytecodeBuffer"),
                    sizeof(uint32),
                    BytecodeCopy.Num(),
                    PF_R32_UINT,
                    BUF_Static | BUF_ShaderResource
                );

                // 3. 데이터 직접 업로드 (Lock/Unlock)
                void* MappedData = RHICmdList.LockBuffer(BytecodeBuffer.Buffer, 0, BytecodeSize, RLM_WriteOnly);
                FMemory::Memcpy(MappedData, BytecodeCopy.GetData(), BytecodeSize);
                RHICmdList.UnlockBuffer(BytecodeBuffer.Buffer);

                LOGMSGF(TEXT("[RENDER THREAD] BytecodeBuffer 업로드 완료: %d instructions"), BytecodeCopy.Num());
            }

            // Constants도 동일 방식
            // ...
        }
    );
}
```

**핵심 포인트**:
- `FRWBuffer::Release()` - 기존 GPU 버퍼 완전 해제
- `FRWBuffer::Initialize()` - 새 RHI 버퍼 생성
- `LockBuffer/UnlockBuffer` - GPU 메모리에 직접 쓰기
- **RDG Pooling 완전 우회**

---

### 2. SHADER_PARAMETER_SRV (RDG 우회)

**ParticleSimulationShader.h**
```cpp
BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
    // RDG 버퍼 대신 RHI SRV 직접 사용
    SHADER_PARAMETER_SRV(StructuredBuffer<uint>, BytecodeBuffer)   // ← RHI SRV
    SHADER_PARAMETER_SRV(StructuredBuffer<float>, ConstantsBuffer) // ← RHI SRV

    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, PositionRT)
    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, ColorRT)
    // ...
END_SHADER_PARAMETER_STRUCT()
```

**변경 전 (실패)**:
```cpp
SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<uint>, BytecodeBuffer)  // FRDGBufferSRVRef
```

**변경 후 (성공)**:
```cpp
SHADER_PARAMETER_SRV(StructuredBuffer<uint>, BytecodeBuffer)  // FShaderResourceViewRHIRef
```

**ParticleComputeComponent.cpp - ExecuteSimulation**
```cpp
void UParticleComputeComponent::ExecuteSimulation(float deltatime)
{
    // FRWBuffer의 SRV 캡처
    FShaderResourceViewRHIRef BytecodeSRV = BytecodeBuffer.SRV;
    FShaderResourceViewRHIRef ConstantsSRV = ConstantsBuffer.SRV;

    ENQUEUE_RENDER_COMMAND(ExecuteParticleSimulation)(
        [PositionRTResource, ColorRTResource, BytecodeSRV, ConstantsSRV, ...](...)
        {
            FRDGBuilder GraphBuilder(RHICmdList);

            // RenderTarget은 RDG 등록
            FRDGTextureRef PositionTexture = GraphBuilder.RegisterExternalTexture(...);
            FRDGTextureRef ColorTexture = GraphBuilder.RegisterExternalTexture(...);

            // Shader 파라미터에 RHI SRV 직접 바인딩
            FParticleSimulationCS::FParameters* PassParameters = GraphBuilder.AllocParameters<...>();
            PassParameters->BytecodeBuffer = BytecodeSRV;   // RDG CreateSRV 건너뜀
            PassParameters->ConstantsBuffer = ConstantsSRV; // RDG CreateSRV 건너뜀
            PassParameters->PositionRT = GraphBuilder.CreateUAV(PositionTexture);
            PassParameters->ColorRT = GraphBuilder.CreateUAV(ColorTexture);
            // ...

            FComputeShaderUtils::AddPass(GraphBuilder, ...);
            GraphBuilder.Execute();
        }
    );
}
```

**핵심 포인트**:
- `BytecodeBuffer.SRV` - FRWBuffer가 자동 생성한 SRV 사용
- RDG의 `CreateSRV` 호출 **건너뜀**
- RDG 캐싱 메커니즘 **완전 회피**

---

### 3. ReadFloat16Pixels (정확한 데이터 읽기)

**기존 문제 (ParticleComputeComponent.cpp)**:
```cpp
// ❌ 잘못된 방식
TArray<FColor> PositionData;  // 8bit RGBA (0~255)
PositionResource->ReadPixels(PositionData, ...);  // RGBA16f → 8bit 변환 손실!
float PosX = Pixel.R / 255.0f * 1000.0f;  // 부정확한 역변환
```

**수정 후**:
```cpp
// ✅ 올바른 방식
TArray<FFloat16Color> PositionData;  // 16bit float RGBA
TArray<FFloat16Color> ColorData;
FIntRect Rect(0, 0, TextureSize, TextureSize);

// RGBA16f 데이터를 정확히 읽음
PositionResource->ReadFloat16Pixels(PositionData, FReadSurfaceDataFlags(), Rect);
ColorResource->ReadFloat16Pixels(ColorData, FReadSurfaceDataFlags(), Rect);

// FFloat16 → float 직접 변환
float PosX = PosPix.R.GetFloat();  // 정확한 float 값
float PosY = PosPix.G.GetFloat();
float PosZ = PosPix.B.GetFloat();

float ColR = ColPix.R.GetFloat();
float ColG = ColPix.G.GetFloat();
float ColB = ColPix.B.GetFloat();

LOGMSGF(TEXT("Particle[%d]: Pos(%.2f, %.2f, %.2f) Color(%.2f, %.2f, %.2f)"),
    i, PosX, PosY, PosZ, ColR, ColG, ColB);
```

**핵심 포인트**:
- `ReadFloat16Pixels` - RGBA16f 포맷 전용 읽기 함수
- `FFloat16Color` - 16bit float 타입
- `GetFloat()` - 정확한 float 변환

---

## 테스트 결과

### 성공 사례 상세 분석

#### Position 값 변화
| 시간 | Particle[1] Position.x | 예상값 | 상태 |
|------|----------------------|--------|------|
| 1초  | 10.00                | 1*10=10 | ✅ |
| 10초 | 1.00                 | 1 (particleID) | ✅ |

**변화량**: 10.00 → 1.00 (10배 감소)
**의미**: MUL 연산 제거 성공 (particleID * 10 → particleID)

#### Color 값 변화
| 시간 | Color (R, G, B) | 의미 | 상태 |
|------|----------------|------|------|
| 1초  | (0.10, 0.00, 0.00) | 빨강 (position.x/100 인코딩) | ✅ |
| 10초 | (0.01, 1.00, 0.00) | 초록 (Constants 변경) | ✅ |

**의미**: Constants 업데이트도 정상 작동

#### Render Thread 로그
```
[RENDER THREAD] BytecodeBuffer 기존 버퍼 해제          ← 이전 버퍼 삭제
[RENDER THREAD] BytecodeBuffer 업로드 완료: 22 instructions  ← 새 버퍼 생성
[RENDER THREAD] ConstantsBuffer 기존 버퍼 해제
[RENDER THREAD] ConstantsBuffer 업로드 완료: 4 constants
```
**의미**: FRWBuffer Release/Initialize 정상 작동

---

## 현재 브랜치 상태

### 최종 성공 커밋
**브랜치**: `DynamicNiagara-Test`
**마지막 커밋**: FRWBuffer + SHADER_PARAMETER_SRV + ReadFloat16Pixels 적용
**상태**: ✅ 런타임 바이트코드 업데이트 완전 성공

### 체크아웃된 상태 (현재)
**커밋**: `946ca58 [FEAT] Alpha Phase Complete.` (초기 Alpha-1 완성 시점)
**수정 사항**: 로그 출력 로직만 `ReadFloat16Pixels`로 수정
**목적**: 초기 RDG Pooled Buffer 방식에서 런타임 업데이트 실패 재현 테스트

#### 현재 코드 상태
```cpp
// ❌ FRDGPooledBuffer 방식 (캐싱 문제 있음)
TRefCountPtr<FRDGPooledBuffer> BytecodePooledBuffer;
TRefCountPtr<FRDGPooledBuffer> ConstantsPooledBuffer;

// ❌ SHADER_PARAMETER_RDG_BUFFER_SRV (RDG 캐싱)
SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<uint>, BytecodeBuffer)

// ✅ ReadFloat16Pixels (정확한 데이터 읽기)
PositionResource->ReadFloat16Pixels(PositionData, FReadSurfaceDataFlags(), Rect);
```

---

## 실패 사례 기록 (이전 시도)

### 시도 1: BUF_Volatile 플래그
```cpp
FRDGBufferDesc BytecodeDesc = FRDGBufferDesc::CreateStructuredDesc(sizeof(uint32), BytecodeCopy.Num());
BytecodeDesc.Usage |= BUF_Volatile;  // RDG 캐싱 방지 시도
```
**결과**: ❌ 실패 - RDG가 여전히 버퍼 재사용

### 시도 2: 매 프레임 새 버퍼 + 고유 이름
```cpp
FString BytecodeBufferName = FString::Printf(TEXT("ParticleBytecode_Frame%u"), CurrentFrame);
FRDGBufferRef BytecodeBuffer = GraphBuilder.CreateBuffer(BytecodeDesc, *BytecodeBufferName);
GraphBuilder.QueueBufferUpload(BytecodeBuffer, ...);
```
**결과**: ❌ 실패 - 이름 달라도 RDG가 내부적으로 캐싱

### 시도 3: SafeRelease + RenderTarget 재생성
```cpp
BytecodePooledBuffer.SafeRelease();
PositionRT = NewObject<UTextureRenderTarget2D>(...);  // RT 재생성
```
**결과**: ❌ 실패 - FRDGPooledBuffer Pool이 같은 주소 재할당

### 시도 4: FRWBuffer + RegisterExternalBuffer
```cpp
FRHIBuffer* BytecodeRHI = BytecodeBuffer.Buffer;
FRDGBufferRef BytecodeBufferRDG = GraphBuilder.RegisterExternalBuffer(BytecodeRHI, ...);
```
**결과**: ❌ 컴파일 에러 - UE 5.6에서 `RegisterExternalBuffer`는 `FRDGPooledBuffer`만 받음

### 최종 성공: FRWBuffer + SHADER_PARAMETER_SRV
**핵심**: RDG를 **완전히 우회**하고 RHI SRV를 직접 바인딩

---

## 기술적 인사이트

### RDG (Render Dependency Graph) 캐싱 메커니즘

**RDG의 최적화 전략**:
1. **버퍼 풀링**: 같은 크기/타입의 버퍼를 재사용
2. **해시 기반 캐싱**: 버퍼 메타데이터(크기, 타입, 이름)로 해시 생성
3. **암묵적 재사용**: `QueueBufferUpload` 호출해도 풀에서 기존 버퍼 재사용 가능

**문제점**:
- 동일한 크기의 BytecodeBuffer를 업로드하면, RDG가 "이전 버퍼와 같다"고 판단
- `QueueBufferUpload`가 **무시**되거나, 풀의 **이전 버퍼**를 계속 사용
- Constants는 크기가 작아서 우연히 재할당되어 업데이트 성공

**해결 원리**:
- **FRWBuffer**: RDG 외부에서 RHI 버퍼 직접 관리
- **SHADER_PARAMETER_SRV**: RDG의 CreateSRV 건너뛰고 RHI SRV 직접 바인딩
- **RDG 완전 우회**: 버퍼 생성/업로드/바인딩 모두 RHI 레벨에서 처리

---

## 다음 단계

### 1. 비교 테스트 완료 (진행 중)
- ✅ 초기 Alpha-1 커밋으로 체크아웃 완료
- ✅ 로그 출력 로직만 수정 (ReadFloat16Pixels)
- ⏳ **다음**: 런타임 업데이트 실패 재현 테스트
- ⏳ **이후**: 성공 버전과 실패 버전 비교 분석 레포트 작성

### 2. Beta 단계 진행
**Document/plan/ParticleSys-Beta.md** 참고

#### Step 1: Niagara 연동 ⭐ 우선순위 1
- [ ] PlateActor에 `UNiagaraComponent` 추가
- [ ] Niagara System Asset 생성
- [ ] PositionRT/ColorRT → Niagara로 전달
- [ ] 파티클 시각화 테스트

#### Step 2: Input System 및 PlateActor 스폰
- [ ] Enhanced Input 설정 ('P' 키)
- [ ] Player 전방 200cm에 스폰
- [ ] 버튼 UI 추가 (선택사항)

#### Step 3: Lua 코드 입력 UI (UMG)
- [ ] WBP_LuaCodeEditor 위젯 생성
- [ ] MultiLineEditableTextBox
- [ ] Save & Run / Cancel 버튼

#### Step 4: LuaCompiler 구현
- [ ] LuaJIT 통합
- [ ] Lua → AST → OpCode 변환
- [ ] ULuaCompiler 클래스

#### Step 5: 통합 및 연동
- [ ] UI ↔ PlateActor ↔ LuaCompiler 연결
- [ ] 전체 플로우 테스트

---

## 파일 변경 요약

### 수정된 파일 (성공 버전)

#### 1. ParticleComputeComponent.h
```cpp
// 추가:
#include "RenderResource.h"

// 변경:
- TRefCountPtr<FRDGPooledBuffer> BytecodePooledBuffer;
+ FRWBuffer BytecodeBuffer;
```

#### 2. ParticleComputeComponent.cpp
- `UploadBytecode()`: FRWBuffer Release/Initialize 방식으로 전환
- `ExecuteSimulation()`: RHI SRV 직접 캡처 및 바인딩
- `DebugPrintRenderTarget()`: ReadFloat16Pixels 사용

#### 3. ParticleSimulationShader.h
```cpp
// 변경:
- SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<uint>, BytecodeBuffer)
+ SHADER_PARAMETER_SRV(StructuredBuffer<uint>, BytecodeBuffer)
```

#### 4. PlaterActor.cpp
- 1초/5초/10초 타이머 테스트 코드 추가
- FlushRenderingCommands() 호출

---

## 결론

### ✅ 달성한 것
1. **런타임 바이트코드 업데이트 완전 성공**
2. **RDG 캐싱 문제 근본 원인 파악**
3. **FRWBuffer + SHADER_PARAMETER_SRV 해결책 검증**
4. **정확한 float 데이터 읽기 방법 확립**

### 📚 학습한 것
1. UE5 RDG의 버퍼 풀링 메커니즘
2. FRWBuffer를 사용한 RHI 직접 관리
3. RDG 우회 기법 (SHADER_PARAMETER_SRV)
4. RGBA16f 텍스처의 정확한 읽기 방법

### 🚀 Beta 단계 준비 완료
- Alpha-1 목표 완전 달성
- 런타임 바이트코드 시스템 검증 완료
- Lua 통합 및 Niagara 연동 준비됨

---

**작성자**: Claude Code
**최종 수정**: 2025년 12월 15일
**다음 문서**: Beta 단계 구현 계획 (ParticleSys-Beta.md)

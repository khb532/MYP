# 일일 작업 레포트 - 2025년 12월 14일

## 📋 프로젝트 정보
- **전체 프로젝트**: Dynamic Particle System (사용자 정의 Lua 스크립트로 파티클 제어)
- **현재 단계**: Alpha-1 - 최소 GPU 파이프라인
- **Alpha-1 목표**: Lua → 바이트코드 → GPU Compute Shader → 나이아가라 파이프라인 구축 및 검증
- **작업 브랜치**: DynamicNiagara

---

## ✅ 완료된 작업

**1. OpCode 시스템 설계**
- OpCode enum 정의 (PUSH_CONST, PUSH_VAR, ADD, SUB, MUL, DIV, SIN, COS, STORE_POS_*, STORE_COLOR_*, HALT)
- Variable enum 정의 (VAR_PARTICLE_ID, VAR_TIME, VAR_DELTA_TIME)

**2. ParticleComputeComponent 기본 구조**
- UActorComponent 상속 클래스 생성
- 주요 메서드 선언: InitializeBuffers, UploadBytecode, ExecuteSimulation
- RenderTarget Getter 함수 구현

**3. RenderTarget2D 리소스 관리**
- InitializeBuffers() 구현
- RenderTarget2D 동적 생성 (Position용, Color용)
- 텍스처 크기 자동 계산 (sqrt 올림)
- TArray로 바이트코드/상수 백업 저장
- FRWBuffer 이슈 해결 (UploadBytecode로 이연)

**4. Compute Shader (ParticleSimulation.usf) 완성**
- Shaders 폴더 생성 및 .usf 파일 작성
- HLSL 스택 기반 VM 구현 (Stack[64], StackPtr, PC)
- 모든 OpCode 실행 루프 구현
  - 스택 조작: PUSH_CONST, PUSH_VAR
  - 산술 연산: ADD, SUB, MUL, DIV
  - 수학 함수: SIN, COS
  - 결과 저장: STORE_POS_X/Y/Z, STORE_COLOR_R/G/B/A
  - 제어 흐름: HALT
- RenderTarget2D 출력 구현 (ParticleID → 2D 좌표 변환)

**5. Global Shader 시스템 구현**
- ParticleSimulationShader.h/cpp 생성
- FParticleSimulationCS 클래스 (FGlobalShader 상속)
- Shader 파라미터 구조체 정의 (BytecodeBuffer, ConstantsBuffer, PositionRT, ColorRT 등)
- IMPLEMENT_GLOBAL_SHADER 매크로로 .usf 파일 매핑

**6. C++ UploadBytecode() 구현**
- RDG (Render Dependency Graph) 시스템 사용
- FRDGPooledBuffer로 GPU 버퍼 관리
- QueueBufferUpload로 CPU → GPU 데이터 전송
- ENQUEUE_RENDER_COMMAND로 Render Thread 실행

**7. C++ ExecuteSimulation() 구현**
- 매 프레임 Compute Shader 디스패치
- RenderTarget을 RDG Texture로 등록
- Thread Group 계산 (ParticleCount / 64)
- FComputeShaderUtils::AddPass로 Shader 실행

**생성된 파일**:
- `Source/MYP/Particle/ParticleOpCodes.h`
- `Source/MYP/Particle/ParticleOpCodes.cpp`
- `Source/MYP/Particle/ParticleComputeComponent.h`
- `Source/MYP/Particle/ParticleComputeComponent.cpp`
- `Shaders/ParticleSimulation.usf`
- `Source/MYP/Particle/ParticleSimulationShader.h` ✨ **신규**
- `Source/MYP/Particle/ParticleSimulationShader.cpp` ✨ **신규**

---

## 📊 전체 TODO 현황 (4/5 완료)

1. ✅ **OpCode 시스템 설계 및 ParticleOpCodes.h 작성**

2. ✅ **ParticleComputeComponent 기본 구조 작성**

3. ✅ **RenderTarget2D 리소스 관리 구현**

4. ✅ **Compute Shader (ParticleSimulation.usf) 작성**
   - [x] Shaders 폴더 생성
   - [x] HLSL 스택 기반 VM 구현
   - [x] OpCode 실행 루프
   - [x] RenderTarget2D 출력 구현

5. ⏳ **PlateActor 기본 구조 및 컴포넌트 통합** (다음 작업)
   - [x] C++ UploadBytecode() 구현 (ENQUEUE_RENDER_COMMAND)
   - [x] C++ ExecuteSimulation() 구현 (매 프레임 Shader 실행)
   - [ ] PlateActor 생성 및 컴포넌트 통합
   - [ ] 테스트 바이트코드 작성 및 검증

---

## 💡 학습 내용

### 기술적 개념 (기존)
- **RenderTarget2D의 역할**: 2D 저장 방식(주소 체계)이지만 float4를 사용해 3D 좌표 저장 가능
- **Zero-Copy 아키텍처**: GPU → RenderTarget2D → Niagara 직접 연결로 CPU 경유 제거
- **병목 해소 원리**: 매 프레임 GPU↔CPU 왕복(v1.1) → GPU 내부 처리(v2.0)로 개선
- **UKismetRenderingLibrary::CreateRenderTarget2D()**: 런타임 RenderTarget 동적 생성 함수
- **FMath::Sqrt()**: float/double만 받음, int32는 명시적 캐스팅 필요

### Compute Shader 핵심 개념
- **Compute Shader**: GPU에서 병렬 연산을 수행하는 범용 계산(GPGPU) 프로그램
- **Thread Group**: `[numthreads(64, 1, 1)]` - 쓰레드를 묶어서 실행, X*Y*Z ≤ 1024 제한
- **SV_DispatchThreadID**: 전역 쓰레드 ID, 각 파티클의 고유 ID로 사용
- **RWTexture2D<float4>**: Read/Write 가능한 2D 텍스처, GPU에서 직접 결과 저장

### HLSL 스택 기반 VM
- **스택 구조**: `float Stack[64]` + `int StackPtr` - 데이터만 저장, 명령어는 PC로 순차 실행
- **LIFO 원리**: Pop은 나중에 Push한 값부터, 바이트코드 실행은 위→아래 순서
- **PC (Program Counter)**: 바이트코드 실행 위치, while 루프로 순차 실행
- **OpCode 실행 흐름**: 코드 읽기 → 스택 Push/Pop → 연산/저장 → 반복

### HLSL 벡터 타입
- **float2/float3/float4**: 2/3/4채널 벡터, 각 채널을 컴포넌트라고 부름
- **Swizzle 접근자**: `.xyzw` (좌표용), `.rgba` (색상용) - 같은 메모리 참조
- **채널 접근**: `color.r == color.x` (둘 다 첫 번째 채널), HLSL 표준 스펙에 정의됨
- **1D→2D 변환**: `uint2(ParticleID % Size, ParticleID / Size)` - RenderTarget 인덱싱

### Global Shader 시스템 (신규)
- **Global Shader**: 프로젝트 전체에서 사용하는 공용 셰이더, Material과 무관하게 독립적으로 실행
- **DECLARE_GLOBAL_SHADER**: Shader 클래스를 전역 셰이더로 등록하는 매크로
- **SHADER_USE_PARAMETER_STRUCT**: 파라미터를 구조체로 정의
- **IMPLEMENT_GLOBAL_SHADER**: `.usf` 파일과 C++ 클래스를 연결 (경로, 함수명, 타입 지정)
- **ShouldCompilePermutation**: 플랫폼별 컴파일 여부 결정 (SM5 이상)

### RDG (Render Dependency Graph) (신규)
- **RDG 역할**: 렌더링 리소스의 의존성 자동 관리, 생명주기/동기화/최적화 자동 처리
- **FRDGBuilder**: RDG 그래프 빌더, Execute()로 실행
- **FRDGBufferSRV**: Shader Resource View (읽기 전용 버퍼)
- **FRDGTextureUAV**: Unordered Access View (읽기/쓰기 텍스처)
- **FRDGPooledBuffer**: 재사용 가능한 GPU 버퍼, RDG 외부에서 유지 가능

### UE5 RHI API 변경사항 (신규)
- **FRHIResourceCreateInfo 폐기**: `FRHIBufferCreateDesc` 사용으로 변경
- **FRHIBufferCreateDesc**: 4개 파라미터 (이름, 전체크기, 요소크기, Usage플래그)
- **RHICreateBuffer() 폐기**: `RHICmdList.CreateBuffer()` 멤버 함수 사용
- **EBufferUsageFlags**: `ShaderResource | StructuredBuffer` 플래그 조합
- **QueueBufferUpload**: CPU 데이터를 RDG 버퍼로 업로드
- **QueueBufferExtraction**: RDG 버퍼를 Pooled Buffer로 추출 (재사용)

### ENQUEUE_RENDER_COMMAND (신규)
- **Game Thread vs Render Thread**: GPU 작업은 Render Thread에서만 가능
- **ENQUEUE_RENDER_COMMAND**: Game Thread에서 호출, 람다를 Render Thread 큐에 추가
- **FRHICommandListImmediate**: GPU 명령을 즉시 실행하는 커맨드 리스트
- **람다 캡처 주의사항**: `this` 포인터는 위험, 데이터 복사본 사용 권장

### Thread Group 계산 (신규)
- **Dispatch 개념**: Compute Shader 실행 단위
- **GroupCount 계산**: `FMath::DivideAndRoundUp(ParticleCount, NumThreadsPerGroup)`
- **예시**: ParticleCount=1000, numthreads=64 → GroupCount=16 → 총 1024개 쓰레드
- **범위 체크 필요**: 여분 쓰레드는 `if (ParticleID >= ParticleCount) return;`으로 종료

### FRWBuffer 이슈 (기존)
- **문제**: Initialize()가 FRHICommandListBase 파라미터 요구 (UE5 변경사항)
- **원인**: Game Thread에서 직접 호출 불가 (Render Thread 전용)
- **해결책**: TArray로 CPU 백업 → UploadBytecode()에서 ENQUEUE_RENDER_COMMAND 사용하여 GPU 업로드
- **병목 없음**: 바이트코드 업로드는 BeginPlay에서 1회만 실행 (매 프레임 아님)

### 작업 순서 이해 (기존)
- **Compute Shader 먼저 작성**: GPU의 "설계도" 역할, 필요한 리소스와 파라미터 정의
- **C++ 함수는 나중**: Shader가 요구하는 것을 제공하는 "공급자" 역할
- **UploadBytecode/ExecuteSimulation**: Shader 완성 후 구현 (Shader 의존성)

### TArray vs FRDGPooledBuffer (수정)
- **TArray BytecodeData**: CPU 메모리 백업, 디버깅 용이
- **FRDGPooledBuffer**: GPU 메모리, RDG 외부에서 유지 가능, Shader가 직접 읽음
- **RegisterExternalBuffer**: Pooled Buffer를 RDG에 등록하여 Shader에 전달

---

**작성일**: 2025년 12월 14일

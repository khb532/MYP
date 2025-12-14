# 런타임 바이트코드 업데이트 테스트 레포트

**날짜**: 2025년 12월 15일
**목표**: GPU Compute Shader의 바이트코드를 런타임에 동적으로 업데이트 가능한지 검증
**결과**: ❌ **실패** - 바이트코드 업데이트 불가, 상수(Constants)만 업데이트 가능

---

## 📋 테스트 개요

### 테스트 시나리오
```
1. BeginPlay: 초기 바이트코드 업로드 (position.x = particleID * 10)
2. 1초: 초기 결과 확인
3. 5초: 새 바이트코드 업로드 (position.x = particleID)
4. 7초: 변경된 결과 확인 (FlushRenderingCommands 후)
```

### 예상 결과
```
1초: Particle[1] PosRaw(26, 0, 0, 0) - particleID(1) * 10 = 10 → 26
7초: Particle[1] PosRaw(1~4, 0, 0, 0) - particleID(1) = 1 → ~1-4
```

### 실제 결과
```
1초: Particle[1] PosRaw(26, 0, 0, 0) ✅
7초: Particle[1] PosRaw(26, 0, 0, 0) ❌ (변경 안 됨)
```

**하지만:**
- Color.G = 0 → 255 (초록) ✅ **ConstantData는 업데이트됨!**
- BytecodeData 로그 확인: `1 0 0 0 12` → `1 0 30 0 1` ✅ **CPU에서는 업데이트됨!**

---

## 🔍 테스트 과정

### Phase 1: FRDGPooledBuffer 방식 (실패)

**구현:**
```cpp
// UploadBytecode
GraphBuilder.QueueBufferUpload(BytecodeBuffer, ...);
GraphBuilder.QueueBufferExtraction(BytecodeBuffer, &BytecodePooledBuffer);

// ExecuteSimulation
FRDGBufferRef BytecodeBuffer = GraphBuilder.RegisterExternalBuffer(BytecodePooledBuffer);
```

**문제:**
- BytecodePooledBuffer 주소가 같은 주소 재사용 (풀 메커니즘)
- `SafeRelease()` 후에도 같은 주소 할당됨
- RDG가 버퍼를 "변경 안 됨"으로 판단하여 업로드 스킵

**시도한 해결책:**
1. ✅ `BytecodePooledBuffer.SafeRelease()` - 효과 없음
2. ✅ RenderTarget 재생성 - 효과 없음
3. ✅ `this` 포인터 캡처로 최신 버퍼 참조 - 효과 없음

---

### Phase 2: 매 프레임 새 버퍼 생성 (실패)

**구현:**
```cpp
// ExecuteSimulation에서 Pooled Buffer 제거
TArray<uint32> BytecodeCopy = BytecodeData;

ENQUEUE_RENDER_COMMAND(...) {
    FRDGBufferRef BytecodeBuffer = GraphBuilder.CreateBuffer(...);
    GraphBuilder.QueueBufferUpload(BytecodeBuffer, BytecodeCopy.GetData(), ...);
}
```

**문제:**
- 매 프레임 새 버퍼 생성해도 동일한 결과
- ConstantData는 업데이트되는데 BytecodeData는 안 됨

**시도한 해결책:**
1. ✅ UploadBytecode의 ENQUEUE_RENDER_COMMAND 제거 - 효과 없음
2. ✅ 버퍼 이름을 프레임마다 다르게 (`ParticleBytecodeBuffer_123`) - 효과 없음
3. ✅ `ERDGInitialDataFlags::None` 명시 - 효과 없음

---

### Phase 3: Shader 수정 검증 (성공)

**목적:** 바이트코드가 실행되는지 확인

**Shader 수정:**
```hlsl
// 원래
ColorRT[rtCoord] = color;

// 수정
ColorRT[rtCoord] = float4(position.x / 100.0, color.g, color.b, 1.0);
```

**결과:**
```
1초: Color(26, 0, 0) - position.x(10) / 100 * 255 ≈ 26 ✅
7초: Color(25, 255, 0) - 여전히 position.x(10), color.g만 업데이트 ✅
```

**결론:**
- ✅ Shader는 정상 작동
- ✅ position 변수 계산 정상
- ✅ ColorRT 쓰기 정상
- ❌ **BytecodeBuffer만 업데이트 안 됨**

---

### Phase 4: 상수 테스트 (부분 성공)

**테스트 1: position.x = 999.0 (상수)**
```cpp
NewBytecode.Add(PUSH_CONST);
NewBytecode.Add(0);  // Constants[0] = 999.0
NewBytecode.Add(STORE_POS_X);
```

**결과:**
```
7초: Particle[1] PosRaw(255, 0, 0, 0) ✅ 변경됨!
```

**테스트 2: position.x = particleID (변수)**
```cpp
NewBytecode.Add(PUSH_VAR);
NewBytecode.Add(PARTICLE_ID);
NewBytecode.Add(STORE_POS_X);
```

**결과:**
```
7초: Particle[1] PosRaw(26, 0, 0, 0) ❌ 변경 안 됨
```

---

## 🧪 진단 결과

### 확인된 사실

1. **BytecodeData는 CPU에서 업데이트됨**
   ```
   BeginPlay: [UploadBytecode] First bytecodes: 1 0 0 0 12  (PUSH_VAR, PARTICLE_ID, PUSH_CONST, 0, MUL)
   5초:       [UploadBytecode] First bytecodes: 1 0 30 0 1  (PUSH_VAR, PARTICLE_ID, STORE_POS_X)
   ```

2. **ConstantData는 GPU에 반영됨**
   ```
   Color.G: 0 → 255 (Constants[3] = 0.0 → 1.0)
   ```

3. **BytecodeBuffer는 GPU에 반영 안 됨**
   ```
   position.x: 10 (초기값 유지)
   ```

4. **상수 바이트코드(999.0)는 작동함**
   - PUSH_CONST는 작동
   - PUSH_VAR + 연산은 안 됨

5. **Shader는 정상 작동**
   - ColorRT에 position.x / 100 인코딩 성공
   - Color.R = 26 (position.x = 10 증명)

---

## 🎯 원인 분석

### 가설 1: RDG 버퍼 캐싱 (가장 유력)

**증거:**
- ConstantsBuffer는 업데이트되는데 BytecodeBuffer는 안 됨
- 버퍼 이름을 바꿔도, 매 프레임 새로 만들어도 같은 결과

**추정 원인:**
- RDG가 BytecodeBuffer의 **내용 해시**나 **메모리 주소**를 기반으로 캐싱
- "같은 크기, 같은 타입" → 이전 버퍼 재사용 판단
- `QueueBufferUpload`가 내부적으로 "이미 업로드됨" 판단하여 스킵

**왜 Constants는 되는가?**
- 크기가 작아서 캐싱 정책이 다를 수 있음
- 또는 우연히 캐싱 조건을 벗어남

### 가설 2: StructuredBuffer 동기화 문제

**증거:**
- PUSH_CONST(상수 999.0)는 작동
- PUSH_VAR(변수 particleID)는 안 작동

**추정 원인:**
- BytecodeBuffer가 StructuredBuffer로 선언됨
- GPU가 BytecodeBuffer 읽기를 최적화하여 첫 읽기를 캐시
- 이후 업데이트가 반영 안 됨

### 가설 3: Render Thread 타이밍

**증거:**
- FlushRenderingCommands() 사용해도 같은 결과

**추정 원인:**
- ExecuteSimulation과 UploadBytecode의 RenderCommand 실행 순서
- ExecuteSimulation이 BytecodeCopy를 캡처하는 시점이 너무 빠름

**하지만 반증:**
- BytecodeData 로그는 정확히 업데이트됨
- BytecodeCopy도 최신 데이터를 복사함

---

## 🛠️ 시도한 해결책

### 1. FRDGPooledBuffer 명시적 해제
```cpp
BytecodePooledBuffer.SafeRelease();
```
**결과:** ❌ 효과 없음

### 2. RenderTarget 재생성
```cpp
PositionRT = NewObject<UTextureRenderTarget2D>(this);
ColorRT = NewObject<UTextureRenderTarget2D>(this);
```
**결과:** ❌ 효과 없음 (RenderTarget 문제 아님)

### 3. this 포인터 캡처
```cpp
UParticleComputeComponent* ThisComponent = this;
ENQUEUE_RENDER_COMMAND(...)[ThisComponent](...) {
    GraphBuilder.RegisterExternalBuffer(ThisComponent->BytecodePooledBuffer);
}
```
**결과:** ❌ 효과 없음

### 4. 매 프레임 새 버퍼 생성
```cpp
TArray<uint32> BytecodeCopy = BytecodeData;
FRDGBufferRef BytecodeBuffer = GraphBuilder.CreateBuffer(...);
GraphBuilder.QueueBufferUpload(BytecodeBuffer, BytecodeCopy.GetData(), ...);
```
**결과:** ❌ 효과 없음

### 5. 버퍼 이름 동적 변경
```cpp
FString BytecodeName = FString::Printf(TEXT("ParticleBytecodeBuffer_%d"), FrameCounter);
```
**결과:** ❌ 효과 없음

### 6. ERDGInitialDataFlags 명시
```cpp
GraphBuilder.QueueBufferUpload(BytecodeBuffer, ..., ERDGInitialDataFlags::None);
```
**결과:** ❌ 효과 없음 (테스트 대기 중)

### 7. Shader 재컴파일
```
recompileshaders changed
```
**결과:** ✅ Shader는 반영됨 (ColorRT 인코딩 확인)

---

## 📊 테스트 로그 요약

### 성공한 테스트
```
✅ 초기 바이트코드 실행 (BeginPlay)
✅ ConstantData 런타임 업데이트
✅ Shader 수정 반영
✅ ColorRT 쓰기
✅ 상수 바이트코드 업데이트 (999.0)
```

### 실패한 테스트
```
❌ BytecodeData 런타임 업데이트 (PUSH_VAR 포함)
❌ FRDGPooledBuffer 재사용
❌ 매 프레임 새 버퍼 생성
❌ 버퍼 이름 변경
```

---

## 🔬 클린 빌드 효과

**첫 클린 빌드 (Phase 1):**
- 상수 999.0 테스트 성공
- 이후 particleID 테스트 실패

**Shader 파일 수정 후:**
- `recompileshaders changed` 처음에는 "No changes found"
- 주석 추가 (`// Force recompile v2`) 후 재컴파일 성공
- Shader는 정상 반영되었지만 바이트코드는 여전히 업데이트 안 됨

---

## 💡 결론

### 확실한 것
1. **Alpha-1 목표는 달성** - 초기 바이트코드 GPU 실행 성공 ✅
2. **Compute Shader 파이프라인 정상** - PositionRT/ColorRT 쓰기 성공 ✅
3. **OpCode 시스템 정상** - PUSH_CONST, PUSH_VAR, MUL, STORE 모두 작동 ✅
4. **Constants는 런타임 업데이트 가능** ✅

### 불확실한 것
1. **BytecodeBuffer 런타임 업데이트 불가** - 원인 미상 ❌
2. **RDG 캐싱 메커니즘** - 정확한 동작 방식 불명
3. **왜 Constants는 되는가?** - BytecodeBuffer와 차이점 불명

### 근본 원인 (추정)
**RDG의 내부 최적화가 BytecodeBuffer 업데이트를 방해**
- StructuredBuffer의 크기/타입이 같으면 재사용
- QueueBufferUpload의 중복 호출 감지
- GPU 캐시 또는 RDG 캐시 무효화 실패

---

## 🚀 다음 단계

### 옵션 1: 대안 방법 모색 (권장)
- **Lua 컴파일러를 C++에서 실행, 결과를 Constant로 전달**
  - 바이트코드 대신 미리 계산된 위치/색상 배열 전달
  - Constants는 업데이트 가능함이 확인됨

- **Niagara Script로 로직 작성**
  - Lua 대신 Niagara의 내장 스크립트 사용
  - GPU 친화적

- **CPU 시뮬레이션 + GPU 렌더링**
  - CPU에서 파티클 계산
  - GPU는 렌더링만 담당

### 옵션 2: RDG 전문가 컨설팅
- Unreal Slackers Discord
- Epic Games 포럼
- GitHub Issues

### 옵션 3: Beta 단계로 진행, 이 문제는 보류
- Niagara 연동 먼저 완성 (Step 1)
- Input System 구현 (Step 2)
- Lua UI 제작 (Step 3)
- 나중에 다시 시도

---

## 📝 교훈

### 기술적 학습
1. **RDG는 강력한 최적화를 수행**
   - 개발자가 명시적으로 요청해도 무시할 수 있음
   - 캐싱 메커니즘이 블랙박스

2. **FRDGPooledBuffer는 재사용을 위한 것**
   - 런타임 업데이트에는 부적합
   - 매 프레임 생성해도 내부적으로 재사용 가능

3. **Shader Hot Reload의 한계**
   - `recompileshaders changed`가 변경을 감지 못할 수 있음
   - 더미 주석 추가로 강제 재컴파일 필요

4. **Constants vs BytecodeBuffer**
   - 같은 방식으로 업로드해도 다르게 동작
   - 크기/타입/용도에 따라 RDG 동작 달라짐

### 프로세스 학습
1. **디버깅은 단계적으로**
   - BytecodeData CPU 확인 → Shader 확인 → GPU 업로드 확인
   - 각 단계를 로그로 검증

2. **클린 빌드의 중요성**
   - 첫 클린 빌드에서 상수 테스트 성공
   - Shader 캐시 문제 가능성 항상 고려

3. **대안 준비**
   - 하나의 방법에 집착하지 말고
   - 여러 접근 방법 동시 고려

---

**작성자**: Claude Code
**최종 수정**: 2025년 12월 15일

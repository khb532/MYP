# 기술적 타당성 심층 검토 및 개선안 보고서

**문서 작성일**: 2025-05-12
**관련 문서**: particle_system_dev_plan.md (Phase 2, 3, 4)
**검토 범위**: 기술적 타당성 및 위험 요소 분석 (Item 2)

---

## 1. GPU 기반 가상 머신 (Compute Shader Interpreter)

### 1.1 문제 제기 (Issue)
* **주제**: Phase 2.1 - HLSL Compute Shader 내부에서 Switch-Case 문을 사용하여 바이트코드를 해석하고 실행하는 로직.
* **위험 요소**: **분기 다이버전스 (Branch Divergence)**
    * GPU 아키텍처(SIMD) 특성상, 워프(Warp/Wavefront) 내의 스레드들이 서로 다른 분기(if-else, switch case)를 탈 경우, 모든 경로를 순차적으로 실행하며 마스킹(Masking) 처리하게 됨.
    * 파티클마다 실행해야 할 코드의 길이(LifeTime)나 로직 분기가 다를 경우, 심각한 성능 저하가 발생함.

### 1.2 기술 검토 (Analysis)
* **실현 가능성**: 가능함 (ShaderToy, Unity VFX Graph 등의 내부 구현 방식과 유사).
* **성능 병목**:
    * 명령어(OpCode)가 많아질수록 Switch문의 분기 예측 비용이 증가.
    * 레지스터 압박(Register Pressure)으로 인해 병렬 수행 가능한 스레드 수(Occupancy)가 감소할 수 있음.

### 1.3 개선안 (Improvement Proposal)
1.  **OpCode 최적화**:
    * `SWITCH` 문을 사용할 때 가장 빈번히 호출되는 OpCode(예: ADD, MUL, LERP)를 상단에 배치하거나, 유사 기능을 그룹화.
    * 복잡한 수학 연산은 별도 OpCode로 분리하지 말고 HLSL 내장 함수(Intrinsic)를 최대한 활용하는 `Packed OpCode` 설계.
2.  **데이터 주도형 분기 제거**:
    * 조건문(`if value > 0`)을 사용하는 대신, `step()`, `lerp()` 등의 HLSL 내장 함수를 활용하여 **Branchless Programming** 기법 적용.
    * 예: `if (x > 0) y = 1 else y = 0` → `y = step(0, x);`
3.  **루프 제한**:
    * 사용자 코드가 무한 루프에 빠지지 않도록 각 파티클당 최대 실행 명령어 수(Instruction Count)를 강제로 제한(Clamp).

---

## 2. 데이터 전송 파이프라인 (Compute to Niagara)

### 2.1 문제 제기 (Issue)
* **주제**: Phase 3.3 - Compute Shader의 시뮬레이션 결과를 Niagara 파티클 시스템에 전달하는 과정.
* **기존 계획**: `Compute Shader` → `CPU Memory (TArray)` → `Niagara Component`
* **위험 요소**: **PCIe 대역폭 병목 및 CPU 오버헤드**
    * GPU 메모리(VRAM)의 데이터를 CPU 메모리(RAM)로 복사(Readback)하는 과정은 매우 느림.
    * 5,000개 이상의 파티클을 매 프레임(60FPS) 복사할 경우, 렌더링 스레드와 게임 스레드 간의 동기화 대기로 인해 프레임 드랍 발생 확실시됨.

### 2.2 기술 검토 (Analysis)
* **판단**: 현재 계획된 `TArray` 변환 방식은 실시간 처리에 부적합함. 초기화 단계나 1초에 1번 업데이트되는 정적 데이터에만 사용 가능한 방식임.

### 2.3 개선안 (Improvement Proposal)
**"Zero-Copy GPU 파이프라인" 구축**

1.  **Render Target (Texture) 활용 (추천)**:
    * **Compute Shader**: 파티클의 위치(XYZ) 정보를 `RenderTarget2D`의 RGB 채널에 기록.
    * **Niagara**: `Grid 2D Collection` 또는 `Texture Sample` 모듈을 사용하여 해당 Render Target을 직접 샘플링.
    * **장점**: 구현이 가장 쉬우며 언리얼 엔진의 기존 인프라와 호환성이 좋음.
2.  **Structured Buffer (UAV) 직접 연결 (고급)**:
    * **C++**: `RWStructuredBuffer`를 생성하고 RHI를 통해 Compute Shader와 Niagara Data Interface에 동시에 바인딩.
    * **Niagara**: 커스텀 Data Interface를 C++로 작성하여 GPU 버퍼 포인터를 직접 참조.
    * **장점**: 메모리 효율이 가장 좋으나 구현 난이도가 높음.

---

## 3. 멀티플레이 결정론 (Multiplayer Determinism)

### 3.1 문제 제기 (Issue)
* **주제**: Phase 4.3 - 클라이언트 간 동일한 코드와 시드(Seed) 값을 공유하여 똑같은 시뮬레이션 결과를 얻는 방식.
* **위험 요소**: **부동소수점 비결정성 (Floating Point Non-determinism)**
    * IEEE 754 표준을 따르더라도, GPU 제조사(NVIDIA, AMD, Intel) 및 드라이버 버전에 따라 삼각함수(`sin`, `cos`)나 최적화(`fma`) 연산 결과가 미세하게 다름.
    * 시뮬레이션 시간이 길어질수록 '나비 효과'처럼 위치 오차가 누적되어 클라이언트 간 파티클 위치가 달라짐.

### 3.2 기술 검토 (Analysis)
* **판단**: 게임플레이에 결정적인 영향(예: 파티클에 맞아 죽음)을 주는 요소라면 이 방식은 **불가능**함. 하지만 단순 시각적 효과(Visual Effects)라면 허용 범위를 설정할 수 있음.

### 3.3 개선안 (Improvement Proposal)
1.  **목표 수정**: "완벽한 동기화"를 포기하고 "시각적 동기화"로 목표 완화.
2.  **하이브리드 동기화 모델**:
    * **Local Simulation**: 기본적으로 각 클라이언트가 로컬에서 코드를 실행(예측).
    * **Server Correction (권한 부여)**:
        * 서버는 전체 시뮬레이션을 돌리는 대신, **'파티클 생성 시점', '수명', '대표 파티클의 대략적 위치'** 등 중요 이벤트만 Reliable RPC로 전송.
        * 또는, 1초에 한 번씩 현재 시뮬레이션 시간(TimeStep)을 강제 동기화하여 오차 누적을 리셋.
3.  **정수 기반 연산 (선택 사항)**:
    * 정말로 위치가 똑같아야 한다면, 바이트코드 인터프리터 내부에서 부동소수점 대신 **고정 소수점(Fixed Point Math)** 연산을 구현해야 함 (구현 난이도 매우 높음, 성능 저하). -> **비추천**

---

## 4. Lua 컴파일러 및 파서 (Compiler Complexity)

### 4.1 문제 제기 (Issue)
* **주제**: Phase 2.3 - Lua 텍스트 코드를 파싱하여 AST(추상 구문 트리)를 만들고 바이트코드로 변환.
* **위험 요소**: **개발 공수 과다 산정**
    * 컴파일러/인터프리터 이론에 대한 깊은 이해가 없으면, 파서(Parser) 작성에만 프로젝트 전체 기간의 절반을 소모할 수 있음.

### 4.2 기술 검토 (Analysis)
* **판단**: Lua 전체 문법을 지원하는 것은 불필요하며 비효율적임. 프로젝트의 목표는 '파티클 제어'이지 'Lua 언어의 완벽한 구현'이 아님.

### 4.3 개선안 (Improvement Proposal)
1.  **기존 라이브러리 활용**:
    * 언리얼용 Lua 플러그인(`LuaMachine`, `UnLua` 등)을 사용하여 C++ 단에서 Lua 코드를 1차 실행(컴파일)하고, 그 결과로 나오는 데이터를 버퍼로 변환.
2.  **DSL (Domain Specific Language) 접근**:
    * Lua의 모든 기능을 지원하지 말고, 파티클 제어에 필요한 함수(`SetPos`, `SetColor` 등)만 허용하는 제한된 문법 사용.
3.  **바이트코드 생성 위임**:
    * 직접 파싱하지 말고, 표준 Lua C API (`lua_dump`)를 사용하여 Lua 공식 컴파일러가 생성한 바이트코드를 그대로 가져와서 GPU용으로 번역(Translation)만 수행.

---

## 5. 종합 요약

| 항목 | 위험도 | 해결 우선순위 | 핵심 솔루션 |
| :--- | :---: | :---: | :--- |
| **GPU 가상 머신** | 중 | 2 | 분기 최소화, 내장 함수 활용 |
| **데이터 파이프라인** | **최상** | **1** | **Render Target을 이용한 Zero-Copy 전송** |
| **멀티플레이 동기화** | 상 | 3 | 시각적 유사성 허용, 주기적 시간 동기화 |
| **Lua 컴파일러** | 중 | 4 | 기존 Lua API 활용하여 파싱 로직 최소화 |

> **Consultant Note**:
> 프로젝트의 성패는 **2.3 데이터 파이프라인 개선안(Render Target 활용)**을 얼마나 빠르게 검증하느냐에 달려 있습니다. 이 부분이 해결되지 않으면 60FPS 구동이 불가능하므로, Alpha 단계에서 반드시 이 아키텍처를 확정 지으시기 바랍니다.
# 프로젝트 검토 보고서 사용자 정의 파티클 시스템

문서 작성일 2025-05-12
대상 문서 particle_system_dev_plan.md
검토자 Gemini (AI Technical Consultant)

---

## 1. 종합 평가 (Executive Summary)

 개발 가능성 (Feasibility) 가능함 (Feasible)
     제안된 아키텍처(`Lua - Bytecode - GPU Compute Shader - Niagara`)는 기술적으로 유효하며, 논리적 흐름이 매우 명확합니다.
 난이도 (Difficulty) 상 (High)
     일반적인 게임 플레이 로직 구현이 아닌, 언리얼 엔진 내부의 RHI(Rendering Hardware Interface)와 렌더링 파이프라인을 다루는 '엔진 내의 엔진'을 개발하는 프로젝트입니다.
 핵심 의견
     계획의 구조는 훌륭하나, Phase 3.3(데이터 전송)과 Phase 2.1(GPU 인터프리터) 구간에서 심각한 성능 병목이 발생할 수 있습니다. 해당 부분의 아키텍처 수정이 성공의 열쇠입니다.

---

## 2. 주요 기술적 분석 및 리스크

### 2.1. GPU 가상 머신 (Phase 2.1 & 2.3)
 계획 Lua 코드를 바이트코드로 변환 후, HLSL 쉐이더 내 `Switch-Case` 문을 통해 각 파티클별로 명령 수행.
 분석 기술적으로 구현 가능하며 ShaderToy 등에서 사용되는 방식입니다.
 리스크 Branch Divergence (분기 다이버전스)
     GPU는 모든 스레드가 동시에 같은 명령어를 처리할 때 가장 빠릅니다.
     각 파티클이 서로 다른 조건문(`ifelse`)이나 반복문 상태에 빠질 경우, GPU 병렬 처리 효율이 급격히 저하될 수 있습니다.
 권장사항 초기 개발 시 OpCode(명령어) 종류를 최소화하고, 복잡한 분기문보다는 수학적 연산 위주로 먼저 구현하여 퍼포먼스를 검증해야 합니다.

### 2.2. 데이터 파이프라인 병목 (Phase 3.3)
 계획 `Compute Shader 결과` → `CPU (TArray 변환)` → `Niagara (SetNiagaraArrayVector)`
 분석 이 부분은 가장 큰 성능 저하를 유발할 수 있는 설계입니다.
 문제점 GPU Readback Latency
     GPU에서 계산된 데이터를 매 프레임 CPU로 가져오는 것(Readback)은 매우 느립니다. 이를 다시 GPU(Niagara)로 보내는 것은 이중 부하를 발생시킵니다.
 권장사항 Zero-Copy 전송 (GPU to GPU)
     CPU를 거치지 않아야 합니다.
     Compute Shader가 `Render Target`이나 `Structured Buffer`에 직접 쓰고, Niagara가 Data Interface를 통해 해당 버퍼를 직접 읽는 방식(Grid 2D Collection, Texture Sample 등)으로 변경해야 합니다.

### 2.3. 멀티플레이 결정론 (Phase 4.3)
 계획 시드(Seed)값과 코드 동기화를 통해 모든 클라이언트에서 동일한 결과 보장.
 분석 이론적으로는 이상적이나 현실적인 난관이 존재합니다.
 리스크 Floating Point Determinism
     서로 다른 GPU 제조사(NVIDIA vs AMD)나 드라이버 버전에 따라 부동소수점(`sin`, `cos` 등) 연산 결과가 미세하게 다를 수 있습니다. 시간이 지날수록 위치 오차가 누적됩니다.
 권장사항 완벽한 동기화보다는 '시각적 유사성'을 목표로 하거나, 주기적으로 서버가 중요 파티클의 위치를 보정해주는 하이브리드 방식이 안전합니다.

---

## 3. 수정 제안 (Recommendations)

성공 확률을 높이기 위해 개발 우선순위와 아키텍처를 다음과 같이 조정할 것을 제안합니다.

### 3.1. 아키텍처 수정 CPU 바이패스
기존 Phase 3.3의 설계를 아래와 같이 변경하십시오.

[기존]
`Compute Shader` → (VRAM to RAM 다운로드) → `C++ TArray` → (RAM to VRAM 업로드) → `Niagara`

[변경]
`Compute Shader` → [Render Target  RWStructuredBuffer] → `Niagara Data Interface` (직접 참조)

### 3.2. Alpha 버전 필수 선행
문서에 포함된 Alpha 버전 계획은 매우 적절합니다. 본 프로젝트 시작 전, 다음 항목만 포함된 프로토타입을 최우선(1-2주 내)으로 완료하십시오.
1.  Lua 파서 없이, C++ 배열에 하드코딩된 바이트코드 준비.
2.  Compute Shader가 이를 읽어 위치 계산.
3.  Niagara가 해당 결과를 화면에 표시.
4.  (UI, 멀티플레이, 기즈모, 보안 검증 생략)

### 3.3. Lua 컴파일러 전략
직접 컴파일러를 작성하는 것(Phase 2.3)은 컴파일러 이론에 능숙하지 않다면 일정이 크게 지연될 수 있습니다. `LuaMachine` 플러그인 등 기존 솔루션이 바이트코드 추출을 지원하는지 먼저 확인하거나, 아주 단순한 형태의 자체 스크립트 언어로 시작하는 것을 고려해 보십시오.

---

## 4. 일정 및 로드맵 검토

 예상 기간 1인 개발 기준 14-19주는 상당히 타이트합니다.
 병목 구간 'Phase 2.3 (컴파일러)'와 'Phase 4 (멀티플레이)'에서 예상보다 2배 이상의 시간이 소요될 수 있습니다.
 전략 UI와 편의 기능(기즈모, 문법 강조 등)은 최후순위로 미루고, [파이프라인 연결]에 모든 역량을 집중해야 합니다.

---

## 5. 결론

이 프로젝트는 기술적으로 충분히 실현 가능하며, 완성 시 매우 높은 수준의 그래픽스 엔지니어링 역량을 증명할 수 있는 포트폴리오가 될 것입니다.

다만, 데이터 전송 방식(CPU 배제)과 GPU 분기 처리 이슈만 초기에 잘 해결한다면, 나머지 기능(UI, 라이브러리 확장)은 순조롭게 진행될 것입니다.

[승인 및 진행 권장]
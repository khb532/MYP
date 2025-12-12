# 사용자 정의 파티클 시스템 개발 계획서

## 프로젝트 개요

### 목표
플레이어가 런타임에 Lua 코드를 작성하여 커스텀 파티클 모션 그래픽을 생성하고, 멀티플레이 환경에서 다른 플레이어와 공유할 수 있는 시스템 개발

### 핵심 기능
- 플레이트 메쉬 소환 및 배치 (기즈모)
- 런타임 Lua 코드 입력 UI
- GPU Compute Shader 기반 파티클 시뮬레이션
- 사용자 정의 함수 지원
- 홀로그램 스타일 렌더링
- 멀티플레이 동기화

---

## Phase 1: 기초 인프라 구축 (2-3주)

### 1.1 플레이트 시스템
**목표**: 기본 플레이트 생성 및 배치 기능

**구현 항목**:
- [ ] `APlateActor` 클래스 생성
  - StaticMeshComponent (500×350 평면)
  - RenderTarget2D (파티클 렌더링용)
  - MaterialInstanceDynamic (홀로그램 머티리얼)
- [ ] 플레이트 소환 시스템
  - PlayerController에서 버튼 입력 처리
  - 플레이어 앞 적절한 위치에 스폰
- [ ] 기즈모 시스템
  - Runtime Transformer Plugin 통합 또는
  - 커스텀 3축 기즈모 구현 (LineTrace + Widget)
  - 위치/회전/스케일 조작

**산출물**:
- 플레이트 Blueprint 또는 C++ 클래스
- 기즈모 인터페이스

---

### 1.2 UI 시스템
**목표**: 코드 입력 및 편집 인터페이스

**구현 항목**:
- [ ] UMG 코드 에디터 위젯
  - Multi-line Editable Text Box
  - 저장/실행 버튼
  - 에러 메시지 표시 영역
- [ ] 선택 사항: Syntax Highlighting
  - Rich Text Block 활용
  - Lua 키워드 색상 처리
- [ ] 코드 템플릿 제공
  - 기본 함수 구조 자동 생성
  - 예제 코드 삽입

**산출물**:
- CodeEditorWidget.uasset
- 기본 Lua 템플릿 파일

---

### 1.3 Lua 통합
**목표**: Lua VM 및 C++ 바인딩

**구현 항목**:
- [ ] Lua 라이브러리 선택 및 통합
  - 옵션 1: LuaMachine Plugin
  - 옵션 2: slua-unreal
  - 옵션 3: 순수 LuaJIT 직접 통합
- [ ] 기본 C++ ↔ Lua 바인딩
  - vec3, Color 타입
  - 수학 함수 (sin, cos, length, normalize 등)
- [ ] 샌드박스 환경 설정
  - 위험한 함수 비활성화 (io, os, debug)
  - 실행 시간 제한 (타임아웃)
  - 메모리 제한

**산출물**:
- ULuaComponent 또는 LuaVM 매니저 클래스
- 바인딩 API 문서

---

## Phase 2: Compute Shader 파이프라인 (3-4주)

### 2.1 HLSL Compute Shader
**목표**: 파티클 시뮬레이션 커널 구현

**구현 항목**:
- [ ] ParticleSimulation.usf 생성
  - 파티클 데이터 구조 정의 (Position, Velocity, Color, Age)
  - 바이트코드 인터프리터 구현
  - OpCode 정의 (50-100개)
- [ ] OpCode 구현
  - **스택 조작**: PUSH_CONST, PUSH_VAR, POP, STORE_VAR
  - **산술 연산**: ADD, SUB, MUL, DIV, MOD, POW
  - **비교 연산**: LT, GT, EQ
  - **수학 함수**: SIN, COS, TAN, SQRT, ABS, FLOOR, CEIL, CLAMP, LERP
  - **벡터 연산**: LENGTH, NORMALIZE, DOT, CROSS
  - **제어 흐름**: JUMP, JUMP_IF_ZERO
  - **특수**: NOISE (해시 기반)
- [ ] 파티클 물리 업데이트 로직
  - 힘 적용
  - 속도 제한
  - 경계 처리

**산출물**:
- ParticleSimulation.usf
- OpCode 명세서

---

### 2.2 GPU 리소스 관리
**목표**: C++에서 GPU 버퍼 생성 및 관리

**구현 항목**:
- [ ] `UParticleComputeSystem` 클래스
  - FRWBuffer ParticleBuffer (파티클 데이터)
  - FRWBuffer BytecodeBuffer (사용자 바이트코드)
  - FRWBuffer ConstantBuffer (상수 풀)
- [ ] GPU 버퍼 업로드/다운로드
  - CPU → GPU 데이터 전송
  - GPU → CPU 결과 읽기
- [ ] Compute Shader 디스패치
  - RHI Command 생성
  - 적절한 ThreadGroup 크기 계산

**산출물**:
- ParticleComputeSystem.h/cpp
- GPU 프로파일링 도구 통합

---

### 2.3 Lua → Bytecode 컴파일러
**목표**: Lua AST 파싱 및 바이트코드 생성

**구현 항목**:
- [ ] `FLuaBytecodeCompiler` 클래스
  - Lua 코드 → AST 파싱
  - AST → 바이트코드 변환
  - 상수 풀 생성
- [ ] 함수 분석
  - 사용자 정의 함수 인식
  - 매개변수 추출
  - 함수 호출 해석
- [ ] 최적화 (선택)
  - 상수 접기 (Constant Folding)
  - 죽은 코드 제거 (Dead Code Elimination)
- [ ] 에러 처리
  - 문법 오류 검출
  - 친절한 에러 메시지

**산출물**:
- LuaBytecodeCompiler.h/cpp
- 바이트코드 디버거 (옵션)

---

## Phase 3: 나이아가라 통합 (2주)

### 3.1 나이아가라 시스템 설정
**목표**: 파티클 렌더링 파이프라인

**구현 항목**:
- [ ] 단일 파티클 이미터 생성
  - 심플한 Sprite/Mesh 파티클
  - 대량 인스턴싱 지원
- [ ] Array Data Interface 설정
  - User.Positions (Vector Array)
  - User.Colors (Color Array)
  - User.Sizes (Float Array, 옵션)
- [ ] 파티클 초기화
  - Position = User.Positions[Particle.ID]
  - Color = User.Colors[Particle.ID]

**산출물**:
- NS_UserParticles.uasset
- NE_UserParticle.uasset

---

### 3.2 홀로그램 머티리얼
**목표**: 시각적 품질 향상

**구현 항목**:
- [ ] 홀로그램 머티리얼 제작
  - Emissive (발광)
  - Opacity (투명도)
  - Fresnel (가장자리 빛남)
  - Depth Fade (부드러운 교차)
- [ ] Post Process
  - Bloom 설정
  - Chromatic Aberration (옵션)
- [ ] 파티클 애니메이션
  - 페이드 인/아웃
  - 반짝임 효과

**산출물**:
- M_Hologram.uasset
- MI_Hologram_Inst.uasset

---

### 3.3 Compute → Niagara 연결
**목표**: GPU 시뮬레이션 결과를 나이아가라로 전달

**구현 항목**:
- [ ] C++ 연결 코드
  - Compute Shader 결과 → TArray 변환
  - UNiagaraComponent::SetNiagaraArrayVector 호출
- [ ] 프레임 동기화
  - Compute Shader를 Tick에서 실행
  - 나이아가라 업데이트 타이밍 조정
- [ ] 퍼포먼스 최적화
  - GPU/CPU 동기화 최소화
  - 비동기 버퍼 읽기 (옵션)

**산출물**:
- ParticleRenderer.h/cpp

---

## Phase 4: 멀티플레이 구현 (2-3주)

### 4.1 네트워크 아키텍처
**목표**: 리플리케이션 구조 설계

**구현 항목**:
- [ ] 플레이트 리플리케이션
  - Actor Replication 활성화
  - 위치/회전 동기화
- [ ] 코드 실행 권한 모델
  - Server Authority (서버 검증)
  - Client Prediction (옵션)
- [ ] RPC 함수 구현
  - `Server_ExecuteUserCode` (Reliable)
  - `Multicast_SpawnParticles` (Reliable)
  - `Multicast_StartSimulation` (Reliable)

**산출물**:
- Replication 설계 문서
- 네트워크 프로파일링 결과

---

### 4.2 정적 그래픽 동기화
**목표**: 결과 데이터만 전송

**구현 항목**:
- [ ] 서버 시뮬레이션
  - 클라이언트 코드 수신
  - 악의적 코드 검증
  - GPU에서 시뮬레이션 실행
- [ ] 결과 압축
  - FVector_NetQuantize 사용
  - 색상 압축 (FColor, 32bit)
- [ ] Multicast 전송
  - 위치 배열 브로드캐스트
  - 색상 배열 브로드캐스트

**산출물**:
- 정적 동기화 프로토타입

---

### 4.3 동적 그래픽 동기화
**목표**: 결정론적 시뮬레이션

**구현 항목**:
- [ ] 시드 기반 랜덤
  - 고정 시드 값 동기화
  - 모든 클라이언트에서 동일한 랜덤 시퀀스
- [ ] 코드 압축
  - 바이트코드 압축 (zlib, LZ4)
  - 델타 압축 (이전 코드와 차이만)
- [ ] 독립 시뮬레이션
  - 각 클라이언트가 로컬 실행
  - 시드 + 코드로 동일 결과 보장

**산출물**:
- 결정론적 시뮬레이션 검증 테스트
- 동기화 오차 분석 리포트

---

### 4.4 보안 및 검증
**목표**: 악의적 코드 방지

**구현 항목**:
- [ ] 코드 검증 시스템
  - 무한 루프 감지
  - 메모리 폭발 방지
  - 파티클 수 제한
  - 실행 시간 제한
- [ ] 샌드박스 강화
  - 위험 함수 블랙리스트
  - 호출 깊이 제한
- [ ] 치팅 방지
  - 서버 권한 검증
  - 해시 체크섬

**산출물**:
- 보안 체크리스트
- 악의적 코드 테스트 케이스

---

## Phase 5: 고급 기능 (3-4주)

### 5.1 Flocking 지원
**목표**: 이웃 파티클 접근

**구현 항목**:
- [ ] 공간 분할 구조
  - Spatial Hash Grid (GPU)
  - 이웃 검색 최적화
- [ ] 추가 GPU 버퍼
  - AllParticlePositions (읽기 전용)
  - AllParticleVelocities (읽기 전용)
- [ ] 바이트코드 확장
  - GET_PARTICLE_POS
  - GET_NEIGHBORS
  - FIND_NEAREST

**산출물**:
- Flocking 데모
- 퍼포먼스 벤치마크

---

### 5.2 사용자 데이터 배열
**목표**: 커스텀 데이터 저장

**구현 항목**:
- [ ] Lua에서 배열 정의
  ```lua
  local myData = {1, 2, 3, 4, 5}
  ```
- [ ] GPU 업로드
  - StructuredBuffer<float> UserDataBuffer
- [ ] 바이트코드 접근
  - GET_USER_DATA opcode

**산출물**:
- 데이터 시각화 예제

---

### 5.3 전역 변수 공유
**목표**: 파티클 간 상태 공유

**구현 항목**:
- [ ] RWStructuredBuffer<float> GlobalVars
- [ ] Atomic 연산 지원
  - InterlockedAdd
  - InterlockedMax
- [ ] Lua 인터페이스
  ```lua
  globalVar count = 0
  atomicAdd(count, 1)
  ```

**산출물**:
- 전역 상태 기반 애니메이션 예제

---

### 5.4 라이브러리 시스템
**목표**: 재사용 가능한 함수 모음

**구현 항목**:
- [ ] 표준 라이브러리
  - Math: lerp, clamp, remap 등
  - Easing: easeInOut, bounce 등
  - Noise: Perlin, Simplex
  - Pattern: spiral, vortex, wave 등
- [ ] 사용자 라이브러리
  - 코드 저장/불러오기
  - 다른 플레이어와 공유
- [ ] Import 시스템
  ```lua
  import "stdlib.math"
  import "mylib.patterns"
  ```

**산출물**:
- 표준 라이브러리 문서
- 커뮤니티 라이브러리 플랫폼 (선택)

---

## Phase 6: 최적화 및 폴리싱 (2-3주)

### 6.1 퍼포먼스 최적화

**프로파일링**:
- [ ] GPU 프로파일링
  - Compute Shader 실행 시간 측정
  - 병목 지점 파악
- [ ] CPU 프로파일링
  - Lua 실행 시간
  - 바이트코드 컴파일 시간
  - 네트워크 대역폭

**최적화**:
- [ ] Compute Shader 최적화
  - 스레드 그룹 크기 조정
  - 메모리 접근 패턴 개선
  - 분기 최소화
- [ ] LOD 시스템
  - 거리에 따라 파티클 수 감소
  - 화면 밖 컬링
- [ ] 객체 풀링
  - 플레이트 재사용
  - 나이아가라 컴포넌트 재사용

**산출물**:
- 퍼포먼스 리포트
- 최적화 가이드라인

---

### 6.2 사용자 경험 개선

**에러 처리**:
- [ ] 친절한 에러 메시지
  - 줄 번호 표시
  - 오류 원인 설명
  - 수정 제안
- [ ] 런타임 디버깅
  - print() 함수 지원
  - 변수 값 모니터링
  - 스텝 실행 (선택)

**튜토리얼**:
- [ ] 인게임 튜토리얼
  - 기본 사용법
  - 예제 코드
  - 인터랙티브 가이드
- [ ] 샘플 갤러리
  - 10-20개 프리셋 예제
  - 코드와 결과 함께 표시

**산출물**:
- 튜토리얼 시스템
- 예제 갤러리

---

### 6.3 안정성 테스트

**테스트 케이스**:
- [ ] 단위 테스트
  - 바이트코드 컴파일러
  - 각 OpCode 동작
  - 수학 함수 정확도
- [ ] 통합 테스트
  - 전체 파이프라인
  - 멀티플레이 시나리오
  - 엣지 케이스
- [ ] 스트레스 테스트
  - 10,000+ 파티클
  - 복잡한 코드 (중첩 루프)
  - 동시 다발 실행

**산출물**:
- 테스트 스위트
- 버그 리포트

---

## 기술 스택 정리

### 필수 기술
1. **Unreal Engine 5**
   - C++ 프로그래밍
   - Blueprint
   - RHI (Rendering Hardware Interface)
   - Replication

2. **HLSL**
   - Compute Shader
   - 바이트코드 인터프리터 구현
   - GPU 최적화

3. **Lua**
   - Lua VM 통합
   - C++ 바인딩
   - 샌드박스

4. **컴파일러 이론**
   - AST 파싱
   - 바이트코드 생성
   - 최적화 기법

5. **나이아가라**
   - 이미터 설정
   - Data Interface
   - 머티리얼 통합

6. **네트워킹**
   - Actor Replication
   - RPC
   - 압축/최적화

---

## 개발 도구

### IDE 및 에디터
- Visual Studio 2022
- Unreal Editor 5.x
- RenderDoc (GPU 디버깅)

### 플러그인
- LuaMachine 또는 slua-unreal
- Runtime Transformer (기즈모)

### 버전 관리
- Git + LFS
- GitHub/GitLab

### 프로파일링
- Unreal Insights
- GPU Profiler
- Network Profiler

---

## 마일스톤 및 일정

| Phase | 기간 | 주요 산출물 |
|-------|------|------------|
| **Phase 1** | 2-3주 | 플레이트 시스템, UI, Lua 통합 |
| **Phase 2** | 3-4주 | Compute Shader, 바이트코드 컴파일러 |
| **Phase 3** | 2주 | 나이아가라 통합, 홀로그램 렌더링 |
| **Phase 4** | 2-3주 | 멀티플레이, 동기화, 보안 |
| **Phase 5** | 3-4주 | 고급 기능 (Flocking, 라이브러리) |
| **Phase 6** | 2-3주 | 최적화, 폴리싱, 테스트 |
| **총 기간** | **14-19주** | **완성된 시스템** |

---

## 위험 요소 및 대응

### 기술적 위험

**1. Compute Shader 퍼포먼스**
- **위험**: 복잡한 바이트코드 실행이 느릴 수 있음
- **대응**: 
  - 프로파일링 후 병목 최적화
  - 단순한 케이스는 네이티브 함수로 우회
  - GPU 점유율 모니터링

**2. 멀티플레이 동기화**
- **위험**: 결정론적 시뮬레이션 실패 가능
- **대응**:
  - 정적 그래픽부터 구현 (안전)
  - 동적은 베이크 애니메이션으로 대체
  - 검증 테스트 철저히

**3. Lua 통합 복잡도**
- **위험**: C++ 바인딩 오버헤드
- **대응**:
  - 기존 플러그인 활용 (LuaMachine)
  - 최소 바인딩으로 시작
  - 점진적 확장

### 일정 위험

**1. 예상보다 긴 개발 시간**
- **대응**: 
  - MVP(Minimum Viable Product) 먼저 완성
  - 고급 기능은 Phase 5로 연기 가능
  - 2주마다 마일스톤 체크

**2. 디버깅 시간**
- **대응**:
  - 단위 테스트 조기 도입
  - 로깅 시스템 강화
  - 재현 가능한 테스트 케이스 작성

---

## MVP (Minimum Viable Product)

**Phase 1-3만으로 MVP 구성 가능**:
- ✅ 플레이트 소환 및 배치
- ✅ Lua 코드 입력
- ✅ 단순한 파티클 애니메이션 (10개 프리셋 함수)
- ✅ 홀로그램 렌더링
- ✅ 싱글플레이

**이후 점진적 추가**:
- Phase 4: 멀티플레이
- Phase 5: 고급 기능
- Phase 6: 폴리싱

---

## 학습 자료

### Compute Shader
- [HLSL Reference (Microsoft)](https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl)
- [GPU Gems: Chapter 29](https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-29-real-time-rigid-body-simulation-gpus)

### Lua
- [Programming in Lua (Book)](https://www.lua.org/pil/)
- [Lua C API Tutorial](https://www.lua.org/manual/5.4/)

### 컴파일러
- [Crafting Interpreters (Book)](https://craftinginterpreters.com/)
- [Let's Build a Compiler (Tutorial)](https://compilers.iecc.com/crenshaw/)

### Unreal RHI
- [Unreal Rendering Docs](https://docs.unrealengine.com/en-US/ProgrammingAndScripting/Rendering/)
- [Community Wiki: Custom Shaders](https://unrealcommunity.wiki/)

### 나이아가라
- [Official Niagara Overview](https://docs.unrealengine.com/5.0/en-US/overview-of-niagara-effects-for-unreal-engine/)
- [Niagara Tutorial Series (YouTube)](https://www.youtube.com/unrealengine)

---

## 성공 지표

### 기술적 목표
- [ ] 5,000개 파티클 @ 60fps
- [ ] 바이트코드 컴파일 < 100ms
- [ ] 멀티플레이 동기화 오차 < 1%
- [ ] 메모리 사용량 < 500MB

### 사용자 경험
- [ ] 코드 작성 → 결과 확인 < 2초
- [ ] 에러율 < 5%
- [ ] 튜토리얼 완료율 > 80%

### 확장성
- [ ] 50개 이상 표준 함수
- [ ] 100개 이상 OpCode
- [ ] 커뮤니티 라이브러리 지원

---

## 참고 프로젝트

### 유사 시스템
- **ShaderToy**: 웹 기반 셰이더 코딩
- **Processing**: 비주얼 코딩 언어
- **TouchDesigner**: 노드 기반 비주얼 프로그래밍
- **Dreams (PS4/PS5)**: 게임 내 창작 도구

### 벤치마크 대상
- 파티클 수: Niagara 공식 데모
- 코드 복잡도: ShaderToy 인기 작품
- 멀티플레이: Fortnite Creative

---

## 다음 단계

1. **즉시 시작 가능**:
   - Unreal 프로젝트 생성
   - 플레이트 Actor 기본 구조 작성
   - Lua 플러그인 조사 및 선택

2. **1주 내 완료**:
   - Phase 1.1 (플레이트 시스템)
   - 기본 UI 프로토타입

3. **1개월 내 목표**:
   - Phase 1-2 완료
   - 첫 번째 파티클 애니메이션 작동

---

## 문서 업데이트

- **작성일**: 2024-12-11
- **버전**: 1.0
- **다음 리뷰**: Phase 1 완료 시

---

## 연락 및 피드백

프로젝트 진행 중 질문이나 기술적 이슈 발생 시:
- GitHub Issues
- Discord 개발 채널
- 주간 진행 상황 리뷰

---

**Good Luck!** 🚀

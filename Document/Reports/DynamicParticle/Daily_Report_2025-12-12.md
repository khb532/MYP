# 일일 작업 레포트 - 2025년 12월 12일

## 📋 프로젝트 정보
- **전체 프로젝트**: Dynamic Particle System (사용자 정의 Lua 스크립트로 파티클 제어)
- **현재 단계**: Alpha - 핵심 파이프라인 검증용 최소 구현
- **Alpha 목표**: Lua → 바이트코드 → GPU Compute Shader → 나이아가라 파이프라인 구축
- **작업 브랜치**: DynamicNiagara

---

## ✅ 완료된 작업

**생성된 파일**:
- `Source/MYP/Particle/ParticleOpCodes.h` - OpCode 및 Variable enum 정의

---

## 📊 전체 TODO 현황 (1/5 완료)

1. ✅ **OpCode 시스템 설계 및 ParticleOpCodes.h 작성**

2. ⏳ **ParticleComputeComponent 기본 구조 작성** (다음 작업)
   - [ ] UActorComponent 상속 클래스 생성
   - [ ] RenderTarget2D 멤버 변수 선언 (PositionRT, ColorRT)
   - [ ] 바이트코드 업로드 함수 구현 (UploadBytecode)
   - [ ] GPU 실행 함수 구현 (ExecuteSimulation)
   - [ ] RenderTarget Getter 함수 추가

3. ⏳ **RenderTarget2D 리소스 관리 구현**
4. ⏳ **Compute Shader (ParticleSimulation.usf) 작성**
5. ⏳ **PlateActor 기본 구조 및 컴포넌트 통합**

---

## 💡 학습 내용

- **스택 기반 VM**: OpCode를 스택에 쌓아 실행하는 방식 이해
- **C++ ↔ GPU 데이터 전달**: enum은 C++에서만 의미 있고, GPU는 숫자만 처리
- **enum class**: 상속받지 않으며 타입 안전성만 제공하는 상수 집합

---

**작성일**: 2025년 12월 12일

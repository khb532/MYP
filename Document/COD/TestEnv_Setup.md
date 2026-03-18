# COD 탄도 테스트 환경 구축

## 개요

- **소스 프로젝트**: `E:\Workspace\Team\COD_CloneProject` (팀 COD 클론)
- **대상 프로젝트**: `E:\Workspace\Unreal\MYP` (개인 프로젝트)
- **작업 목적**: COD 클론의 탄도 시스템(`BulletActor`, `WeaponBase`)을 개인 프로젝트에 이식하고 탄도 동작을 테스트하는 환경 구축

## TODO

### 1. 코드 이식
- [ ] `BulletActor.h/cpp`, `WeaponBase.h/cpp` → MYP Source 복사
- [ ] `COD_API` → `MYP_API` 치환
- [ ] `MYP.Build.cs` 에 `Niagara` 모듈 추가
- [ ] 빌드 확인

### 2. FPP 번들팩
- [ ] 에디터 `Add Feature or Content Pack` → First Person 추가
- [ ] `World Settings` → GameMode → `BP_FirstPersonGameMode` 지정

### 3. 거리 표지판
- 액터: `Source/MYP/cod/DistanceSign.h/.cpp` (`ADistanceSign`)
- [x] `ADistanceSign` 구현 완료
  - `USceneComponent Root` 를 실제 루트로, `Mesh` / `TextRender` 를 형제로 어태치 (스케일 분리)
  - `UTextRenderComponent TextRender` 추가
  - `BeginPlay`에서 X좌표 → m 변환 (`/ 100.f`), 소수점 1자리 포맷 (`200.5M`)
- [x] BP 생성 후 레벨 배치 확인
- [x] 스타팅 라인 (0,0,0 기준 빨간 Plane) 배치
  - Plane Scale `X=0.1, Y=폭에 맞게`, Z=1 (Z-fighting 방지)
  - 빨간 머티리얼 적용

### 4. 탄도 테스트 준비
- [ ] `BP_Bullet` → `bDrawDebug = true`
- [ ] PIE 발사 → 거리별 탄착점/낙하량 육안 확인

---
*참고: Unreal 1m = 100cm*

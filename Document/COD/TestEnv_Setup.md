# COD 탄도 테스트 환경 구축

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
- 현재 상태: `UStaticMeshComponent Mesh` 선언만 있음, 구현 미완
- [ ] `ADistanceSign` 구현
  - `UTextRenderComponent` 추가 (주석처리된 TextRender 대체)
  - `BeginPlay`에서 거리 텍스트 자동 표시
- [ ] BP 생성 후 레벨 배치 (원점 기준, X축)
  - 10m → X: 1000
  - 20m → X: 2000
  - 50m → X: 5000
  - 100m → X: 10000
  - 200m → X: 20000

### 4. 탄도 테스트 준비
- [ ] `BP_Bullet` → `bDrawDebug = true`
- [ ] PIE 발사 → 거리별 탄착점/낙하량 육안 확인

---
*참고: Unreal 1m = 100cm*

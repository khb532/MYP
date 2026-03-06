# 일일 작업 레포트 - 2026년 03월 06일

## 📋 프로젝트 정보
- **전체 프로젝트**: COD CloneProject - Call of Duty 스타일 팀 협업 UE5 C++ 게임
- **현재 단계**: 포트폴리오 정리
- **포트폴리오 목표**: COD 프로젝트 내 담당 개발 사항 문서화
- **작업 브랜치**: Dev (COD_CloneProject)
- **기여자 식별**: khb532 / Dove9

---

## ✅ 완료된 작업

**분석 완료 - 본인 커밋 전수 조사 (author: khb532 / Dove9)**

총 커밋 수: **~55개** (2025-08-07 ~ 2025-10-10)

---

## 📊 담당 기능 전체 현황

### 1. Ally AI 시스템 (AllyAI-khb 브랜치)

#### [FEAT] AllyBase `2025-08-11`
- `AAllyCharacterBase` 클래스 설계 및 구현
- 아군 AI의 기반 캐릭터 클래스 (ACharacter 파생)
- `EAllyState` UENUM 정의: `Idle / Ready / Move / Cover / Shoot / Damage / Die`
- AI 자동 점유 설정: `AutoPossessAI = PlacedInWorldOrSpawned`
- `AAllyAIController` 기초 구조 작성
- `PathTestPawn` 내비게이션 테스트용 폰 구현

#### [FEAT] Ally FSM `2025-08-12`
- 상태 머신(FSM) 구조 분리: `AllyFSM` 클래스 독립 설계
- `Tick` 기반 상태 전이 로직 구현
- Cover 쿨다운, 사격 타이머, 이동 플래그 관리

#### [FEAT] Ally AI Controller `2025-08-13`
- `AAllyAIController::OnPossess` → `StoryManager` 자동 등록
- `MoveToLocation` 기반 거점 이동 (`DefenseAcceptanceRadius` 도착 판정)
- `OnMoveCompleted` 콜백 → `OnArrivedAtPosition` 연동
- `RecieveOrder(EPhase)` 명령 수신 인터페이스 구현

#### [FEAT] WeaponBase `2025-08-14`
- `AWeaponBase` 무기 베이스 클래스 설계
- `Muzzle` ArrowComponent 기반 발사 위치 정의
- `PullTrigger()` → `SpawnBullet()` + `PlayMuzzleVFX()` + SFX 파이프라인
- Niagara 머즐 VFX 스폰 (`UNiagaraFunctionLibrary::SpawnSystemAtLocation`)
- `AAllyCharacterBase`에 무기 장착: 소켓 `"Grip"` 어태치

---

### 2. 탄도 시스템

#### [FEAT] Bullet `2025-08-18`
- `ABulletActor` 설계 및 구현
- **물리 기반 탄도 모델** (Semi-implicit Euler):
  - 중력: `FVector(0, 0, GetWorld()->GetGravityZ())`
  - 공기 저항: `-Velocity * 0.8f` (선형 드래그)
  - 속도 → 위치 순차 적분
- 탄환 회전: `MakeRotFromZX(Velocity, UpVector)` 속도 방향 추적
- `lifetime` 변수로 3초 자동 소멸
- `DrawDebugLine` 기반 탄도 궤적 시각화 (`bDrawDebug`)

#### [FIX] Bullet Drag `2025-08-28`
- 공기 저항 계수 튜닝 및 안정화

#### Bullet 개선 `2025-10-10`
- `BulletActor.cpp` / `BulletActor.h` 추가 개선

---

### 3. 스토리/페이즈 관리 시스템

#### [FEAT] StoryManager `2025-08-24`
- `AStoryManager` 게임 흐름 관리자 설계
- `EPhase` UENUM: `Start / Phase1 / Phase2 / Ending`
- `AllyControllers` / `EnemyControllers` TWeakObjectPtr 배열로 AI 등록 관리
- `RegAICtrl()` → Ally/Enemy 자동 분류 등록
- `APhaseTrigger` 구현: BoxComponent 오버랩 시 페이즈 전환
- `ChangePhase()` 호출로 전체 AI에 일괄 명령 브로드캐스트

#### [FEAT] TriggerBox+Level `2025-08-25`
- `AAllyAIController::MoveDefenseLocation()` Phase별 거점 분기
  - Phase1 → `FirstDefensePoint`
  - Phase2 → `SecondDefensePoint`
- `PhaseTrigger` 오버랩 완성 및 레벨 배치
- `DefaultEngine.ini` NavMesh 설정 적용

#### [FEAT] Story-Phase `2025-08-31`
- 페이즈 전환 로직 안정화
- `HasRecieved` 플래그로 중복 명령 방지
- FSM 상태와 페이즈 연동 완성 (Shoot ↔ Cover 사이클)
- `ShootMontage` 애님 몽타주 연동

---

### 4. AirStrike 시스템

#### [FEAT] AirStrike `2025-09-01`
- `AAircraft` 액터 설계
- `AirStrike()` 호출 시 항공기 활성화 + 가시화
- `StoryManager::EndPhase()` → `pAircraft->AirStrike()` 연동
- `PrimaryActorTick.bStartWithTickEnabled = false` (비활성 대기)

#### [FEAT] Airstrike SFX `2025-09-01`
- 항공기 사운드 큐 추가 (`sound` UPROPERTY)
- `UGameplayStatics::PlaySound2D` 재생

#### [FEAT] ally muzzle sfx `2025-09-01`
- `WeaponBase` 머즐 사운드 추가
- `muzzleSFX` 사운드 큐 연동

---

## 💡 핵심 기술 포인트

| 기술 | 내용 |
|------|------|
| **AI FSM** | Tick 기반 상태 머신, Cover/Shoot 사이클, 쿨타임 관리 |
| **AIController** | OnMoveCompleted 콜백, MoveToLocation, StoryManager 자동 등록 |
| **탄도 물리** | Semi-implicit Euler 적분, 중력+선형 드래그, 속도 방향 회전 |
| **게임 흐름** | Observer 패턴 유사 페이즈 시스템, TWeakObjectPtr AI 풀 관리 |
| **VFX/SFX** | Niagara 파티클 스폰, Sound2D, 머즐 플래시 파이프라인 |
| **충돌/이동** | NavMesh MoveToLocation, AcceptanceRadius 도착 판정 |

---

## 📁 담당 주요 파일

```
Source/COD/
├── Public/Ally/
│   ├── AllyCharacterBase.h      - 아군 AI 기반 캐릭터
│   ├── AllyAIController.h       - AI 컨트롤러
│   ├── WeaponBase.h             - 무기 베이스
│   └── BulletActor.h            - 탄환 물리
├── Private/Ally/
│   ├── AllyCharacterBase.cpp    - FSM Tick 구현
│   ├── AllyAIController.cpp     - 거점 이동/페이즈 명령
│   ├── WeaponBase.cpp           - 발사/VFX/SFX
│   └── BulletActor.cpp          - 탄도 물리 연산
├── Public/
│   ├── StoryManager.h           - 게임 흐름 관리자
│   ├── PhaseTrigger.h           - 페이즈 트리거
│   └── Aircraft.h               - 항공기 액터
└── Private/
    ├── StoryManager.cpp         - 페이즈 브로드캐스트
    ├── PhaseTrigger.cpp         - 오버랩 → 페이즈 전환
    └── Aircraft.cpp             - AirStrike 활성화
```

---

**작성일**: 2026년 03월 06일

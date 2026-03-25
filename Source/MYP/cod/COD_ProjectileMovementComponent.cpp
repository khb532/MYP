#include "MYP.h"
#include "COD_ProjectileMovementComponent.h"

/*
 * ============================================================
 *  탄도 공학 설계 메모 (프로토타입 v1)
 * ============================================================
 *
 *  [ 기본 철학 ]
 *  - 발사체는 매 프레임 "자신이 가진 속도벡터"로 스스로 움직인다.
 *  - SetActorLocation 같은 좌표 직접 수정 방식 사용 안 함.
 *  - 엔진 MovementComponent 상속 없이 UActorComponent에서 직접 구축.
 *  - Chaos와의 통신은 SweepSingleByChannel 단일 진입점으로 제한.
 *
 *  [ 프로토타입 적용 탄도식: 중력만 ]
 *
 *  매 프레임 속도벡터에 중력 가속도를 적분한다.
 *
 *      V(t + dt) = V(t) + A * dt
 *
 *      A = { 0, 0, -GravityScale * 980.0f }    (단위: cm/s²)
 *
 *  이동 델타:
 *
 *      Delta = V(t) * dt
 *
 *  이후 Delta 방향으로 SweepSingleByChannel 을 쏴서
 *  충돌 여부를 판정한 뒤 위치를 확정한다.
 *
 *  [ 미사용 (추후 확장) ]
 *  - AirDrag : 공기저항 (현재 헤더에 선언만 된 상태)
 *  - 스핀 드리프트, 바람 등
 *
 * ============================================================
 */


UCOD_ProjectileMovementComponent::UCOD_ProjectileMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCOD_ProjectileMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// TODO [BP-1] Owner 유효성 검사
	// AActor* Owner = GetOwner();
	// if (!IsValid(Owner)) { LOGERRORF(TEXT("Owner is null")); SetComponentTickEnabled(false); return; }

	// TODO [BP-2] UpdatedComponent 바인딩
	// UpdatedComponent = Owner->GetRootComponent();
	// if (!UpdatedComponent) { LOGERRORF(TEXT("RootComponent is null")); SetComponentTickEnabled(false); return; }

	// TODO [BP-3] 초기 속도 복사
	// Velocity = InitVelocity;
}

void UCOD_ProjectileMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// TODO [TICK-1] 유효성 검사
	// if (!IsValid(UpdatedComponent) || !IsValid(GetOwner()) || !GetWorld()) return;

	// TODO [TICK-2] 물리 적분
	// FVector Acceleration = FVector(0.f, 0.f, -GravityScale * 980.0f);
	// Velocity += Acceleration * DeltaTime;
	// FVector Delta  = Velocity * DeltaTime;
	// FVector NewPos = UpdatedComponent->GetComponentLocation() + Delta;

	// TODO [TICK-3] SweepSingleByChannel
	// FCollisionShape SweepShape = FCollisionShape::MakeSphere(5.0f);
	// FCollisionQueryParams QueryParams;
	// QueryParams.AddIgnoredActor(GetOwner());
	// QueryParams.bTraceComplex = false;
	//
	// FHitResult HitResult;
	// bool bHit = GetWorld()->SweepSingleByChannel(
	//     HitResult, UpdatedComponent->GetComponentLocation(), NewPos,
	//     FQuat::Identity, ECC_Visibility, SweepShape, QueryParams);

	// TODO [TICK-4] HitResult 처리
	// if (bHit)
	// {
	//     LOGMSGF(TEXT("Hit: %s"), HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("None"));
	//     GetOwner()->Destroy();
	//     return;
	// }
	// UpdatedComponent->SetWorldLocation(NewPos, false, nullptr, ETeleportType::None);
}


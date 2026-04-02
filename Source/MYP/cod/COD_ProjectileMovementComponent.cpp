#include "MYP.h"
#include "COD_ProjectileMovementComponent.h"
#include "DrawDebugHelpers.h"


/*
 * ============================================================
 *  탄도 공학 설계 메모 (프로토타입 v1)
 * ============================================================
 *  - 발사체는 매 프레임 "자신이 가진 속도벡터"로 스스로 움직인다.
 *
 *  [ 프로토타입 적용 탄도식: 중력만 ]
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

	// [BP-1] Owner 유효성 검사
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		LOGERRORF(TEXT("Not Detected Owner"));
		SetComponentTickEnabled(false);
		return;
	}

	// [BP-2] UpdatedComponent 바인딩
	UpdatedComponent = Owner->GetRootComponent();
	if (!IsValid(UpdatedComponent))
	{
		LOGERRORF(TEXT("Not  Set UpdatedComponent"));
		SetComponentTickEnabled(false);
		return;
	}
	
	// [BP-3] 초기 속도 복사
	Velocity = GetOwner()->GetActorForwardVector() * InitSpeed;
}

void UCOD_ProjectileMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// [TICK-1] 유효성 검사
	if (!IsValid(UpdatedComponent))
	{
		LOGERRORF(TEXT("Not Detected UpdatedComponent"));
		return;
	}
	
	if (!IsValid(GetOwner())) 
	{
		LOGERRORF(TEXT("Not Detected Owner"));
		return;
	}
		
	if (!IsValid(GetWorld()))
	{
		LOGERRORF(TEXT("Not Detected World"));
		return;
	}
	

	// [TICK-2] 물리 적분
	FVector Delta = Velocity * DeltaTime;
	Velocity += ApplyGravity() * DeltaTime;
	FVector NewPos = UpdatedComponent->GetComponentLocation() + Delta;
	
	// [TICK-3] SweepSingleByChannel
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.bTraceComplex = false;
	
	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, UpdatedComponent->GetComponentLocation(), NewPos, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(5.f), Params);

	// [TICK-4] HitResult 처리
	if(bHit)
	{
		if (OnHitDelegate.IsBound())
			OnHitDelegate.Broadcast(HitResult);
		else
		{
			LOGERRORF(TEXT("Hitting Actor Not Bound"));
		}
	}
	else
	{
		DrawDebugLine(GetWorld(), UpdatedComponent->GetComponentLocation(), NewPos, FColor::Red, false, 2.f);
		UpdatedComponent->SetWorldLocation(NewPos, false, nullptr, ETeleportType::None);
		UpdatedComponent->SetWorldRotation(CalcRotation(Velocity), false, nullptr, ETeleportType::None);
	}
}

FVector UCOD_ProjectileMovementComponent::ApplyGravity()
{
	FVector Accel = {0.f, 0.f, -GravityScale * 980.f};
	return Accel;
}

FRotator UCOD_ProjectileMovementComponent::CalcRotation(FVector _Velocity)
{
	// 회전 동기화
	FRotator Rot;
	FVector Forward = _Velocity.GetSafeNormal();
	FVector Up = FVector::UpVector;
	FVector Right = FVector::CrossProduct(Up, Forward).GetSafeNormal();
	Up = FVector::CrossProduct(Forward, Right).GetSafeNormal();
	FMatrix m = FMatrix(Forward, Right, Up, FVector::ZeroVector);
	Rot = m.Rotator();
	
	return Rot;
}


#include "COD_ProjMoveComponent_Base.h"
#include "MYP.h"
#include "DrawDebugHelpers.h"
#include "MYP_TestGameMode.h"
#include "VectorUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ProfilingWidget.h"

UCOD_ProjMoveComponent_Base::UCOD_ProjMoveComponent_Base()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCOD_ProjMoveComponent_Base::BeginPlay()
{
	Super::BeginPlay();

	// Owner 유효성 검사
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		LOGERRORF(TEXT("Not Detected Owner"));
		SetComponentTickEnabled(false);
		return;
	}

	// UpdatedComponent 바인딩
	UpdatedComponent = Owner->GetRootComponent();
	if (!IsValid(UpdatedComponent))
	{
		LOGERRORF(TEXT("Not  Set UpdatedComponent"));
		SetComponentTickEnabled(false);
		return;
	}
	
	GM = Cast<AMYP_TestGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
}

void UCOD_ProjMoveComponent_Base::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (IsValid(GM) && IsValid(GetOwner()))
	{
		if (UProfilingWidget* Widget = GM->GetProfilingWidget())
			Widget->RemoveChannelByName(FString::Printf(TEXT("[%s_Velocity(m/s)]"), *GetOwner()->GetName()));
	}
}

void UCOD_ProjMoveComponent_Base::TickComponent(float DeltaTime, ELevelTick TickType,
                                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// LOGMSGF(TEXT("Tick 동작중 Velocity: %s"), *Velocity.ToString());
	

	// 유효성 검사
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
	

	// 물리 적분
	Velocity += ComputeAcceleration(DeltaTime) * DeltaTime;
	
	if (IsValid(GM))
		PrintProfile(GM->GetProfilingWidget(), Velocity.Size() / 100.f,
	FString::Printf(TEXT("[%s_Velocity(m/s)]"), *GetOwner()->GetName()), GetOwner());
	
	FVector Delta = Velocity * DeltaTime;
	FVector NewPos = UpdatedComponent->GetComponentLocation() + Delta;
	
	// SweepSingleByChannel
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.bTraceComplex = false;
	
	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, UpdatedComponent->GetComponentLocation(), NewPos, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(5.f), Params);

	// 지면(Z=0) 이탈 처리
	if (NewPos.Z <= 0.f)
	{
		Velocity = FVector::ZeroVector;
		SetComponentTickEnabled(false);
		return;
	}

	// Hit
	if(bHit)
	{
		LOGWARNF(TEXT("Hit 발생: %s"), *GetNameSafe(HitResult.GetActor()));
		DrawDebugLine(GetWorld(), UpdatedComponent->GetComponentLocation(), HitResult.ImpactPoint, FColor::Red, false, 2.f);
		UpdatedComponent->SetWorldLocation(HitResult.ImpactPoint, false, nullptr, ETeleportType::None);
		
		if (OnHitDelegate.IsBound())
			OnHitDelegate.Broadcast(HitResult);
		else
		{
			LOGERRORF(TEXT("Hitting Actor Not Bound"));
		}

		Velocity = FVector::ZeroVector;
		SetComponentTickEnabled(false);
	}
	else
	{
		TrajectoryPoints.Add(NewPos);
		DrawDebugLine(GetWorld(), UpdatedComponent->GetComponentLocation(), NewPos, FColor::Red, false, 2.f);
		UpdatedComponent->SetWorldLocation(NewPos, false, nullptr, ETeleportType::None);
		
		if (bSyncRot)
			UpdatedComponent->SetWorldRotation(SyncRotation(Velocity), false, nullptr, ETeleportType::None);
	}
}

void UCOD_ProjMoveComponent_Base::InitBulletData(float _Mass, float _Cd, float _Area, float _MuzzleVelocity, float _WeaponMultiplier)
{
	Mass = _Mass;
	Cd = _Cd;
	CrossSectionArea = _Area;
	Velocity = GetOwner()->GetActorForwardVector() * (_MuzzleVelocity * _WeaponMultiplier);
	
}

FVector UCOD_ProjMoveComponent_Base::ComputeAcceleration(float DeltaTime)
{
	return FVector(0.f, 0.f, -980.f);
}

FRotator UCOD_ProjMoveComponent_Base::SyncRotation(FVector _Velocity)
{
	// return _Velocity.GetSafeNormal().Rotation();
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


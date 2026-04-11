
#include "BulletBase.h"

#include "COD_ProjMoveComponent_Base.h"


ABulletBase::ABulletBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABulletBase::BeginPlay()
{
	Super::BeginPlay();

	// WeaponMultiplier는 Weapon이 InitWithBulletData 호출 시 전달
	if (UCOD_ProjMoveComponent_Base* MoveComp = FindComponentByClass<UCOD_ProjMoveComponent_Base>())
	{
		MoveComp->OnHitDelegate.AddDynamic(this, &ABulletBase::OnMovementHit);
	}
}

void ABulletBase::OnMovementHit(const FHitResult& HitResult)
{
	if (bHit) return;
	bHit = true;
	
	OnBulletHit(HitResult);
}

void ABulletBase::OnBulletHit(const FHitResult& HitResult)
{
	BP_OnBulletHit(HitResult);
	
	// Apply Damage
	
	// Destroy
	FTimerHandle DestroyTimer;
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &ABulletBase::DeferredDestroy, DestructionDelay, false);
	
}

void ABulletBase::DeferredDestroy()
{
	Destroy();
}

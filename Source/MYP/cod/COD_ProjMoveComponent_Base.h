#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "COD_ProjMoveComponent_Base.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBulletHitDelegate, const FHitResult&, Hit);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYP_API UCOD_ProjMoveComponent_Base : public UActorComponent
{
	GENERATED_BODY()

	/* Method */
public:
	UCOD_ProjMoveComponent_Base();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void InitBulletData(float  _Mass, float _Cd, float _Area, float _MuzzleVelocity, float _WeaponMultiplier = 1.f);
	
protected:
	virtual FVector ComputeAcceleration(float DeltaTime);
	
private:
	FRotator SyncRotation(FVector _Velocity);

	
	
	/* Field */
public:	
	UPROPERTY(BlueprintAssignable)
	FOnBulletHitDelegate OnHitDelegate;

protected:
	// 현재 프레임 속도 벡터 (매 Tick마다 물리 적분으로 갱신됨)
	FVector Velocity = FVector::ZeroVector;
	
	float Mass = 1.f;
	
	float Cd = 0.3f;
	
	float CrossSectionArea = 1.f;

private:
	// 실제로 이동시킬 대상 컴포넌트 (BeginPlay에서 Owner의 RootComponent로 바인딩)
	UPROPERTY()
	TObjectPtr<USceneComponent> UpdatedComponent = nullptr;
	
	TArray<FVector> TrajectoryPoints;
	
};

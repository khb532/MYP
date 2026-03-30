#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "COD_ProjectileMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitDelegate, const FHitResult&, HitResult);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYP_API UCOD_ProjectileMovementComponent : public UActorComponent
{
	GENERATED_BODY()

	/* Method */
public:
	UCOD_ProjectileMovementComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	
	/* Field */
public:
	// 발사 초기 속도 벡터 (BeginPlay에서 Velocity에 복사됨)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile")
	float InitSpeed = 2000.f;

	// 중력 배율 (1.0 = 기본 중력 980cm/s², 0이면 무중력)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile")
	float GravityScale = 1.0f;

	// 공기저항 계수 (0.0 ~ 1.0, 클수록 빠르게 감속)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile")
	float AirDrag = 0.01f;
	
	UPROPERTY(BlueprintAssignable)
	FOnHitDelegate OnHitDelegate;

private:
	// 현재 프레임 속도 벡터 (매 Tick마다 물리 적분으로 갱신됨)
	FVector Velocity = FVector::ZeroVector;

	// 실제로 이동시킬 대상 컴포넌트 (BeginPlay에서 Owner의 RootComponent로 바인딩)
	UPROPERTY()
	TObjectPtr<USceneComponent> UpdatedComponent = nullptr;
	
	
};

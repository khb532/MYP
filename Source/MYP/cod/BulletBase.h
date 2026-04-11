#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletBase.generated.h"

UCLASS(Abstract)
class MYP_API ABulletBase : public AActor
{
	GENERATED_BODY()

	/* Method */
public:
	ABulletBase();
	
	UFUNCTION()
	void OnMovementHit(const FHitResult& HitResult);
	
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnBulletHit"))
	void BP_OnBulletHit(const FHitResult& HitResult);
	
	
protected:
	virtual void BeginPlay() override;
	virtual void OnBulletHit(const FHitResult& HitResult);
	void DeferredDestroy();

private:
	
	
	
	
	/* Field */
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics")
	float Mass = 4.f;	//	질량(g)
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics")
	float CD = 0.3f;	// 항력계수
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics")
	float MuzzleVelocity = 90000.f;		// 총구속도(cm/s)
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics")
	float CrossSectionArea = 1.f;		// 단면적(cm²)
	
	//	Damage
	//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics")
	//	float HitDamage = 25.f;

	//	Destruction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destruction")
	float DestructionDelay = 5.f;
	
	
	
protected:
	bool bHit = false;	
	
	
private:
	
	
	
	
	
};

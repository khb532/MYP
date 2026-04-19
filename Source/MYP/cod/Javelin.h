#pragma once

#include "CoreMinimal.h"
#include "BulletBase.h"
#include "Javelin.generated.h"

UCLASS()
class MYP_API AJavelin : public ABulletBase
{
	GENERATED_BODY()

	/* Method */
public:
	AJavelin();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	/* Field */
public:
	UPROPERTY(VisibleAnywhere, Category="Components")
	class UCOD_ProjMoveComponent_Javelin* ProjectileMovement;

private:
	
	
};

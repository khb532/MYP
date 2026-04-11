#pragma once

#include "CoreMinimal.h"
#include "BulletBase.h"
#include "SmallCaliber.generated.h"

class UCOD_ProjMoveComponent_Rifle;

UCLASS()
class MYP_API ASmallCaliber : public ABulletBase
{
	GENERATED_BODY()

	/* Method */
public:
	ASmallCaliber();

	
	
protected:
	
	
	

private:
	
	
	
	
	/* Field */
public:
	
	


private:
	UPROPERTY(VisibleAnywhere, Category="Components")
	UCOD_ProjMoveComponent_Rifle* ProjectileMovement;
	
	
	
};

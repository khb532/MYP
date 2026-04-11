#pragma once

#include "CoreMinimal.h"
#include "COD_ProjMoveComponent_Base.h"
#include "COD_ProjMoveComponent_Rifle.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYP_API UCOD_ProjMoveComponent_Rifle : public UCOD_ProjMoveComponent_Base
{
	GENERATED_BODY()

	/* Method */
public:
	UCOD_ProjMoveComponent_Rifle();

protected:
	virtual FVector ComputeAcceleration(float DeltaTime) override;

private:
	
	
	
	/* Field */
public:
	// 공기밀도 (해수면 기준 g/cm³)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics")
	float AirDensity = 0.001225f;

	// V² 수치 폭발 방지용 게임 스케일 튜닝값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics")
	float DragScale = 1.f;
	
	//	중력 스케일
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics")
	float GravityScale = 1.f;

private:
	
	
	
};

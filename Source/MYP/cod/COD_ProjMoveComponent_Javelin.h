#pragma once

#include "CoreMinimal.h"
#include "COD_ProjMoveComponent_Base.h"
#include "COD_ProjMoveComponent_Javelin.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYP_API UCOD_ProjMoveComponent_Javelin : public UCOD_ProjMoveComponent_Base
{
	GENERATED_BODY()

	/* Method */
public:
	UCOD_ProjMoveComponent_Javelin();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual FVector ComputeAcceleration(float DeltaTime) override;
	virtual void InitBulletData(float _Mass, float _Cd, float _Area, float _MuzzleVelocity, float _WeaponMultiplier = 1.f);

private:
	// 탄두 회전 엔진 추진 함수

	// 메인 엔진 추진 함수
	void MainEngineBoost(float DeltaTime);


	/* Field */
public:
	// 공기밀도 (해수면 기준 g/cm³)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics")
	float AirDensity = 0.001225f;

	// V² 수치 폭발 방지용 게임 스케일 튜닝값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics")
	float DragScale = 1.f;

	// 중력 스케일
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics")
	float GravityScale = 1.f;

	// 연소 지속 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics|Rocket")
	float BurnTime = 5.f;

	// 추력 가속도 (cm/s²)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ballistics|Rocket")
	float ThrustAccel = 5000.f;

private:
	float BurnElapsed = 0.f;
	FVector LaunchDir = FVector::ZeroVector;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlaterActor.generated.h"

class UParticleComputeComponent;

UCLASS()
class MYP_API APlaterActor : public AActor
{
	GENERATED_BODY()

	/* Method */
public:
	APlaterActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	
	
private:
	
	
	
	
	
	/* Field */
public:
	
	
	
	
private:
	UPROPERTY(VisibleAnywhere)
	UParticleComputeComponent* ComputeComponent;
	
	UPROPERTY(EditAnywhere, Category=Particle)
	int32 ParticleCount = 100;
	
	
	
	
};

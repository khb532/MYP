#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DistanceSign.generated.h"

class UTextRenderComponent;

UCLASS()
class MYP_API ADistanceSign : public AActor
{
	GENERATED_BODY()

	/* Method */
public:
	ADistanceSign();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	
	
	
	
	/* Field */
public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTextRenderComponent> TextRender;

	
	
private:
	
	
	
	
};

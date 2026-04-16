#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MYP_TestGameMode.generated.h"

class UProfilingWidget;

UCLASS()
class MYP_API AMYP_TestGameMode : public AGameModeBase
{
	GENERATED_BODY()

	/* Method */
public:
	virtual void BeginPlay() override;

	UProfilingWidget* GetProfilingWidget() const { return ProfilingWidget; }

private:


	/* Field */
public:
	UPROPERTY(EditAnywhere, Category="Debug")
	TSubclassOf<UProfilingWidget> ProfilingWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UProfilingWidget> ProfilingWidget;

};

#include "MYP_TestGameMode.h"
#include "Misc/ProfilingWidget.h"
#include "Kismet/GameplayStatics.h"

void AMYP_TestGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ProfilingWidgetClass)
	{
		ProfilingWidget = CreateWidget<UProfilingWidget>(
			UGameplayStatics::GetPlayerController(GetWorld(), 0), ProfilingWidgetClass);
		ProfilingWidget->AddToViewport(99);
	}
}

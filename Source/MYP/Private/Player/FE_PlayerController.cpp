
#include "Player/FE_PlayerController.h"

#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"

void AFE_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	TArray<AActor*> foundcameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraActor::StaticClass(), FName("Default"), foundcameras);
	
	if (!foundcameras.IsEmpty())
	{
		SetViewTarget(foundcameras[0]);
	}
}

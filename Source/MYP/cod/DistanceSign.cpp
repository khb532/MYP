
#include "DistanceSign.h"

#include "Components/TextRenderComponent.h"


ADistanceSign::ADistanceSign()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetupAttachment(Root);
	
	TextRender = CreateDefaultSubobject<UTextRenderComponent>("TextRender");
	TextRender->SetupAttachment(Root);
}

void ADistanceSign::BeginPlay()
{
	Super::BeginPlay();

	float Loc = GetActorLocation().X / 100.f;
	FNumberFormattingOptions Opts;
	Opts.MinimumFractionalDigits = 1;
	Opts.MaximumFractionalDigits = 1;
	FText Result = FText::Format(INVTEXT("{0}M"), FText::AsNumber(Loc, &Opts));
	TextRender->SetText(Result);
	
}

void ADistanceSign::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


#include "ParticleComputeComponent.h"

#include "MYP.h"
#include "Kismet/KismetRenderingLibrary.h"


UParticleComputeComponent::UParticleComputeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UParticleComputeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void UParticleComputeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}

void UParticleComputeComponent::InitializeBuffers(int32 particles)
{
	ParticleCount = particles;
	
	TextureSize = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(particles)));
	
	PositionRT = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), TextureSize, TextureSize, RTF_RGBA16f);
	ColorRT = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), TextureSize, TextureSize, RTF_RGBA16f);
	if (!PositionRT || !ColorRT)
	{
		LOGERROR();
		return;
	}
	
	BytecodeData.Reserve(4096);
	ConstantData.Reserve(256);
	
	LOGMSGF(TEXT("Initialized %d particles, TextureSize=%d"), ParticleCount, TextureSize);
}

void UParticleComputeComponent::UploadBytecode(const TArray<uint32>& bytecode, const TArray<float>& constants)
{
	
}

void UParticleComputeComponent::ExecuteSimulation(float deltatime)
{
	
}


#include "ParticleComputeComponent.h"

#include "MYP.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "RHIResources.h"
#include "RenderResource.h"


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
	// CPU 백업 저장
	BytecodeData = bytecode;
	ConstantData = constants;

	// 람다 캡처용 복사본
	TArray<uint32> BytecodeCopy = bytecode;
	TArray<float> ConstantsCopy = constants;

	// Render Thread에서 GPU 버퍼 생성 및 업로드
	ENQUEUE_RENDER_COMMAND(UploadParticleBytecode)(
		[this, BytecodeCopy, ConstantsCopy](FRHICommandListImmediate& RHICmdList)
		{
			// 1. Bytecode 버퍼 생성
			if (BytecodeCopy.Num() > 0)
			{
				FRHIBufferCreateDesc BytecodeDesc
				(
					TEXT("ParticleBytecodeBuffer"),
					BytecodeCopy.Num() * sizeof(uint32),
					sizeof(uint32),
					EBufferUsageFlags::ShaderResource | EBufferUsageFlags::StructuredBuffer
				);

				BytecodeBufferRHI = RHICmdList.CreateBuffer(BytecodeDesc);

				// 데이터 업로드
				void* MappedData = RHICmdList.LockBuffer(BytecodeBufferRHI, 0, BytecodeCopy.Num() * sizeof(uint32), RLM_WriteOnly);
				FMemory::Memcpy(MappedData, BytecodeCopy.GetData(), BytecodeCopy.Num() * sizeof(uint32));
				RHICmdList.UnlockBuffer(BytecodeBufferRHI);
			}

			// 2. Constants 버퍼 생성
			if (ConstantsCopy.Num() > 0)
			{
				FRHIBufferCreateDesc ConstantsDesc
				(
					TEXT("ParticleConstantsBuffer"),
					ConstantsCopy.Num() * sizeof(float),
					sizeof(float),
					EBufferUsageFlags::ShaderResource | EBufferUsageFlags::StructuredBuffer
				);

				ConstantsBufferRHI = RHICmdList.CreateBuffer(ConstantsDesc);

				// 데이터 업로드
				void* MappedData = RHICmdList.LockBuffer(ConstantsBufferRHI, 0, ConstantsCopy.Num() * sizeof(float), RLM_WriteOnly);
				FMemory::Memcpy(MappedData, ConstantsCopy.GetData(), ConstantsCopy.Num() * sizeof(float));
				RHICmdList.UnlockBuffer(ConstantsBufferRHI);
			}
		}
	);

	LOGMSGF(TEXT("Uploaded %d bytecode instructions, %d constants"), bytecode.Num(), constants.Num());
}

void UParticleComputeComponent::ExecuteSimulation(float deltatime)
{
	
}


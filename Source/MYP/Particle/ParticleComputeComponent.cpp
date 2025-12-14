#include "ParticleComputeComponent.h"

#include "MYP.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "RHIResources.h"
#include "RenderResource.h"
#include "ParticleSimulationShader.h"
#include "RenderGraphBuilder.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"


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
			FRDGBuilder GraphBuilder(RHICmdList);

			// 1. Bytecode 버퍼 생성
			if (BytecodeCopy.Num() > 0)
			{
				FRDGBufferDesc BytecodeDesc = FRDGBufferDesc::CreateStructuredDesc(sizeof(uint32), BytecodeCopy.Num());
				FRDGBufferRef BytecodeBuffer = GraphBuilder.CreateBuffer(BytecodeDesc, TEXT("ParticleBytecodeBuffer"));

				// 데이터 업로드
				GraphBuilder.QueueBufferUpload(BytecodeBuffer, BytecodeCopy.GetData(), BytecodeCopy.Num() * sizeof(uint32));

				// Pooled Buffer로 추출
				GraphBuilder.QueueBufferExtraction(BytecodeBuffer, &BytecodePooledBuffer);
			}

			// 2. Constants 버퍼 생성
			if (ConstantsCopy.Num() > 0)
			{
				FRDGBufferDesc ConstantsDesc = FRDGBufferDesc::CreateStructuredDesc(sizeof(float), ConstantsCopy.Num());
				FRDGBufferRef ConstantsBuffer = GraphBuilder.CreateBuffer(ConstantsDesc, TEXT("ParticleConstantsBuffer"));

				// 데이터 업로드
				GraphBuilder.QueueBufferUpload(ConstantsBuffer, ConstantsCopy.GetData(), ConstantsCopy.Num() * sizeof(float));

				// Pooled Buffer로 추출
				GraphBuilder.QueueBufferExtraction(ConstantsBuffer, &ConstantsPooledBuffer);
			}

			GraphBuilder.Execute();
		}
	);

	LOGMSGF(TEXT("Uploaded %d bytecode instructions, %d constants"), bytecode.Num(), constants.Num());
}

void UParticleComputeComponent::ExecuteSimulation(float deltatime)
{
	if (!PositionRT || !ColorRT || !BytecodePooledBuffer.IsValid() || !ConstantsPooledBuffer.IsValid())
	{
		return;
	}

	// 람다 캡처용 복사본
	FTextureRenderTargetResource* PositionRTResource = PositionRT->GameThread_GetRenderTargetResource();
	FTextureRenderTargetResource* ColorRTResource = ColorRT->GameThread_GetRenderTargetResource();
	TRefCountPtr<FRDGPooledBuffer> BytecodePooledBufferCopy = BytecodePooledBuffer;
	TRefCountPtr<FRDGPooledBuffer> ConstantsPooledBufferCopy = ConstantsPooledBuffer;
	uint32 ParticleCountCopy = ParticleCount;
	uint32 TextureSizeCopy = TextureSize;
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float DeltaTimeCopy = deltatime;

	ENQUEUE_RENDER_COMMAND(ExecuteParticleSimulation)(
		[PositionRTResource, ColorRTResource, BytecodePooledBufferCopy, ConstantsPooledBufferCopy,
		 ParticleCountCopy, TextureSizeCopy, CurrentTime, DeltaTimeCopy](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			// 1. RenderTarget을 RDG Texture로 등록
			FRDGTextureRef PositionTexture = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(PositionRTResource->GetRenderTargetTexture(), TEXT("PositionRT"))
			);
			FRDGTextureRef ColorTexture = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(ColorRTResource->GetRenderTargetTexture(), TEXT("ColorRT"))
			);

			// 2. Buffer를 RDG Buffer로 등록
			FRDGBufferRef BytecodeBuffer = GraphBuilder.RegisterExternalBuffer(BytecodePooledBufferCopy);
			FRDGBufferRef ConstantsBuffer = GraphBuilder.RegisterExternalBuffer(ConstantsPooledBufferCopy);

			// 3. Shader 파라미터 설정
			FParticleSimulationCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FParticleSimulationCS::FParameters>();
			PassParameters->BytecodeBuffer = GraphBuilder.CreateSRV(BytecodeBuffer);
			PassParameters->ConstantsBuffer = GraphBuilder.CreateSRV(ConstantsBuffer);
			PassParameters->PositionRT = GraphBuilder.CreateUAV(PositionTexture);
			PassParameters->ColorRT = GraphBuilder.CreateUAV(ColorTexture);
			PassParameters->ParticleCount = ParticleCountCopy;
			PassParameters->TextureSize = TextureSizeCopy;
			PassParameters->CurrentTime = CurrentTime;
			PassParameters->DeltaTime = DeltaTimeCopy;

			// 4. Compute Shader 실행
			TShaderMapRef<FParticleSimulationCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
			FIntVector GroupCount(FMath::DivideAndRoundUp(ParticleCountCopy, 64u), 1, 1);

			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("ParticleSimulation"),
				ComputeShader,
				PassParameters,
				GroupCount
			);

			// 5. RDG 실행
			GraphBuilder.Execute();
		}
	);
}


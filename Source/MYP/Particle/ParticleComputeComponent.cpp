#include "ParticleComputeComponent.h"

#include "MYP.h"
#include "Engine/TextureRenderTarget2D.h"
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

	// Position RenderTarget 생성 (UAV 쓰기 가능하도록)
	PositionRT = NewObject<UTextureRenderTarget2D>(this);
	if (!PositionRT)
	{
		LOGERRORF(TEXT("Failed to create PositionRT"));
		return;
	}
	PositionRT->RenderTargetFormat = RTF_RGBA16f;
	PositionRT->ClearColor = FLinearColor::Black;
	PositionRT->bAutoGenerateMips = false;
	PositionRT->bCanCreateUAV = true;  // Compute Shader UAV 쓰기 필수
	PositionRT->InitAutoFormat(TextureSize, TextureSize);
	PositionRT->UpdateResourceImmediate(true);

	// Color RenderTarget 생성 (UAV 쓰기 가능하도록)
	ColorRT = NewObject<UTextureRenderTarget2D>(this);
	if (!ColorRT)
	{
		LOGERRORF(TEXT("Failed to create ColorRT"));
		return;
	}
	ColorRT->RenderTargetFormat = RTF_RGBA16f;
	ColorRT->ClearColor = FLinearColor::Black;
	ColorRT->bAutoGenerateMips = false;
	ColorRT->bCanCreateUAV = true;  // Compute Shader UAV 쓰기 필수
	ColorRT->InitAutoFormat(TextureSize, TextureSize);
	ColorRT->UpdateResourceImmediate(true);

	BytecodeData.Reserve(4096);
	ConstantData.Reserve(256);

	LOGMSGF(TEXT("Initialized %d particles, TextureSize=%d x %d"), ParticleCount, TextureSize, TextureSize);
}

void UParticleComputeComponent::UploadBytecode(const TArray<uint32>& bytecode, const TArray<float>& constants)
{
	// CPU 백업 저장
	BytecodeData = bytecode;
	ConstantData = constants;

	// 디버그: 첫 5개 바이트코드 출력
	FString BytecodeStr;
	for (int32 i = 0; i < FMath::Min(5, BytecodeData.Num()); ++i)
	{
		BytecodeStr += FString::Printf(TEXT("%d "), BytecodeData[i]);
	}
	LOGMSGF(TEXT("[UploadBytecode] First bytecodes: %s"), *BytecodeStr);

	// ⚠️ 기존 GPU 버퍼 존재 여부 확인 (RenderTarget 재생성 판단용)
	bool bNeedRecreateRT = BytecodePooledBuffer.IsValid();

	// ⚠️ 기존 GPU 버퍼 명시적 해제 (풀 재사용 방지)
	// FRDGPooledBuffer가 같은 주소를 재사용하면 RDG가 업로드를 스킵할 수 있음
	if (BytecodePooledBuffer.IsValid())
	{
		LOGMSGF(TEXT("[UploadBytecode] 기존 GPU 버퍼 해제: %p"), BytecodePooledBuffer.GetReference());
		BytecodePooledBuffer.SafeRelease();
	}
	if (ConstantsPooledBuffer.IsValid())
	{
		ConstantsPooledBuffer.SafeRelease();
	}

	// ⚠️ RenderTarget 재생성 (ReadPixels 캐시 무효화)
	// ColorRT는 업데이트되지만 PositionRT는 캐시를 읽는 문제 해결
	// 초기 업로드 시에는 건너뛰기 (bNeedRecreateRT == false)
	if (PositionRT && ColorRT && bNeedRecreateRT)
	{
		LOGMSGF(TEXT("[UploadBytecode] RenderTarget 재생성으로 캐시 무효화"));

		// Position RenderTarget 재생성
		PositionRT = NewObject<UTextureRenderTarget2D>(this);
		PositionRT->RenderTargetFormat = RTF_RGBA16f;
		PositionRT->ClearColor = FLinearColor::Black;
		PositionRT->bAutoGenerateMips = false;
		PositionRT->bCanCreateUAV = true;
		PositionRT->InitAutoFormat(TextureSize, TextureSize);
		PositionRT->UpdateResourceImmediate(true);

		// Color RenderTarget 재생성
		ColorRT = NewObject<UTextureRenderTarget2D>(this);
		ColorRT->RenderTargetFormat = RTF_RGBA16f;
		ColorRT->ClearColor = FLinearColor::Black;
		ColorRT->bAutoGenerateMips = false;
		ColorRT->bCanCreateUAV = true;
		ColorRT->InitAutoFormat(TextureSize, TextureSize);
		ColorRT->UpdateResourceImmediate(true);
	}

	// ⚠️ FRDGPooledBuffer 방식은 폐기
	// ExecuteSimulation에서 BytecodeData/ConstantData를 직접 사용함

	LOGMSGF(TEXT("[GAME THREAD] Bytecode updated: %d instructions, %d constants"), bytecode.Num(), constants.Num());
}

void UParticleComputeComponent::ExecuteSimulation(float deltatime)
{
	if (!PositionRT || !ColorRT || BytecodeData.Num() == 0 || ConstantData.Num() == 0)
	{
		return;
	}

	// 람다 캡처용 복사본
	FTextureRenderTargetResource* PositionRTResource = PositionRT->GameThread_GetRenderTargetResource();
	FTextureRenderTargetResource* ColorRTResource = ColorRT->GameThread_GetRenderTargetResource();

	// ⚠️ Pooled Buffer 대신 BytecodeData/ConstantData를 직접 복사 (매 프레임 업로드)
	TArray<uint32> BytecodeCopy = BytecodeData;
	TArray<float> ConstantsCopy = ConstantData;

	uint32 ParticleCountCopy = ParticleCount;
	uint32 TextureSizeCopy = TextureSize;
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float DeltaTimeCopy = deltatime;

	ENQUEUE_RENDER_COMMAND(ExecuteParticleSimulation)(
		[PositionRTResource, ColorRTResource, BytecodeCopy, ConstantsCopy,
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

			// 2. 매 프레임 새 버퍼 생성 및 업로드
			FRDGBufferDesc BytecodeDesc = FRDGBufferDesc::CreateStructuredDesc(sizeof(uint32), BytecodeCopy.Num());
			FRDGBufferRef BytecodeBuffer = GraphBuilder.CreateBuffer(BytecodeDesc, TEXT("ParticleBytecodeBuffer"));
			GraphBuilder.QueueBufferUpload(BytecodeBuffer, BytecodeCopy.GetData(), BytecodeCopy.Num() * sizeof(uint32), ERDGInitialDataFlags::None);

			FRDGBufferDesc ConstantsDesc = FRDGBufferDesc::CreateStructuredDesc(sizeof(float), ConstantsCopy.Num());
			FRDGBufferRef ConstantsBuffer = GraphBuilder.CreateBuffer(ConstantsDesc, TEXT("ParticleConstantsBuffer"));
			GraphBuilder.QueueBufferUpload(ConstantsBuffer, ConstantsCopy.GetData(), ConstantsCopy.Num() * sizeof(float), ERDGInitialDataFlags::None);

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

void UParticleComputeComponent::DebugPrintRenderTarget()
{
	if (!PositionRT || !ColorRT)
	{
		LOGERRORF(TEXT("RenderTarget not initialized"));
		return;
	}
	
	FTextureRenderTargetResource* PositionResource = PositionRT->GameThread_GetRenderTargetResource();
	FTextureRenderTargetResource* ColorResource = ColorRT->GameThread_GetRenderTargetResource();

	TArray<FColor> PositionData;
	TArray<FColor> ColorData;
	FIntRect Rect(0, 0, TextureSize, TextureSize);
	PositionResource->ReadPixels(PositionData, FReadSurfaceDataFlags(), Rect);
	ColorResource->ReadPixels(ColorData, FReadSurfaceDataFlags(), Rect);

	// 처음 3개 파티클만 출력
	LOGMSGF(TEXT("=== First 3 Particles ==="));
	for (int32 i = 0; i < FMath::Min(3, ParticleCount); ++i)
	{
		int32 x = i % TextureSize;
		int32 y = i / TextureSize;
		int32 Index = y * TextureSize + x;

		if (Index < PositionData.Num() && Index < ColorData.Num())
		{
			FColor PosPix = PositionData[Index];
			FColor ColPix = ColorData[Index];

			// RGBA16f → float 변환 (근사치)
			float PosX = PosPix.R / 255.0f * 1000.0f;  // 스케일 조정
			float PosY = PosPix.G / 255.0f * 1000.0f;
			float PosZ = PosPix.B / 255.0f * 1000.0f;

			LOGMSGF(TEXT("Particle[%d]: Pos(%.1f, %.1f, %.1f) PosRaw(%d,%d,%d,%d) Color(%d,%d,%d)"),
				i, PosX, PosY, PosZ, PosPix.R, PosPix.G, PosPix.B, PosPix.A, ColPix.R, ColPix.G, ColPix.B);
		}
	}
}


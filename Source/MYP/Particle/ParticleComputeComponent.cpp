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

	// 람다 캡처용 복사본 생성
	TArray<uint32> BytecodeCopy = bytecode;
	TArray<float> ConstantsCopy = constants;

	// RHI 직접 업로드 (RDG 우회)
	ENQUEUE_RENDER_COMMAND(UploadParticleBytecode)(
		[this, BytecodeCopy, ConstantsCopy](FRHICommandListImmediate& RHICmdList)
		{
			// 1. Bytecode 버퍼 초기화 및 업데이트
			if (BytecodeBuffer.Buffer)
			{
				// 기존 버퍼가 있으면 명시적으로 해제 (강제 재생성)
				BytecodeBuffer.Release();
				LOGMSGF(TEXT("[RENDER THREAD] BytecodeBuffer 기존 버퍼 해제"));
			}

			if (BytecodeCopy.Num() > 0)
			{
				uint32 BytecodeSize = BytecodeCopy.Num() * sizeof(uint32);

				// FRWBuffer 초기화: Structured Buffer로 설정 (UE 5.6 API)
				BytecodeBuffer.Initialize(
					RHICmdList,                 // RHICommandList (첫 번째 인자)
					TEXT("ParticleBytecodeBuffer"),
					sizeof(uint32),             // Stride
					BytecodeCopy.Num(),         // NumElements
					PF_R32_UINT,                // Format
					BUF_Static | BUF_ShaderResource  // RHI 플래그
				);

				// 데이터 업로드 (RHI Lock/Unlock 직접 사용)
				void* MappedData = RHICmdList.LockBuffer(BytecodeBuffer.Buffer, 0, BytecodeSize, RLM_WriteOnly);
				FMemory::Memcpy(MappedData, BytecodeCopy.GetData(), BytecodeSize);
				RHICmdList.UnlockBuffer(BytecodeBuffer.Buffer);

				LOGMSGF(TEXT("[RENDER THREAD] BytecodeBuffer 업로드 완료: %d instructions"), BytecodeCopy.Num());
			}

			// 2. Constants 버퍼도 동일하게 처리
			if (ConstantsBuffer.Buffer)
			{
				ConstantsBuffer.Release();
				LOGMSGF(TEXT("[RENDER THREAD] ConstantsBuffer 기존 버퍼 해제"));
			}

			if (ConstantsCopy.Num() > 0)
			{
				uint32 ConstantsSize = ConstantsCopy.Num() * sizeof(float);

				ConstantsBuffer.Initialize(
					RHICmdList,                 // RHICommandList (첫 번째 인자)
					TEXT("ParticleConstantsBuffer"),
					sizeof(float),
					ConstantsCopy.Num(),
					PF_R32_FLOAT,
					BUF_Static | BUF_ShaderResource
				);

				void* MappedData = RHICmdList.LockBuffer(ConstantsBuffer.Buffer, 0, ConstantsSize, RLM_WriteOnly);
				FMemory::Memcpy(MappedData, ConstantsCopy.GetData(), ConstantsSize);
				RHICmdList.UnlockBuffer(ConstantsBuffer.Buffer);

				LOGMSGF(TEXT("[RENDER THREAD] ConstantsBuffer 업로드 완료: %d constants"), ConstantsCopy.Num());
			}
		}
	);

	LOGMSGF(TEXT("[GAME THREAD] Bytecode updated: %d instructions, %d constants"), bytecode.Num(), constants.Num());
}

void UParticleComputeComponent::ExecuteSimulation(float deltatime)
{
	if (!PositionRT || !ColorRT || BytecodeData.Num() == 0 || ConstantData.Num() == 0)
	{
		return;
	}

	// 버퍼 유효성 검사 (UploadBytecode가 먼저 호출되어야 함)
	if (!BytecodeBuffer.Buffer || !ConstantsBuffer.Buffer)
	{
		LOGERRORF(TEXT("ExecuteSimulation: Buffers not initialized. Call UploadBytecode first."));
		return;
	}

	// 람다 캡처용 복사본
	FTextureRenderTargetResource* PositionRTResource = PositionRT->GameThread_GetRenderTargetResource();
	FTextureRenderTargetResource* ColorRTResource = ColorRT->GameThread_GetRenderTargetResource();

	// ⚠️ FRWBuffer SRV 캡처 (UE 5.6에서는 SRV를 직접 사용)
	FShaderResourceViewRHIRef BytecodeSRV = BytecodeBuffer.SRV;
	FShaderResourceViewRHIRef ConstantsSRV = ConstantsBuffer.SRV;

	uint32 ParticleCountCopy = ParticleCount;
	uint32 TextureSizeCopy = TextureSize;
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float DeltaTimeCopy = deltatime;

	ENQUEUE_RENDER_COMMAND(ExecuteParticleSimulation)(
		[PositionRTResource, ColorRTResource, BytecodeSRV, ConstantsSRV,
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

			// 2. FRWBuffer의 SRV를 Shader Parameter에 직접 바인딩 (RDG 우회)
			// RDG CreateSRV는 사용하지 않고, RHI SRV를 직접 전달
			// 이는 Legacy 방식이지만 FRWBuffer의 런타임 업데이트를 보장

			// 3. Shader 파라미터 설정
			FParticleSimulationCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FParticleSimulationCS::FParameters>();
			// RHI SRV를 RDG 파라미터에 직접 할당 (타입 호환 확인 필요)
			PassParameters->BytecodeBuffer = BytecodeSRV;
			PassParameters->ConstantsBuffer = ConstantsSRV;
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

	// ⚠️ RGBA16f 포맷을 제대로 읽기 위해 FFloat16Color 사용
	TArray<FFloat16Color> PositionData;
	TArray<FFloat16Color> ColorData;
	FIntRect Rect(0, 0, TextureSize, TextureSize);

	// ReadFloat16Pixels로 RGBA16f 데이터를 정확히 읽음
	// UE 5.6 API: ReadFloat16Pixels(OutData, Flags, Rect)
	PositionResource->ReadFloat16Pixels(PositionData, FReadSurfaceDataFlags(), Rect);
	ColorResource->ReadFloat16Pixels(ColorData, FReadSurfaceDataFlags(), Rect);

	// 처음 3개 파티클만 출력
	LOGMSGF(TEXT("=== First 3 Particles ==="));
	for (int32 i = 0; i < FMath::Min(3, ParticleCount); ++i)
	{
		int32 x = i % TextureSize;
		int32 y = i / TextureSize;
		int32 Index = y * TextureSize + x;

		if (Index < PositionData.Num() && Index < ColorData.Num())
		{
			FFloat16Color PosPix = PositionData[Index];
			FFloat16Color ColPix = ColorData[Index];

			// FFloat16 → float 변환 (정확한 값)
			float PosX = PosPix.R.GetFloat();
			float PosY = PosPix.G.GetFloat();
			float PosZ = PosPix.B.GetFloat();

			float ColR = ColPix.R.GetFloat();
			float ColG = ColPix.G.GetFloat();
			float ColB = ColPix.B.GetFloat();

			LOGMSGF(TEXT("Particle[%d]: Pos(%.2f, %.2f, %.2f) Color(%.2f, %.2f, %.2f)"),
				i, PosX, PosY, PosZ, ColR, ColG, ColB);
		}
	}
}


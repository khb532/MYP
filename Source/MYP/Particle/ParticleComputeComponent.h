#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RHI.h"
#include "RenderGraphResources.h"
#include "RenderResource.h"  // FRWBuffer를 위한 헤더
#include "ParticleComputeComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYP_API UParticleComputeComponent : public UActorComponent
{
	GENERATED_BODY()

	/* Method */
public:
	UParticleComputeComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//	Init
	//	Create RenderTarget, Alloc Buffer
	void InitializeBuffers(int32 particles);
	
	//	Upload Bytecode at GPU
	void UploadBytecode(const TArray<uint32>& bytecode, const TArray<float>& constants);
	
	//	Execute Compute Shader Tick
	void ExecuteSimulation(float deltatime);

	//	return Texture for Niagara read
	UTextureRenderTarget2D* GetPositionTexture() const { return PositionRT; }
	UTextureRenderTarget2D* GetColorTexture() const { return ColorRT; }
	
	void DebugPrintRenderTarget();
	
	
	
private:
	
	
	
	
	
	/* Field */
public:
	//	GPU 메모리에 유지되는 RenderTarget (zero-copy)
	UPROPERTY()
	UTextureRenderTarget2D* PositionRT;
	
	UPROPERTY()
	UTextureRenderTarget2D* ColorRT;
	
	//	ByteCode & Constant Buffer
	TArray<uint32> BytecodeData;
	TArray<float> ConstantData;
	
	int32 ParticleCount;
	int32 TextureSize;	//	sqrt(ParticleCount)
	
	
	
private:
	//	GPU Buffer (RHI 직접 관리 - RDG Pooling 우회)
	//	FRWBuffer는 FRHIBuffer와 SRV/UAV를 함께 관리하는 Wrapper
	FRWBuffer BytecodeBuffer;
	FRWBuffer ConstantsBuffer;
	
	
	
	
};

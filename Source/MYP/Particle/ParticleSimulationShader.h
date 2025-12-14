#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"

//	Particle Simulation Compute Shader
class FParticleSimulationCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FParticleSimulationCS);
	SHADER_USE_PARAMETER_STRUCT(FParticleSimulationCS, FGlobalShader)
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		//	입력 버퍼 (ReadOnly)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<uint>, BytecodeBuffer)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float>, ConstantsBuffer)
	
		//	출력 텍스처 (R/W)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, PositionRT)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, ColorRT)
	
		//	Parameter
		SHADER_PARAMETER(uint32, ParticleCount)
		SHADER_PARAMETER(uint32, TextureSize)
		SHADER_PARAMETER(float, CurrentTime)
		SHADER_PARAMETER(float, DeltaTime)
	END_SHADER_PARAMETER_STRUCT()
	
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
		
};

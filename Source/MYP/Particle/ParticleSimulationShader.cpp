#include "ParticleSimulationShader.h"
#include "ShaderCompilerCore.h"

//	Shader 구현 및 .usf 파일 매핑
IMPLEMENT_GLOBAL_SHADER(FParticleSimulationCS, "/Project/ParticleSimulation.usf", "MainCS", SF_Compute);
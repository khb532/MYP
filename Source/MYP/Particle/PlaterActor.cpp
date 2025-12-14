#include "PlaterActor.h"

#include "MYP.h"
#include "ParticleComputeComponent.h"
#include "ParticleOpCodes.h"

APlaterActor::APlaterActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ComputeComponent = CreateDefaultSubobject<UParticleComputeComponent>(TEXT("ComputeComponent"));
}

void APlaterActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 1. 버퍼 초기화
    ComputeComponent->InitializeBuffers(ParticleCount);

    // 2. 테스트 바이트코드 생성
    TArray<uint32> Bytecode;
    TArray<float> Constants;

    // 상수 풀 정의
    Constants.Add(10.0f);  // Constants[0] = 10.0 (X 간격)
    Constants.Add(0.0f);   // Constants[1] = 0.0 (Y, Z 위치)
    Constants.Add(1.0f);   // Constants[2] = 1.0 (R)
    Constants.Add(0.0f);   // Constants[3] = 0.0 (G, B, A)

    // position.x = particleID * 10.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_VAR));
    Bytecode.Add(static_cast<uint32>(EParticleVariable::PARTICLE_ID));
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
    Bytecode.Add(0);  // Constants[0] = 10.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::MUL));
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_X));

    // position.y = 0.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
    Bytecode.Add(1);  // Constants[1] = 0.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_Y));

    // position.z = 0.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
    Bytecode.Add(1);  // Constants[1] = 0.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_Z));

    // color.r = 1.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
    Bytecode.Add(2);  // Constants[2] = 1.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_R));

    // color.g = 0.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
    Bytecode.Add(3);  // Constants[3] = 0.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_G));

    // color.b = 0.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
    Bytecode.Add(3);  // Constants[3] = 0.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_B));

    // color.a = 1.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
    Bytecode.Add(2);  // Constants[2] = 1.0
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_A));

    // HALT
    Bytecode.Add(static_cast<uint32>(EParticleOpCode::HALT));

    // 3. 바이트코드 업로드
    ComputeComponent->UploadBytecode(Bytecode, Constants);

    LOGMSGF(TEXT("PlateActor initialized with test bytecode"));
	
	
	//	1 초후 디버그 출력
	FTimerHandle h_timer;
	GetWorld()->GetTimerManager().SetTimer(h_timer, [this]()
	{
		ComputeComponent->DebugPrintRenderTarget();
	}, 1.f, false);
}

void APlaterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ComputeComponent->ExecuteSimulation(DeltaTime);
}


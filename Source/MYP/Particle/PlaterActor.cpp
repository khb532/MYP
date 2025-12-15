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
	
	
	//	========== 런타임 바이트코드 업데이트 테스트 ==========

	//	1초 후: 초기 바이트코드 결과 디버그 출력 (particleID * 10)
	FTimerHandle h_timer1;
	GetWorld()->GetTimerManager().SetTimer(h_timer1, [this]()
	{
		LOGMSGF(TEXT("=== [1초] 초기 바이트코드 결과 (particleID * 10) ==="));
		ComputeComponent->DebugPrintRenderTarget();
	}, 1.f, false);

	//	5초 후: 새로운 바이트코드 업로드 (particleID * 20)
	FTimerHandle h_timer5;
	GetWorld()->GetTimerManager().SetTimer(h_timer5, [this]()
	{
		LOGMSGF(TEXT("=== [5초] 바이트코드 동적 변경: position.x = particleID (연산 없이) ==="));

		TArray<uint32> NewBytecode;
		TArray<float> NewConstants;

		// 새 상수 풀 (10.0 → 20.0으로 변경)
		NewConstants.Add(20.0f);  // Constants[0] = 20.0 (X 간격 2배)
		NewConstants.Add(0.0f);   // Constants[1] = 0.0 (Y, Z 위치)
		NewConstants.Add(0.0f);   // Constants[2] = 0.0 (R - 빨강에서 초록으로 변경)
		NewConstants.Add(1.0f);   // Constants[3] = 1.0 (G)

		// position.x = particleID (연산 없이, PUSH_VAR 테스트)
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_VAR));
		NewBytecode.Add(static_cast<uint32>(EParticleVariable::PARTICLE_ID));
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_X));

		// position.y = 0.0
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(1);
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_Y));

		// position.z = 0.0
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(1);
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_Z));

		// color.r = 0.0 (초록색으로 변경)
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(2);  // Constants[2] = 0.0
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_R));

		// color.g = 1.0
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(3);  // Constants[3] = 1.0
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_G));

		// color.b = 0.0
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(2);
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_B));

		// color.a = 1.0
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(3);
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_A));

		// HALT
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::HALT));

		// 새 바이트코드 업로드 (FRDGPooledBuffer 교체 테스트)
		ComputeComponent->UploadBytecode(NewBytecode, NewConstants);
		LOGMSGF(TEXT("새 바이트코드 업로드 완료 (20.0x, 초록색)"));

	}, 5.f, false);

	//	10초 후: Render Thread 동기화 및 변경된 바이트코드 결과 디버그 출력
	FTimerHandle h_timer10;
	GetWorld()->GetTimerManager().SetTimer(h_timer10, [this]()
	{
		LOGMSGF(TEXT("=== [10초] Render Thread 강제 동기화 후 결과 확인 ==="));

		// Render Thread의 모든 명령 완료 대기
		FlushRenderingCommands();

		ComputeComponent->DebugPrintRenderTarget();
		LOGMSGF(TEXT("=== 검증: Particle[0]=0, Particle[1]=1, Particle[2]=2로 변경되었는지 확인 (MUL 없이 PUSH_VAR만) ==="));
	}, 10.f, false);
}

void APlaterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ComputeComponent->ExecuteSimulation(DeltaTime);
}


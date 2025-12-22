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
	

	// ==================== 런타임 바이트코드 업데이트 테스트 ====================

	// 1초: 초기 바이트코드 결과 확인 (position.x = particleID * 10)
	FTimerHandle h_timer1;
	GetWorld()->GetTimerManager().SetTimer(h_timer1, [this]()
	{
		LOGMSGF(TEXT("=== [1초] 초기 바이트코드 결과 (particleID * 10) ==="));
		ComputeComponent->DebugPrintRenderTarget();
		LOGMSGF(TEXT("예상: Particle[0]=0, Particle[1]=10, Particle[2]=20 (빨강)"));
	}, 1.f, false);

	// 3초: Constants만 변경 테스트 (10.0 → 999.0, 빨강 → 초록)
	FTimerHandle h_timer3;
	GetWorld()->GetTimerManager().SetTimer(h_timer3, [this]()
	{
		LOGMSGF(TEXT("=== [3초] Constants 변경 테스트 (999.0, 초록색) ==="));

		TArray<uint32> SameBytecode;
		TArray<float> NewConstants;

		// 새 상수 풀
		NewConstants.Add(999.0f);  // Constants[0] = 999.0 (테스트용 큰 값)
		NewConstants.Add(0.0f);    // Constants[1] = 0.0
		NewConstants.Add(0.0f);    // Constants[2] = 0.0 (R = 0)
		NewConstants.Add(1.0f);    // Constants[3] = 1.0 (G = 1)

		// 동일한 바이트코드 (position.x = particleID * Constants[0])
		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_VAR));
		SameBytecode.Add(static_cast<uint32>(EParticleVariable::PARTICLE_ID));
		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		SameBytecode.Add(0);  // Constants[0] = 999.0
		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::MUL));
		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_X));

		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		SameBytecode.Add(1);
		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_Y));

		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		SameBytecode.Add(1);
		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_Z));

		// 초록색으로 변경
		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		SameBytecode.Add(2);  // R = 0
		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_R));

		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		SameBytecode.Add(3);  // G = 1
		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_G));

		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		SameBytecode.Add(1);  // B = 0
		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_B));

		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		SameBytecode.Add(3);  // A = 1
		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_A));

		SameBytecode.Add(static_cast<uint32>(EParticleOpCode::HALT));

		ComputeComponent->UploadBytecode(SameBytecode, NewConstants);
		LOGMSGF(TEXT("Constants 업데이트 완료 (999.0x, 초록색)"));
	}, 3.f, false);

	// 5초: Constants 변경 결과 확인
	FTimerHandle h_timer5;
	GetWorld()->GetTimerManager().SetTimer(h_timer5, [this]()
	{
		LOGMSGF(TEXT("=== [5초] Constants 변경 결과 확인 ==="));
		FlushRenderingCommands();
		ComputeComponent->DebugPrintRenderTarget();
		LOGMSGF(TEXT("예상: Particle[1]=999 이상 (초록색) - Constants 업데이트 성공"));
	}, 5.f, false);

	// 7초: Bytecode 변경 테스트 (MUL 제거, particleID만)
	FTimerHandle h_timer7;
	GetWorld()->GetTimerManager().SetTimer(h_timer7, [this]()
	{
		LOGMSGF(TEXT("=== [7초] Bytecode 변경 테스트 (MUL 제거) ==="));

		TArray<uint32> NewBytecode;
		TArray<float> NewConstants;

		// 새 상수 풀 (파란색)
		NewConstants.Add(20.0f);   // Constants[0] (사용 안 함)
		NewConstants.Add(0.0f);    // Constants[1] = 0.0
		NewConstants.Add(0.0f);    // Constants[2] = 0.0 (R)
		NewConstants.Add(0.0f);    // Constants[3] = 0.0 (G)
		NewConstants.Add(1.0f);    // Constants[4] = 1.0 (B)

		// position.x = particleID (MUL 없이)
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_VAR));
		NewBytecode.Add(static_cast<uint32>(EParticleVariable::PARTICLE_ID));
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_X));

		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(1);
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_Y));

		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(1);
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_POS_Z));

		// 파란색으로 변경
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(2);  // R = 0
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_R));

		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(3);  // G = 0
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_G));

		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(4);  // B = 1
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_B));

		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::PUSH_CONST));
		NewBytecode.Add(4);  // A = 1
		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::STORE_COLOR_A));

		NewBytecode.Add(static_cast<uint32>(EParticleOpCode::HALT));

		ComputeComponent->UploadBytecode(NewBytecode, NewConstants);
		LOGMSGF(TEXT("Bytecode 업데이트 완료 (MUL 제거, 파란색)"));
	}, 7.f, false);

	// 10초: Bytecode 변경 결과 확인
	FTimerHandle h_timer10;
	GetWorld()->GetTimerManager().SetTimer(h_timer10, [this]()
	{
		LOGMSGF(TEXT("=== [10초] Bytecode 변경 결과 최종 확인 ==="));
		FlushRenderingCommands();
		ComputeComponent->DebugPrintRenderTarget();
		LOGMSGF(TEXT("예상: Particle[0]=0, Particle[1]=1, Particle[2]=2 (파란색) - Bytecode 업데이트 성공"));
		LOGMSGF(TEXT("만약 여전히 999 이상이면 Bytecode 업데이트 실패 (RDG Pooling 문제)"));
	}, 10.f, false);
}

void APlaterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ComputeComponent->ExecuteSimulation(DeltaTime);
}


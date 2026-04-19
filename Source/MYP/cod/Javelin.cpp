
#include "Javelin.h"

#include "COD_ProjMoveComponent_Javelin.h"
#include "MYP.h"


AJavelin::AJavelin()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Mass = 22000.f;			// 총 발사체 질량 (kg)

	CD = 0.4f;				// 항력계수

	MuzzleVelocity = 1000.f;	// 초기속도 0 — 로켓 추진으로 가속

	CrossSectionArea = 126.7f;	// 단면적 (cm²) — 직경 127mm 기준
	
	ProjectileMovement = CreateDefaultSubobject<UCOD_ProjMoveComponent_Javelin>(TEXT("ProjectileMovement"));
}

void AJavelin::BeginPlay()
{
	Super::BeginPlay();

	if (ProjectileMovement)
	{
		ProjectileMovement->InitBulletData(Mass, CD, CrossSectionArea, MuzzleVelocity);
	}
}	

void AJavelin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


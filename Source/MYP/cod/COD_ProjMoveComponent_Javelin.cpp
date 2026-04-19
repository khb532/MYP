
#include "COD_ProjMoveComponent_Javelin.h"


UCOD_ProjMoveComponent_Javelin::UCOD_ProjMoveComponent_Javelin()
{
	PrimaryComponentTick.bCanEverTick = true;
	bSyncRot = false;
}

void UCOD_ProjMoveComponent_Javelin::BeginPlay()
{
	Super::BeginPlay();
}

void UCOD_ProjMoveComponent_Javelin::InitBulletData(float _Mass, float _Cd, float _Area, float _MuzzleVelocity, float _WeaponMultiplier)
{
	Super::InitBulletData(_Mass, _Cd, _Area, _MuzzleVelocity, _WeaponMultiplier);
	LaunchDir = GetOwner()->GetActorForwardVector();
	BurnElapsed = 0.f;
}

void UCOD_ProjMoveComponent_Javelin::TickComponent(float DeltaTime, ELevelTick TickType,
                                                   FActorComponentTickFunction* ThisTickFunction)
{
	MainEngineBoost(DeltaTime);
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FVector UCOD_ProjMoveComponent_Javelin::ComputeAcceleration(float DeltaTime)
{
	FVector GravityAccel = GravityScale * FVector(0.f, 0.f, -980.f);
	
	// F_drag = -0.5 * ρ * Cd * A * V²
	FVector DragAccel = FVector::ZeroVector;
	float SpeedSquad = Velocity.SizeSquared();
	if (SpeedSquad > 0.f)
	{
		float DragForceMag = 0.5f * AirDensity * Cd * CrossSectionArea * SpeedSquad;
		float DragAccelMag = DragForceMag / Mass * DragScale;
		DragAccel = -Velocity.GetSafeNormal() * DragAccelMag;
	}
	
	return GravityAccel + DragAccel;
}

void UCOD_ProjMoveComponent_Javelin::MainEngineBoost(float DeltaTime)
{
	if (BurnElapsed >= BurnTime) return;

	BurnElapsed += DeltaTime;
	Velocity += LaunchDir * ThrustAccel * DeltaTime;
}

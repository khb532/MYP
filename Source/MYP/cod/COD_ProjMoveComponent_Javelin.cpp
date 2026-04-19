
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
	CurrentPitch = GetOwner()->GetActorRotation().Pitch;
}

void UCOD_ProjMoveComponent_Javelin::ApplyPitchTorque(float TargetPitch, float DeltaTime)
{
	if (CurrentPitch > TargetPitch) return;
	
	AngularVelocity += AngularAccel * DeltaTime;
	CurrentPitch += AngularVelocity * DeltaTime;
	
	UpdatedComponent->SetWorldRotation(FRotator(CurrentPitch, UpdatedComponent->GetComponentRotation().Yaw, UpdatedComponent->GetComponentRotation().Roll));
}

void UCOD_ProjMoveComponent_Javelin::TickComponent(float DeltaTime, ELevelTick TickType,
                                                   FActorComponentTickFunction* ThisTickFunction)
{
	MainEngineBoost(DeltaTime);
	
	if (BurnElapsed >= 0.f)
		ApplyPitchTorque(80.f, DeltaTime);
	
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
	
	if (BurnElapsed >= 0.f && UpdatedComponent)
	{
		FVector CurrentForward = UpdatedComponent->GetForwardVector();
		Velocity += CurrentForward * ThrustAccel * DeltaTime;
	}
}

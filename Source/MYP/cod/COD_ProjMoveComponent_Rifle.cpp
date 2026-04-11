
#include "COD_ProjMoveComponent_Rifle.h"


UCOD_ProjMoveComponent_Rifle::UCOD_ProjMoveComponent_Rifle()
{
	PrimaryComponentTick.bCanEverTick = true;
}

FVector UCOD_ProjMoveComponent_Rifle::ComputeAcceleration(float DeltaTime)
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





#include "SmallCaliber.h"

#include "COD_ProjMoveComponent_Rifle.h"


ASmallCaliber::ASmallCaliber()
{
	Mass = 4.f;
	MuzzleVelocity = 92000.f;
	CD = 0.3f;
	CrossSectionArea = 0.243f;
	
	ProjectileMovement = CreateDefaultSubobject<UCOD_ProjMoveComponent_Rifle>(TEXT("ProjectileMovement"));
}


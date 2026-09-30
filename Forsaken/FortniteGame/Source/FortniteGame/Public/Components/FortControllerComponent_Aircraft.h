#pragma once

#include "CoreGlobals.h"

class FortControllerComponent_Aircraft
{
private:
	static void ServerAttemptAircraftJump(UFortControllerComponent_Aircraft* ControllerComponent, const FRotator& ClientRotation);
public:
	static void Setup();
};

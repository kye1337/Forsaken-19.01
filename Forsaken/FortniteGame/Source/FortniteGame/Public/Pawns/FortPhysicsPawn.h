#pragma once

#include "CoreGlobals.h"

class FortPhysicsPawn
{
private:
	static void ServerMove(AFortPhysicsPawn* PhysicsPawn, FReplicatedPhysicsPawnState& InState);
public:
	static void Setup();
};

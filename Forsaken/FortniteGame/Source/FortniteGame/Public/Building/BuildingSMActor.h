#pragma once

#include "CoreGlobals.h"
#include "Items/FortInventory.h"

class BuildingSMActor
{
private:
	static void AttemptSpawnResources(ABuildingSMActor* BuildingActor, AFortPlayerPawn* InstigatorPawn, float ActualDamageDealt, bool bJustHitWeakspot);
public:
	static void Setup();
};

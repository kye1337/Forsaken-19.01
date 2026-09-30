#pragma once

#include "CoreGlobals.h"

class FortAthenaAISpawnerDataComponent_InventoryBase
{
private:
	static void GetInventoryItems(UFortAthenaAISpawnerDataComponent_InventoryBase* InventoryBase, TArray<FItemAndCount>* OutList);
public:
	static void Setup();
};

#pragma once

#include "CoreGlobals.h"
#include "Items/FortInventory.h"
#include "Items/FortLootPackage.h"

class FortKismetLibrary
{
private:
	class Original
	{
	public:
		static inline bool (*PickLootDrops)(UObject* Context, FFrame& Stack, bool* Ret);
	public:
		static inline AFortPickup* (*K2_SpawnPickupInWorld)(UObject* Context, FFrame& Stack, AFortPickup** Ret);
		static inline AFortPickup* (*SpawnItemVariantPickupInWorld)(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	public:
		static inline UFortWorldItem* (*GiveItemToInventoryOwner)(UObject* Context, FFrame& Stack, UFortWorldItem** Ret);
	};
private:
	static bool PickLootDrops(UObject* Context, FFrame& Stack, bool* Ret);
private:
	static AFortPickup* K2_SpawnPickupInWorld(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static AFortPickup* SpawnItemVariantPickupInWorld(UObject* Context, FFrame& Stack, AFortPickup** Ret);
private:
	static UFortWorldItem* GiveItemToInventoryOwner(UObject* Context, FFrame& Stack, UFortWorldItem** Ret);
private:
	static AFortAIDirector* GetAIDirector(UObject* Context, FFrame& Stack, AFortAIDirector** Ret);
	static AFortAIGoalManager* GetAIGoalManager(UObject* Context, FFrame& Stack, AFortAIGoalManager** Ret);
public:
	static void Setup();
};

#pragma once

#include "CoreGlobals.h"
#include "FortInventory.h"

class FortInventoryOwnerInterface
{
private:
	static bool CreateInventory(IFortInventoryOwnerInterface* InventoryOwner);
	static void InitializeInventory(IFortInventoryOwnerInterface* InventoryOwner, const TArray<FItemAndCount>* ItemList);
private:
	static bool RemoveInventoryItem(IFortInventoryOwnerInterface* InventoryOwner, const FGuid& ItemGuid, int32 Count, bool bForceRemoval, bool bForcePersistWhenEmpty);
public:
	static void Setup();
};

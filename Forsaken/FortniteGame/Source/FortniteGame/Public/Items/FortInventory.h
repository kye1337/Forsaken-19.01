#pragma once

#include "CoreGlobals.h"

class FortInventory
{
public:
	static FFortItemEntry* GetItemEntry(AFortInventory* WorldInventory, const FGuid& ItemGuid);
	static FFortItemEntry* GetItemEntry(AFortInventory* WorldInventory, const UFortItemDefinition* ItemDefinition, bool bCheckStackSize = false);
	static FFortItemEntry* GetItemEntry(AFortInventory* WorldInventory, const UClass* Class);

	static UFortWorldItem* GetItemInstance(AFortInventory* WorldInventory, const FGuid& ItemGuid);
	static UFortWorldItem* GetItemInstance(AFortInventory* WorldInventory, const UFortItemDefinition* ItemDefinition);
	static UFortWorldItem* GetItemInstance(AFortInventory* WorldInventory, const UClass* Class);
public:
	static bool GetDropableItems(AFortInventory* WorldInventory, TArray<UFortWorldItem*>* OutDroppableItems);
	static int32 GetNumberOfSlotsTaken(AFortInventory* WorldInventory);
public:
	static UFortWorldItem* CreateItem(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count = 1, int32 LoadedAmmo = 0, int32 PhantomReserveAmmo = 0);
	static void RemoveItem(AFortInventory* WorldInventory, const FGuid& ItemGuid, int32 Count, bool bForceRemoval = false);
	static void StackItem(AFortInventory* WorldInventory, FFortItemEntry* ItemEntry, int32 Count);
public:
	static void TryToStackItem(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo, int32 PhantomReserveAmmo);
	static void TryToAddItem(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo, int32 PhantomReserveAmmo);
	static void AddItem(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count = 1, int32 LoadedAmmo = 0, int32 PhantomReserveAmmo = 0);
};

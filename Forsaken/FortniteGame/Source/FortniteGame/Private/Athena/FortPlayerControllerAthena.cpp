#include "Athena/FortPlayerControllerAthena.h"
#include "Items/FortInventory.h"

void FortPlayerControllerAthena::RemoveAllInventoryItems(AFortPlayerControllerAthena* PlayerController)
{
	AFortInventory* WorldInventory = PlayerController->WorldInventory;

	if (WorldInventory == NULL)
		return;

	TArray<UFortWorldItem*> DroppableItems;

	if (FortInventory::GetDropableItems(WorldInventory, &DroppableItems))
	{
		for (UFortWorldItem* DroppableItem : DroppableItems)
		{
			FFortItemEntry& ItemEntry = DroppableItem->ItemEntry;

			FortInventory::RemoveItem(WorldInventory, ItemEntry.ItemGuid, 0, true);
		}
	}
}

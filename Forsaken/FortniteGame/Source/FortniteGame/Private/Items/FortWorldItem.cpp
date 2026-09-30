#include "Items/FortWorldItem.h"

void FortWorldItem::SetLoadedAmmo(UFortWorldItem* WorldItem, int32 InCount)
{
	AFortInventory* WorldInventory = WorldItem->OwnerInventory;

	if (WorldInventory == NULL)
		return;

	if (InCount != WorldItem->ItemEntry.LoadedAmmo)
	{
		WorldItem->ItemEntry.LoadedAmmo = InCount;

		WorldInventory->Inventory.MarkItemDirty(WorldItem->ItemEntry);

		FFortItemEntry* ItemEntry = FortInventory::GetItemEntry(WorldInventory, WorldItem->ItemEntry.ItemGuid);

		if (ItemEntry == NULL)
			return;

		ItemEntry->LoadedAmmo = InCount;

		WorldInventory->Inventory.MarkItemDirty(ItemEntry);
		WorldInventory->HandleInventoryLocalUpdate();
	}
}

void FortWorldItem::SetPhantomReserveAmmo(UFortWorldItem* WorldItem, int32 InCount)
{
	AFortInventory* WorldInventory = WorldItem->OwnerInventory;

	if (WorldInventory == NULL)
		return;

	if (InCount != WorldItem->ItemEntry.PhantomReserveAmmo)
	{
		WorldItem->ItemEntry.PhantomReserveAmmo = InCount;

		WorldInventory->Inventory.MarkItemDirty(WorldItem->ItemEntry);

		FFortItemEntry* ItemEntry = FortInventory::GetItemEntry(WorldInventory, WorldItem->ItemEntry.ItemGuid);

		if (ItemEntry == NULL)
			return;

		ItemEntry->PhantomReserveAmmo = InCount;

		WorldInventory->Inventory.MarkItemDirty(ItemEntry);
		WorldInventory->HandleInventoryLocalUpdate();
	}
}

void FortWorldItem::Setup()
{
	UKismetDetourLibrary::InitializeDetour<UFortWorldItem, VFT>(0xb8, SetLoadedAmmo);
	UKismetDetourLibrary::InitializeDetour<UFortWorldItem, VFT>(0xb9, SetPhantomReserveAmmo);
}

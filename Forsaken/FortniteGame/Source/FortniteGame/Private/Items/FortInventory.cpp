#include "Items/FortInventory.h"

FFortItemEntry* FortInventory::GetItemEntry(AFortInventory* WorldInventory, const FGuid& ItemGuid)
{
	for (FFortItemEntry& ReplicatedEntry : WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ReplicatedEntry.ItemGuid == ItemGuid)
			return &ReplicatedEntry;
	}

	return NULL;
}

FFortItemEntry* FortInventory::GetItemEntry(AFortInventory* WorldInventory, const UFortItemDefinition* ItemDefinition, bool bCheckStackSize)
{
	for (FFortItemEntry& ReplicatedEntry : WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ReplicatedEntry.ItemDefinition == ItemDefinition)
		{
			if (bCheckStackSize && ReplicatedEntry.Count >= ItemDefinition->MaxStackSize.GetValueAtLevel(0))
				continue;

			return &ReplicatedEntry;
		}
	}

	return NULL;
}

FFortItemEntry* FortInventory::GetItemEntry(AFortInventory* WorldInventory, const UClass* Class)
{
	for (FFortItemEntry& ReplicatedEntry : WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ReplicatedEntry.ItemDefinition->Class == Class)
			return &ReplicatedEntry;
	}

	return NULL;
}

UFortWorldItem* FortInventory::GetItemInstance(AFortInventory* WorldInventory, const FGuid& ItemGuid)
{
	for (UFortWorldItem* ItemInstance : WorldInventory->Inventory.ItemInstances)
	{
		if (ItemInstance->ItemEntry.ItemGuid == ItemGuid)
			return ItemInstance;
	}

	return NULL;
}

UFortWorldItem* FortInventory::GetItemInstance(AFortInventory* WorldInventory, const UFortItemDefinition* ItemDefinition)
{
	for (UFortWorldItem* ItemInstance : WorldInventory->Inventory.ItemInstances)
	{
		if (ItemInstance->ItemEntry.ItemDefinition == ItemDefinition)
			return ItemInstance;
	}

	return NULL;
}

UFortWorldItem* FortInventory::GetItemInstance(AFortInventory* WorldInventory, const UClass* Class)
{
	for (UFortWorldItem* ItemInstance : WorldInventory->Inventory.ItemInstances)
	{
		if (ItemInstance->ItemEntry.ItemDefinition->Class == Class)
			return ItemInstance;
	}

	return NULL;
}

bool FortInventory::GetDropableItems(AFortInventory* WorldInventory, TArray<UFortWorldItem*>* OutDroppableItems)
{
	for (UFortWorldItem* ItemInstance : WorldInventory->Inventory.ItemInstances)
	{
		if (ItemInstance->CanBeDropped())
			OutDroppableItems->Add(ItemInstance);
	}

	return (OutDroppableItems->Num() > 0);
}

int32 FortInventory::GetNumberOfSlotsTaken(AFortInventory* WorldInventory)
{
	int32 NumberOfSlotsTaken = 0;

	for (FFortItemEntry& ReplicatedEntry : WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ReplicatedEntry.ItemDefinition->GetQuickbar() == Primary)
		{
			UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(ReplicatedEntry.ItemDefinition);

			if (WorldItemDefinition == NULL)
				continue;

			NumberOfSlotsTaken += WorldItemDefinition->NumberOfSlotsToTake;
		}
	}

	return NumberOfSlotsTaken;
}

UFortWorldItem* FortInventory::CreateItem(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo, int32 PhantomReserveAmmo)
{
	if (Count <= 0)
		return NULL;

	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(WorldInventory->GetOwner());

	if (PlayerController == NULL)
		return NULL;

	UFortWorldItem* WorldItem = Cast<UFortWorldItem>(ItemDefinition->CreateTemporaryItemInstanceBP(Count, 0));

	if (WorldItem == NULL)
		return NULL;

	WorldItem->OwnerInventory = WorldInventory;
	WorldItem->SetOwningControllerForTemporaryItem(PlayerController);

	WorldItem->ItemEntry.ItemDefinition = ItemDefinition;
	WorldItem->ItemEntry.Count = Count;
	WorldItem->ItemEntry.LoadedAmmo = LoadedAmmo;
	WorldItem->ItemEntry.PhantomReserveAmmo = PhantomReserveAmmo;

	WorldInventory->Inventory.ItemInstances.Add(WorldItem);
	WorldInventory->Inventory.ReplicatedEntries.Add(WorldItem->ItemEntry);
	WorldInventory->Inventory.MarkItemDirty(WorldItem->ItemEntry);
	WorldInventory->HandleInventoryLocalUpdate();

	return WorldItem;
}

void FortInventory::RemoveItem(AFortInventory* WorldInventory, const FGuid& ItemGuid, int32 Count, bool bForceRemoval)
{
	for (int32 i = 0; i < WorldInventory->Inventory.ReplicatedEntries.Num(); ++i)
	{
		FFortItemEntry& ReplicatedEntry = WorldInventory->Inventory.ReplicatedEntries[i];

		if (ReplicatedEntry.ItemGuid == ItemGuid)
		{
			int32 NewCount = ReplicatedEntry.Count -= Count;

			if (bForceRemoval || NewCount <= 0)
			{
				WorldInventory->Inventory.ReplicatedEntries.Remove(i);
			}
			else
			{
				WorldInventory->Inventory.MarkItemDirty(ReplicatedEntry);
			}

			break;
		}
	}

	for (int32 i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); ++i)
	{
		UFortWorldItem* ItemInstance = WorldInventory->Inventory.ItemInstances[i];

		if (ItemInstance->ItemEntry.ItemGuid == ItemGuid)
		{
			int32 NewCount = ItemInstance->ItemEntry.Count -= Count;

			if (bForceRemoval || NewCount <= 0)
			{
				WorldInventory->Inventory.ItemInstances.Remove(i);
			}
			else
			{
				WorldInventory->Inventory.MarkItemDirty(ItemInstance->ItemEntry);
			}

			break;
		}
	}

	WorldInventory->Inventory.MarkArrayDirty();
	WorldInventory->HandleInventoryLocalUpdate();
}

void FortInventory::StackItem(AFortInventory* WorldInventory, FFortItemEntry* ItemEntry, int32 Count)
{
	ItemEntry->Count += Count;

	WorldInventory->Inventory.MarkItemDirty(ItemEntry);
	WorldInventory->HandleInventoryLocalUpdate();
}

void FortInventory::TryToStackItem(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo, int32 PhantomReserveAmmo)
{
	auto CreateAndTossPickup = [](AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo, int32 PhantomReserveAmmo) -> AFortPickup*
		{
			FFortItemEntry ItemEntry;
			ItemEntry.FortItemEntry(ItemDefinition, Count, 0, FGuid());

			ItemEntry.LoadedAmmo = LoadedAmmo;
			ItemEntry.PhantomReserveAmmo = PhantomReserveAmmo;

			FortPickupCreationData CreationData = FortPickupCreationData(UWorld::GetWorld(), &ItemEntry, PlayerController->MyFortPawn->K2_GetActorLocation(), FRotator(), NULL, NULL, NULL, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::TossedByPlayer);

			AFortPickup* Pickup = AFortPickup::CreateFromData(&CreationData);

			auto TossDirection = PlayerController->MyFortPawn->K2_GetActorLocation() + (PlayerController->MyFortPawn->GetActorForwardVector() * UKismetMathLibrary::RandomFloatInRange(500.f, 600.f));

			Pickup->TossPickup(TossDirection, PlayerController->MyFortPawn, 0, true, true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::TossedByPlayer);

			return Pickup;
		};

	FFortItemEntry* ItemEntry = GetItemEntry(WorldInventory, ItemDefinition, true);

	if (ItemEntry != NULL)
	{
		AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(WorldInventory->GetOwner());

		if (PlayerController == NULL)
			return;

		int32 Remaining = Count;
		int32 MaxStackSize = ItemDefinition->MaxStackSize.GetValueAtLevel(0);

		for (FFortItemEntry& ReplicatedEntry : WorldInventory->Inventory.ReplicatedEntries)
		{
			if (ReplicatedEntry.ItemDefinition == ItemDefinition)
			{
				if (ReplicatedEntry.Count >= MaxStackSize)
					continue;

				if (ReplicatedEntry.Count + Remaining > MaxStackSize)
				{
					StackItem(WorldInventory, ItemEntry, MaxStackSize - ReplicatedEntry.Count);
					Remaining -= MaxStackSize - ReplicatedEntry.Count;
				}
				else
				{
					StackItem(WorldInventory, &ReplicatedEntry, Remaining);
					return;
				}
			}
		}

		if (Remaining <= 0)
			return;

		if (!ItemDefinition->bAllowMultipleStacks)
		{
			CreateAndTossPickup(PlayerController, ItemDefinition, Remaining, LoadedAmmo, PhantomReserveAmmo);
			return;
		}

		if (ItemDefinition->GetQuickbar() == Secondary)
		{
			CreateItem(WorldInventory, ItemDefinition, Remaining, LoadedAmmo, PhantomReserveAmmo);
			return;
		}

		if (PlayerController->MyFortPawn->CurrentWeapon->WeaponData->GetQuickbar() == Harvesting)
		{
			CreateAndTossPickup(PlayerController, ItemDefinition, Remaining, LoadedAmmo, PhantomReserveAmmo);
		}
		else if (PlayerController->MyFortPawn->CurrentWeapon->WeaponData->GetQuickbar() == Primary)
		{
			FFortItemEntry* CurrentEntry = GetItemEntry(WorldInventory, PlayerController->MyFortPawn->CurrentWeapon->ItemEntryGuid);

			PlayerController->ServerAttemptInventoryDrop(CurrentEntry->ItemGuid, CurrentEntry->Count, false);
			CreateItem(WorldInventory, ItemDefinition, Remaining, LoadedAmmo, PhantomReserveAmmo);
		}
	}
	else
	{
		TryToAddItem(WorldInventory, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
	}
}

void FortInventory::TryToAddItem(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo, int32 PhantomReserveAmmo)
{
	auto CreateAndTossPickup = [](AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo, int32 PhantomReserveAmmo) -> AFortPickup*
		{
			FFortItemEntry ItemEntry;
			ItemEntry.FortItemEntry(ItemDefinition, Count, 0, FGuid());

			ItemEntry.LoadedAmmo = LoadedAmmo;
			ItemEntry.PhantomReserveAmmo = PhantomReserveAmmo;

			FortPickupCreationData CreationData = FortPickupCreationData(UWorld::GetWorld(), &ItemEntry, PlayerController->MyFortPawn->K2_GetActorLocation(), FRotator(), NULL, NULL, NULL, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::TossedByPlayer);

			AFortPickup* Pickup = AFortPickup::CreateFromData(&CreationData);

			auto TossDirection = PlayerController->MyFortPawn->K2_GetActorLocation() + (PlayerController->MyFortPawn->GetActorForwardVector() * UKismetMathLibrary::RandomFloatInRange(500.f, 600.f));

			Pickup->TossPickup(TossDirection, PlayerController->MyFortPawn, 0, true, true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::TossedByPlayer);

			return Pickup;
		};

	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(WorldInventory->GetOwner());

	if (PlayerController == NULL)
		return;

	AFortPlayerPawn* Pawn = PlayerController->MyFortPawn;

	if (Pawn == NULL)
		return;

	FFortItemEntry* ItemEntry = GetItemEntry(WorldInventory, ItemDefinition);

	if (ItemEntry != NULL && ItemEntry->Count >= ItemDefinition->MaxStackSize.GetValueAtLevel(0) && !ItemDefinition->bAllowMultipleStacks)
	{
		CreateAndTossPickup(PlayerController, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
		return;
	}

	if (GetNumberOfSlotsTaken(WorldInventory) >= 5 && ItemDefinition->GetQuickbar() == Primary)
	{
		if (AFortWeapon* CurrentWeapon = Pawn->CurrentWeapon)
		{
			FFortItemEntry* CurrentEntry = GetItemEntry(WorldInventory, Pawn->CurrentWeapon->ItemEntryGuid);

			if (CurrentEntry == NULL)
			{
				CreateAndTossPickup(PlayerController, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
				return;
			}

			if (CurrentEntry->ItemDefinition->GetQuickbar() == Primary)
			{
				PlayerController->ServerAttemptInventoryDrop(CurrentEntry->ItemGuid, CurrentEntry->Count, false);
				FortInventory::CreateItem(WorldInventory, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
			}
			else if (CurrentEntry->ItemDefinition->GetQuickbar() == Harvesting)
			{
				if (AFortWeapon* PreviousWeapon = Pawn->PreviousWeapon)
				{
					FFortItemEntry* PreviousEntry = GetItemEntry(WorldInventory, PreviousWeapon->ItemEntryGuid);

					if (PreviousEntry != NULL)
					{
						if (PreviousEntry->ItemDefinition->GetQuickbar() != Primary)
						{
							CreateAndTossPickup(PlayerController, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
							return;
						}

						PlayerController->ServerAttemptInventoryDrop(PreviousEntry->ItemGuid, PreviousEntry->Count, false);
						FortInventory::CreateItem(WorldInventory, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
					}
					else
					{
						CreateAndTossPickup(PlayerController, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
					}
				}
				else
				{
					CreateAndTossPickup(PlayerController, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
				}
			}
			else
			{
				CreateAndTossPickup(PlayerController, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
			}
		}
		else
		{
			CreateAndTossPickup(PlayerController, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
		}
	}
	else
	{
		CreateItem(WorldInventory, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
	}
}

void FortInventory::AddItem(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo, int32 PhantomReserveAmmo)
{
	if (Count <= 0)
		return;

	if (ItemDefinition->IsStackable())
	{
		TryToStackItem(WorldInventory, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
	}
	else
	{
		TryToAddItem(WorldInventory, ItemDefinition, Count, LoadedAmmo, PhantomReserveAmmo);
	}
}

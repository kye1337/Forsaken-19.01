#include "Items/FortLootPackage.h"

void UFortLootPackage::GetLootTierDataTables(TArray<UDataTable*>* OutLootTierDataTables)
{
	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

	if (GameState == NULL)
		return;

	UFortPlaylistAthena* Playlist = GameState->CurrentPlaylistInfo.BasePlaylist;

	if (Playlist)
	{
		if (UDataTable* LootTierData = Cast<UDataTable>(Playlist->LootTierData.LoadSynchronous()))
		{
			//LootTierData->AddToRoot();
			OutLootTierDataTables->Add(LootTierData);
		}
	}
	else
	{
		UDataTable* AthenaLootTierData_Client = StaticLoadObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client");

		//AthenaLootTierData_Client->AddToRoot();
		OutLootTierDataTables->Add(AthenaLootTierData_Client);
	}

	for (int32 i = 0; i < UObject::GObjects->Num(); ++i)
	{
		UFortGameFeatureData* GameFeatureData = Cast<UFortGameFeatureData>(UObject::GObjects->GetByIndex(i));

		if (GameFeatureData == NULL)
			continue;

		if (UDataTable* LootTierData = Cast<UDataTable>(GameFeatureData->DefaultLootTableData.LootTierData.LoadSynchronous()))
		{
			//LootTierData->AddToRoot();
			OutLootTierDataTables->Add(LootTierData);
		}

		if (Playlist == NULL)
			continue;

		for (const FGameplayTag& Tag : Playlist->GameplayTagContainer.GameplayTags)
		{
			for (auto& [GameplayTag, LootTableData] : GameFeatureData->PlaylistOverrideLootTableData)
			{
				if (Tag.TagName != GameplayTag.TagName)
					continue;

				if (UDataTable* FeaturedLootTierData = Cast<UDataTable>(LootTableData.LootTierData.LoadSynchronous()))
				{
					//FeaturedLootTierData->AddToRoot();
					OutLootTierDataTables->Add(FeaturedLootTierData);
				}
			}
		}
	}
}

void UFortLootPackage::GetLootPackageDataTables(TArray<UDataTable*>* OutLootPackageDataTables)
{
	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

	if (GameState == NULL)
		return;

	UFortPlaylistAthena* Playlist = GameState->CurrentPlaylistInfo.BasePlaylist;

	if (Playlist)
	{
		if (UDataTable* LootPackages = Cast<UDataTable>(Playlist->LootPackages.LoadSynchronous()))
		{
			std::cout << LootPackages->RowMap.Num() << std::endl;
			//LootPackages->AddToRoot();
			OutLootPackageDataTables->Add(LootPackages);
		}
	}
	else
	{
		UDataTable* AthenaLootPackages_Client = StaticLoadObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client");
		//AthenaLootPackages_Client->AddToRoot();
		OutLootPackageDataTables->Add(AthenaLootPackages_Client);
	}

	for (int32 i = 0; i < UObject::GObjects->Num(); ++i)
	{
		UFortGameFeatureData* GameFeatureData = Cast<UFortGameFeatureData>(UObject::GObjects->GetByIndex(i));

		if (GameFeatureData == NULL)
			continue;

		if (UDataTable* LootPackageData = Cast<UDataTable>(GameFeatureData->DefaultLootTableData.LootPackageData.LoadSynchronous()))
		{
			std::cout << LootPackageData->RowMap.Num() << std::endl;
			//LootPackageData->AddToRoot();
			OutLootPackageDataTables->Add(LootPackageData);
		}

		if (Playlist == NULL)
			continue;

		for (const FGameplayTag& Tag : Playlist->GameplayTagContainer.GameplayTags)
		{
			for (auto& [GameplayTag, LootTableData] : GameFeatureData->PlaylistOverrideLootTableData)
			{
				if (Tag.TagName != GameplayTag.TagName)
					continue;

				if (UDataTable* FeaturedLootPackageData = Cast<UDataTable>(LootTableData.LootPackageData.LoadSynchronous()))
				{
					std::cout << FeaturedLootPackageData->RowMap.Num() << std::endl;

					//FeaturedLootPackageData->AddToRoot();
					OutLootPackageDataTables->Add(FeaturedLootPackageData);
				}
			}
		}
	}
}

FFortLootTierData* UFortLootPackage::PickLootTierData(TArray<UDataTable*>& DataTables, const FName TierGroupName)
{
	TArray<FFortLootTierData*> LootTierDataTables;

	for (UDataTable* DataTable : DataTables)
	{
		if (DataTable == NULL)
			continue;

		if (DataTable->RowMap.Num() <= 0)
		{
			continue;
		}

		for (const auto& [RowName, RowValue] : DataTable->RowMap)
		{
			if (!RowName.ComparisonIndex || RowValue == NULL)
				continue;

			FFortLootTierData* LootTierRow = (FFortLootTierData*)RowValue;

			if (LootTierRow == NULL)
				continue;

			if (LootTierRow->TierGroup != TierGroupName)
				continue;

			LootTierDataTables.Add(LootTierRow);
		}

		if (UCompositeDataTable* CompositeDataTable = Cast<UCompositeDataTable>(DataTable))
		{
			for (UDataTable* ParentTable : CompositeDataTable->ParentTables)
			{
				if (ParentTable == NULL)
					continue;

				for (auto& [RowName, RowValue] : ParentTable->RowMap)
				{
					if (!RowName.ComparisonIndex || RowValue == NULL)
						continue;

					FFortLootTierData* LootTierRow = (FFortLootTierData*)RowValue;

					if (LootTierRow == NULL)
						continue;

					if (LootTierRow->TierGroup != TierGroupName)
						continue;

					LootTierDataTables.Add(LootTierRow);
				}
			}
		}
	}

	float TotalWeight = DataTable::GetTotalWeight<FFortLootTierData>(LootTierDataTables);

	float RandomWeight = (rand() * 0.00003051851f) * TotalWeight;

	FFortLootTierData* LootTierData = DataTable::PickWeightedElement<FFortLootTierData>(LootTierDataTables, RandomWeight);

	return LootTierData;
}

FFortLootPackageData* UFortLootPackage::PickLootPackageData(TArray<UDataTable*>& DataTables, FName LootPackage, int32 LootPackageCategory, int32 WorldLevel)
{
	TArray<FFortLootPackageData*> LootPackageDataTables;

	for (UDataTable* DataTable : DataTables)
	{
		if (DataTable == NULL)
			continue;

		if (DataTable->RowMap.Num() <= 0)
		{
			continue;
		}

		for (const auto& [RowName, RowValue] : DataTable->RowMap)
		{
			if (!RowName.ComparisonIndex || RowValue == NULL)
				continue;

			FFortLootPackageData* LootPackageRow = (FFortLootPackageData*)RowValue;

			if (LootPackageRow == NULL)
				continue;

			if (LootPackageRow->LootPackageID != LootPackage)
				continue;

			if (LootPackageCategory != -1 && LootPackageRow->LootPackageCategory != LootPackageCategory)
				continue;

			if (WorldLevel >= 0)
			{
				if (LootPackageRow->MaxWorldLevel >= 0 && WorldLevel > LootPackageRow->MaxWorldLevel)
					continue;

				if (LootPackageRow->MinWorldLevel >= 0 && WorldLevel < LootPackageRow->MinWorldLevel)
					continue;
			}

			LootPackageDataTables.Add(LootPackageRow);
		}

		if (UCompositeDataTable* CompositeDataTable = Cast<UCompositeDataTable>(DataTable))
		{
			for (UDataTable* ParentTable : CompositeDataTable->ParentTables)
			{
				if (ParentTable == NULL)
					continue;

				for (auto& [RowName, RowValue] : ParentTable->RowMap)
				{
					if (!RowName.ComparisonIndex || RowValue == NULL)
						continue;

					FFortLootPackageData* LootPackageRow = (FFortLootPackageData*)RowValue;

					if (LootPackageRow == NULL)
						continue;

					if (LootPackageRow->LootPackageID != LootPackage)
						continue;

					if (LootPackageCategory != -1 && LootPackageRow->LootPackageCategory != LootPackageCategory)
						continue;

					if (WorldLevel >= 0)
					{
						if (LootPackageRow->MaxWorldLevel >= 0 && WorldLevel > LootPackageRow->MaxWorldLevel)
							continue;

						if (LootPackageRow->MinWorldLevel >= 0 && WorldLevel < LootPackageRow->MinWorldLevel)
							continue;
					}

					LootPackageDataTables.Add(LootPackageRow);
				}
			}
		}
	}

	float TotalWeight = DataTable::GetTotalWeight<FFortLootPackageData>(LootPackageDataTables);

	float RandomWeight = (rand() * 0.00003051851f) * TotalWeight;

	FFortLootPackageData* LootPackageData = DataTable::PickWeightedElement<FFortLootPackageData>(LootPackageDataTables, RandomWeight);

	return LootPackageData;
}

void UFortLootPackage::PickLootDropsFromLootPackage(TArray<FFortItemEntry>* OutItemEntries, TArray<UDataTable*>& LootPackageDataTables, FName LootPackage, int32 LootPackageCategory, int32 WorldLevel)
{
	FFortLootPackageData* LootPackageData = PickLootPackageData(LootPackageDataTables, LootPackage, LootPackageCategory, WorldLevel);

	if (LootPackageData == NULL)
		return;

	if (LootPackageData->LootPackageCall.Num() > 1)
	{
		for (int32 i = 0; i < LootPackageData->Count; ++i)
		{
			PickLootDropsFromLootPackage(OutItemEntries, LootPackageDataTables, UKismetStringLibrary::Conv_StringToName(LootPackageData->LootPackageCall), 0, WorldLevel);
		}

		return;
	}

	UFortItemDefinition* ItemDefinition = Cast<UFortItemDefinition>(LootPackageData->ItemDefinition.LoadSynchronous());

	if (ItemDefinition == NULL)
		return;

	UFortWeaponItemDefinition* WeaponDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition); // IM LAZY AND SICK OF THIS

	int32 LoadedAmmo = WeaponDefinition ? WeaponDefinition->GetClipSize() : 0;

	int32 ItemLevel = 0;

	int32 Count = LootPackageData->Count;

	if (Count <= 0)
		return;

	int32 FinalItemLevel = 0;

	if (ItemLevel >= 0)
		FinalItemLevel = ItemLevel;

	while (Count > 0)
	{
		int32 MaxStackSize = ItemDefinition->MaxStackSize.GetValueAtLevel(0);

		int32 CurrentCountForEntry = MaxStackSize;

		if (Count <= MaxStackSize)
			CurrentCountForEntry = Count;

		if (CurrentCountForEntry <= 0)
			CurrentCountForEntry = 0;

		int32 ActualItemLevel = 0;/*Framework::PickLevel(WorldItemDefinition, FinalItemLevel);*/

		bool bHasCombined = false;

		for (FFortItemEntry& CurrentLootDrop : *OutItemEntries)
		{
			if (CurrentLootDrop.ItemDefinition == ItemDefinition)
			{
				int NewCount = CurrentLootDrop.Count + CurrentCountForEntry;

				if (NewCount <= ItemDefinition->MaxStackSize.GetValueAtLevel(0))
				{
					bHasCombined = true;
					CurrentLootDrop.Count = NewCount;
				}
			}
		}

		if (!bHasCombined)
		{
			FFortItemEntry ItemEntry;
			ItemEntry.FortItemEntry(ItemDefinition, CurrentCountForEntry, ActualItemLevel, FGuid());

			ItemEntry.LoadedAmmo = LoadedAmmo;

			OutItemEntries->Add(ItemEntry);
		}

		bool IsWeapon = LootPackageData->LootPackageID.ToString().contains(".Weapon.") && WeaponDefinition;

		if (IsWeapon)
		{
			UFortWorldItemDefinition* AmmoWorldItemDefinition = WeaponDefinition->GetAmmoWorldItemDefinition_BP();

			if (AmmoWorldItemDefinition)
			{
				FFortItemEntry ItemEntry;
				ItemEntry.FortItemEntry(AmmoWorldItemDefinition, AmmoWorldItemDefinition->DropCount, 0, FGuid());

				OutItemEntries->Add(ItemEntry);
			}
		}

		Count -= CurrentCountForEntry;
	}
}

bool UFortLootPackage::PickLootDrops(TArray<FFortItemEntry>* OutLootToDrop, const FName TierGroupName, const int32 WorldLevel, const int32 ForcedLootTier)
{
	static TArray<UDataTable*> LootTierDataTables;
	static TArray<UDataTable*> LootPackageDataTables;

	if (LootTierDataTables.Num() <= 0 || LootPackageDataTables.Num() <= 0)
	{
		GetLootTierDataTables(&LootTierDataTables);
		GetLootPackageDataTables(&LootPackageDataTables);
	}

	if (LootTierDataTables.Num() <= 0 || LootPackageDataTables.Num() <= 0)
		return false;

	FFortLootTierData* LootTierData = PickLootTierData(LootTierDataTables, TierGroupName);

	if (LootTierData == NULL)
		return false;

	int32 NumLootPackageDrops = UKismetMathLibrary::FFloor(LootTierData->NumLootPackageDrops);

	float Remaining = LootTierData->NumLootPackageDrops - NumLootPackageDrops;

	if (Remaining > 0.0000099999997f)
		NumLootPackageDrops += Remaining >= (rand() * 0.00003051851f);

	OutLootToDrop->Reserve(NumLootPackageDrops);

	for (int32 i = 0; i < NumLootPackageDrops; ++i)
	{
		if (i >= LootTierData->LootPackageCategoryMinArray.Num())
			break;

		for (int32 j = 0; j < LootTierData->LootPackageCategoryMinArray[i]; ++j)
		{
			if (LootTierData->LootPackageCategoryMinArray[i] < 1)
				break;

			PickLootDropsFromLootPackage(OutLootToDrop, LootPackageDataTables, LootTierData->LootPackage, i, WorldLevel);
		}
	}

	return (OutLootToDrop->Num() > 0);
}

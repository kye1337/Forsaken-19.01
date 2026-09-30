#pragma once

#include "CoreGlobals.h"
#include "Engine/DataTable.h"

class UFortLootPackage // This 100% needs to be worked on im just glad its done cause i wanna work on other features
{
public:
	static void GetLootTierDataTables(TArray<UDataTable*>* OutLootTierDataTables);
	static void GetLootPackageDataTables(TArray<UDataTable*>* OutLootPackageDataTables);
public:
	static FFortLootTierData* PickLootTierData(TArray<UDataTable*>& DataTables, const FName TierGroupName);
	static FFortLootPackageData* PickLootPackageData(TArray<UDataTable*>& DataTables, FName LootPackage, int32 LootPackageCategory, int32 WorldLevel);
public:
	static void PickLootDropsFromLootPackage(TArray<FFortItemEntry>* OutItemEntries, TArray<UDataTable*>& LootPackageDataTables, FName LootPackage, int32 LootPackageCategory, int32 WorldLevel);
public:
	static bool PickLootDrops(TArray<FFortItemEntry>* OutLootToDrop, const FName TierGroupName, const int32 WorldLevel, const int32 ForcedLootTier);
};

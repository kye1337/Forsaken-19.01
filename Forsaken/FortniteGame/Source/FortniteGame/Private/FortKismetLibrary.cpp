#include "FortKismetLibrary.h"

bool FortKismetLibrary::PickLootDrops(UObject* Context, FFrame& Stack, bool* Ret)
{
	UPROPERTY(UObject*, WorldContextObject);
	TARRAY(FFortItemEntry, OutLootToDrop);
	UPROPERTY(FName, TierGroupName);
	UPROPERTY(int32, WorldLevel);
	UPROPERTY(int32, ForcedLootTier);

	Original::PickLootDrops(Context, Stack, Ret);

	return *Ret = UFortLootPackage::PickLootDrops(&OutLootToDrop, TierGroupName, WorldLevel, ForcedLootTier);
}

AFortPickup* FortKismetLibrary::K2_SpawnPickupInWorld(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
	UPROPERTY(UObject*, WorldContextObject);
	UPROPERTY(UFortWorldItemDefinition*, ItemDefinition);
	UPROPERTY(int32, NumberToSpawn);
	UPROPERTY(FVector, Position);
	UPROPERTY(FVector, Direction);
	UPROPERTY(int32, OverrideMaxStackCount);
	UPROPERTY(bool, bToss);
	UPROPERTY(bool, bRandomRotation);
	UPROPERTY(bool, bBlockedFromAutoPickup);
	UPROPERTY(int32, PickupInstigatorHandle);
	UPROPERTY(EFortPickupSourceTypeFlag, SourceType);
	UPROPERTY(EFortPickupSpawnSource, Source);
	UPROPERTY(AFortPlayerController*, OptionalOwnerPC);
	UPROPERTY(bool, bPickupOnlyRelevantToOwner);

	FFortItemEntry ItemEntry;
	ItemEntry.FortItemEntry(ItemDefinition, NumberToSpawn, 0, FGuid());

	if (UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
	{
		ItemEntry.LoadedAmmo = WeaponItemDefinition->GetClipSize();

		if (WeaponItemDefinition->bUsesPhantomReserveAmmo)
			ItemEntry.PhantomReserveAmmo = (WeaponItemDefinition->GetInitialClips() - 1) * WeaponItemDefinition->GetClipSize();
	}

	static UWorld* (*Test)(UEngine*, UObject*, int) =
		decltype(Test)(InSDKUtils::GetImageBase() + 0xB843A8);

	auto world = Test(UEngine::GetEngine(), WorldContextObject, 1);

	FortPickupCreationData CreationData = FortPickupCreationData(world, &ItemEntry, Position, FRotator(), OptionalOwnerPC, NULL, NULL, SourceType, Source);

	CreationData.bRandomRotation = bRandomRotation;
	CreationData.bPickupOnlyRelevantToOwner = bPickupOnlyRelevantToOwner;

	AFortPickup* Pickup = AFortPickup::CreateFromData(&CreationData);

	if (Pickup)
	{
		Pickup->bWeaponsCanBeAutoPickups = bBlockedFromAutoPickup;
		Pickup->TossPickup(Position, OptionalOwnerPC ? OptionalOwnerPC->MyFortPawn : NULL, OverrideMaxStackCount, bToss, true, SourceType, Source);
	}

	Original::K2_SpawnPickupInWorld(Context, Stack, Ret);

	return *Ret = Pickup;
}

AFortPickup* FortKismetLibrary::SpawnItemVariantPickupInWorld(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
	UPROPERTY(UObject*, WorldContextObject);
	UPROPERTY(FSpawnItemVariantParams, Params_0);

	FFortItemEntry ItemEntry;
	ItemEntry.FortItemEntry(Params_0.WorldItemDefinition, Params_0.NumberToSpawn, 0, FGuid());

	if (UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(Params_0.WorldItemDefinition))
	{
		ItemEntry.LoadedAmmo = WeaponItemDefinition->GetClipSize();

		if (WeaponItemDefinition->bUsesPhantomReserveAmmo)
			ItemEntry.PhantomReserveAmmo = (WeaponItemDefinition->GetInitialClips() - 1) * WeaponItemDefinition->GetClipSize();
	}

	FortPickupCreationData CreationData = FortPickupCreationData(UWorld::GetWorld(), &ItemEntry, Params_0.Position, FRotator(Params_0.Direction.X, Params_0.Direction.Y, Params_0.Direction.Z), Params_0.OptionalOwnerPC, NULL, NULL, Params_0.SourceType, Params_0.Source);

	CreationData.bRandomRotation = Params_0.bRandomRotation;
	CreationData.bPickupOnlyRelevantToOwner = Params_0.bPickupOnlyRelevantToOwner;

	AFortPickup* Pickup = AFortPickup::CreateFromData(&CreationData);

	if (Pickup)
	{
		Pickup->bWeaponsCanBeAutoPickups = Params_0.bBlockedFromAutoPickup;
		Pickup->TossPickup(Params_0.Position, Params_0.OptionalOwnerPC ? Params_0.OptionalOwnerPC->MyFortPawn : NULL, Params_0.OverrideMaxStackCount, Params_0.bToss, true, Params_0.SourceType, Params_0.Source);
	}

	Original::SpawnItemVariantPickupInWorld(Context, Stack, Ret);

	return *Ret = Pickup;
}

UFortWorldItem* FortKismetLibrary::GiveItemToInventoryOwner(UObject* Context, FFrame& Stack, UFortWorldItem** Ret)
{
	UPROPERTY(TScriptInterface<IFortInventoryOwnerInterface>, InventoryOwner);
	UPROPERTY(UFortWorldItemDefinition*, ItemDefinition);
	UPROPERTY(FGuid, ItemVariantGuid);
	UPROPERTY(int32, NumberToGive);
	UPROPERTY(bool, bNotifyPlayer);
	UPROPERTY(int32, ItemLevel);
	UPROPERTY(int32, PickupInstigatorHandle);
	UPROPERTY(bool, bUseItemPickupAnalyticEvent);

	if (AFortPlayerController* PlayerController = Cast<AFortPlayerController>(InventoryOwner.ObjectPointer))
	{
		if (AFortInventory* WorldInventory = PlayerController->WorldInventory)
		{
			FortInventory::CreateItem(WorldInventory, ItemDefinition, NumberToGive, ItemDefinition->GetInitialAmmo(ItemLevel), 0);
		}
	}

	Original::GiveItemToInventoryOwner(Context, Stack, Ret);

	return *Ret = NULL;
}

AFortAIDirector* FortKismetLibrary::GetAIDirector(UObject* Context, FFrame& Stack, AFortAIDirector** Ret)
{
	UPROPERTY(UObject*, WorldContextObject);

	Stack.Code += Stack.Code != NULL;

	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

	if (GameMode == NULL)
		return *Ret = NULL;

	AFortAIDirector* AIDirector = GameMode->AIDirector;

	if (AIDirector == NULL)
		return *Ret = NULL;

	return *Ret = AIDirector;
}

AFortAIGoalManager* FortKismetLibrary::GetAIGoalManager(UObject* Context, FFrame& Stack, AFortAIGoalManager** Ret)
{
	UPROPERTY(UObject*, WorldContextObject);

	Stack.Code += Stack.Code != NULL;

	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

	if (GameMode == NULL)
		return *Ret = NULL;

	AFortAIGoalManager* AIGoalManager = GameMode->AIGoalManager;

	if (AIGoalManager == NULL)
		return *Ret = NULL;

	return *Ret = AIGoalManager;
}

void FortKismetLibrary::Setup()
{
	UFunction* execPickLootDrops = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortKismetLibrary.PickLootDrops"));

	UFunction* execK2_SpawnPickupInWorld = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorld"));
	UFunction* execSpawnItemVariantPickupInWorld = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortKismetLibrary.SpawnItemVariantPickupInWorld"));

	UFunction* execGiveItemToInventoryOwner = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner"));

	UKismetDetourLibrary::InitializeDetour<UFortKismetLibrary, Exec>(execPickLootDrops, PickLootDrops, &Original::PickLootDrops);

	UKismetDetourLibrary::InitializeDetour<UFortKismetLibrary, Exec>(execK2_SpawnPickupInWorld, K2_SpawnPickupInWorld, &Original::K2_SpawnPickupInWorld);
	UKismetDetourLibrary::InitializeDetour<UFortKismetLibrary, Exec>(execSpawnItemVariantPickupInWorld, SpawnItemVariantPickupInWorld, &Original::SpawnItemVariantPickupInWorld);

	UKismetDetourLibrary::InitializeDetour<UFortKismetLibrary, Exec>(execGiveItemToInventoryOwner, GiveItemToInventoryOwner, &Original::GiveItemToInventoryOwner);

	UFunction* execGetAIDirector = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortKismetLibrary.GetAIDirector"));
	UFunction* execGetAIGoalManager = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortKismetLibrary.GetAIGoalManager"));

	UKismetDetourLibrary::InitializeDetour<UFortKismetLibrary, Exec>(execGetAIDirector, GetAIDirector);
	UKismetDetourLibrary::InitializeDetour<UFortKismetLibrary, Exec>(execGetAIGoalManager, GetAIGoalManager);
}

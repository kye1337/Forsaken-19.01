#include "FortniteGame/Source/FortniteAI/Public/AI/FortAthenaAIBotController.h"
#include "FortniteGame/Source/FortniteAI/Public/AI/Inventory/FortBotInventoryInfo.h"
#include "Items/FortInventory.h"
#include <random>
#include <thread>

UFortWorldItem* FortAthenaAIBotController::CreateItem(AFortAthenaAIBotController* BotController, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo, int32 PhantomReserveAmmo)
{
	if (Count <= 0)
		return NULL;

	AFortInventory* WorldInventory = BotController->Inventory;

	if (WorldInventory == NULL)
		return NULL;

	UFortWorldItem* WorldItem = Cast<UFortWorldItem>(ItemDefinition->CreateTemporaryItemInstanceBP(Count, 0));

	if (WorldItem == NULL)
		return NULL;

	WorldItem->OwnerInventory = WorldInventory;

	WorldItem->ItemEntry.ItemDefinition = ItemDefinition;
	WorldItem->ItemEntry.Count = Count;
	WorldItem->ItemEntry.LoadedAmmo = LoadedAmmo;
	WorldItem->ItemEntry.PhantomReserveAmmo = PhantomReserveAmmo;

	WorldInventory->Inventory.ItemInstances.Add(WorldItem);
	WorldInventory->Inventory.ReplicatedEntries.Add(WorldItem->ItemEntry);
	WorldInventory->Inventory.MarkItemDirty(WorldItem->ItemEntry);
	WorldInventory->HandleInventoryLocalUpdate();

	if (ItemDefinition->GetQuickbar() == Primary || ItemDefinition->GetQuickbar() == Harvesting)
	{
		for (int32 i = 0; i < 6; ++i)
		{
			FFortBotInventoryInfo& SlotItem = BotController->SlotItems[i];

			if (SlotItem.FortItem == NULL)
			{
				FortBotInventoryInfo::SetItem(&SlotItem, WorldItem);
				break;
			}
		}
	}

	return WorldItem;
}

void FortAthenaAIBotController::RemoveAllInventoryItems(AFortAthenaAIBotController* BotController)
{
	AFortInventory* WorldInventory = BotController->Inventory;

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

void FortAthenaAIBotController::OnPawnAISpawned(AActor* Actor, AFortPawn* AIPawn)
{
	Original::OnPawnAISpawned(Actor, AIPawn);

	UTexture2D* Texture = StaticLoadObject<UTexture2D>(L"/Game/Athena/HUD/Spectator/DeimosRiftMapIcon.DeimosRiftMapIcon");

	AFortAthenaAIBotController* BotController = Cast<AFortAthenaAIBotController>(AIPawn->Controller);

	if (BotController == NULL)
		return;

	if (Texture == NULL)
		return;

	BotController->DebugMinimapData.bIsOverridden = true;
	BotController->DebugMinimapData.DebugMinimapIconBrush.DrawAs = ESlateBrushDrawType::Image;
	BotController->DebugMinimapData.DebugMinimapIconBrush.SetResourceObject(Texture);
	BotController->DebugMinimapData.DebugMinimapIconBrush.ImageSize = FVector2D(4.f, 4.f);
	BotController->DebugMinimapData.DebugMinimapIconScale = FVector2D(4.f, 4.f);

	BotController->CachedAIServicePlayerBots->AddBotOnMinimap(BotController);

	if (!BotController->bIsAnAthenaGameParticipant)
	{
		auto PC = (AFortAthenaAIBotController*)AIPawn->Controller;
		auto PlayerState = (AFortPlayerStateAthena*)AIPawn->PlayerState;

		UFortNPCConversationParticipantComponent* bruh = (UFortNPCConversationParticipantComponent*)AIPawn->GetComponentByClass(UFortNPCConversationParticipantComponent::StaticClass());

		if (!bruh)
			return;

		bruh->BotControllerOwner = BotController;
		bruh->PlayerPawnOwner = Cast<AFortPlayerPawn>(AIPawn);

		int MaxServices = bruh->MaxServices;
		bruh->bCanStartConversation = true;
		bruh->OnRep_CanStartConversation();

		std::cout << bruh->SalesInventory->GetFullName() << std::endl;

		UDataTable* NPCSales_S19 = StaticLoadObject<UDataTable>(TEXT("/BattlePassS19/DataTables/NPCSales_S19.NPCSales_S19"));

		if (NPCSales_S19 == NULL)
		{
			std::cout << "HI IM NULL LOLOLOLOLOLOLOLOLOKL" << std::endl;
			return;
		}

		for (const auto& [RowName, RowValue] : NPCSales_S19->RowMap)
		{
			if (!RowName.ComparisonIndex || RowValue == NULL)
				continue;

			FNPCSaleInventoryRow* SaleInventoryRow = (FNPCSaleInventoryRow*)RowValue;

			if (SaleInventoryRow == NULL)
				continue;

			std::cout << bruh->CharacterData->GameplayTag.TagName.ToString() << std::endl;
			std::cout << SaleInventoryRow->NPC.TagName.ToString() << std::endl;

			std::cout << BotController->BehaviorTree->GetFullName() << std::endl;

			if (SaleInventoryRow->NPC.TagName.ComparisonIndex != bruh->CharacterData->GameplayTag.TagName.ComparisonIndex)
				continue;

			std::cout << "HELLO: " << SaleInventoryRow->LootTier.ToString() << std::endl;

			bruh->SupportedSales.Add(*SaleInventoryRow);
		}
	}
	else
	{
		for (auto& StartitemItem : Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode)->StartingItems)
		{
			CreateItem(BotController, StartitemItem.Item, StartitemItem.Count, 0, 0);
		}
	}
}

AFortAthenaAIBotController* FortAthenaAIBotController::GetSquadLeader(AFortAthenaAIBotController* BotController)
{
	AFortAthenaAIBotController* SquadLeader = Original::GetSquadLeader(BotController);

	if (SquadLeader == NULL)
		SquadLeader = BotController;

	return SquadLeader;
}

void FortAthenaAIBotController::OnPossesedPawnDied(AFortAthenaAIBotController* BotController, AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser, const FVector& HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, const FVector& Momentum)
{
	Original::OnPossesedPawnDied(BotController, DamagedActor, Damage, InstigatedBy, DamageCauser, HitLocation, FHitComponent, BoneName, Momentum);

	if (InstigatedBy == NULL)
		return;

	AFortPlayerStateAthena* KillerPlayerState = Cast<AFortPlayerStateAthena>(InstigatedBy->PlayerState);

	if (!KillerPlayerState)
		return;

	AFortPlayerPawnAthena* KillerPawn = Cast<AFortPlayerPawnAthena>(InstigatedBy->Pawn);

	if (KillerPawn == NULL)
		return;

	AFortPlayerPawnAthena* DeadPawn = BotController->PlayerBotPawn;

	if (DeadPawn == NULL)
		return;

	AFortInventory* WorldInventory = BotController->Inventory;

	if (WorldInventory == NULL)
		return;

	const FVector DeathLocation = DeadPawn->K2_GetActorLocation();

	TArray<UFortWorldItem*> DroppableItems;

	if (FortInventory::GetDropableItems(WorldInventory, &DroppableItems))
	{
		for (UFortWorldItem* DroppableItem : DroppableItems)
		{
			FortPickupCreationData CreationData = FortPickupCreationData(UWorld::GetWorld(), &DroppableItem->ItemEntry, DeadPawn->K2_GetActorLocation(), DeadPawn->K2_GetActorRotation(), NULL, NULL, NULL, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination);

			AFortPickup* Pickup = AFortPickup::CreateFromData(&CreationData);

			if (Pickup != NULL)
			{
				Pickup->TossPickup(DeadPawn->K2_GetActorLocation(), NULL, 0, true, false, EFortPickupSourceTypeFlag::AI, EFortPickupSpawnSource::BotElimination);
			}
		}
	}

	// guy is skidding everything holy 

	if (AFortPlayerStateAthena* KilledPlayerState = Cast<AFortPlayerStateAthena>(BotController->PlayerState))
	{
		KilledPlayerState->PawnDeathLocation = DeathLocation;

		if (BotController->bIsAnAthenaGameParticipant)
		{
			RtlSecureZeroMemory(&KilledPlayerState->DeathInfo, 0x90);

			FGameplayTagContainer CopyTags;

			CopyTags.AppendTags(DeadPawn->DeathTags); // switch to the offset one that we don't know the offset to

			KilledPlayerState->DeathInfo.bInitialized = true;

			KilledPlayerState->DeathInfo.bDBNO = DeadPawn->IsDBNO() /*KilledPawn->bWasDBNOOnDeath*/;
			KilledPlayerState->DeathInfo.DeathLocation = DeathLocation;
			KilledPlayerState->DeathInfo.DeathTags = CopyTags;

			KilledPlayerState->DeathInfo.DeathCause = AFortPlayerStateAthena::ToDeathCause(CopyTags, KilledPlayerState->DeathInfo.bDBNO);

			AFortPlayerStateAthena* KillerPlayerState = Cast<AFortPlayerStateAthena>(InstigatedBy->PlayerState);

			if (KillerPlayerState)
			{
				KilledPlayerState->DeathInfo.FinisherOrDowner = KillerPlayerState;
			}
			else
			{
				KilledPlayerState->DeathInfo.FinisherOrDowner = KilledPlayerState;
			}

			if (KilledPlayerState->DeathInfo.DeathCause == EDeathCause::FallDamage)
			{
				KilledPlayerState->DeathInfo.Distance = DeadPawn->LastFallDistance;
			}
			else
			{
				KilledPlayerState->DeathInfo.Distance = DeadPawn->GetDistanceTo(InstigatedBy->Pawn);
			}

			KilledPlayerState->OnRep_DeathInfo();

			if (KillerPlayerState && KillerPlayerState != KilledPlayerState)
			{
				++KillerPlayerState->KillScore;

				KillerPlayerState->OnRep_Kills();
				KillerPlayerState->ClientReportKill(KilledPlayerState);

				if (AFortTeamInfo* PlayerTeam = KillerPlayerState->PlayerTeam)
				{
					for (AController* TeamMember : PlayerTeam->TeamMembers)
					{
						AFortPlayerStateAthena* TeamMemberState = Cast<AFortPlayerStateAthena>(TeamMember->PlayerState);

						if (!TeamMemberState)
							continue;

						++TeamMemberState->TeamKillScore;

						TeamMemberState->OnRep_TeamKillScore();
						TeamMemberState->ClientReportTeamKill(1);
					}
				}
			}

			static FName const NAME_AthenaKillsStat = UKismetStringLibrary::Conv_StringToName(L"AthenaKills");

			if (AFortPlayerPawn* KillerPawn = Cast<AFortPlayerPawn>(InstigatedBy->Pawn))
			{
				AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(KillerPawn->Controller);

				if (PlayerController != NULL)
				{
					PlayerController->ModifyStat(NAME_AthenaKillsStat, 1, EStatMod::Delta, true);
				}
			}
		}

		if (KillerPlayerState->Place == 1)
		{
			auto KillerController = Cast<AFortPlayerControllerAthena>(KillerPlayerState->GetOwner());

			UFortWeaponItemDefinition* WeaponItemDefinition = NULL;

			if (DamageCauser)
			{
				if (DamageCauser->IsA(AFortProjectileBase::StaticClass()))
				{
					if (AFortWeapon* Weapon = Cast<AFortWeapon>(DamageCauser->Owner))
					{
						WeaponItemDefinition = Weapon->WeaponData;
					}
				}

				if (AFortWeapon* Weapon = Cast<AFortWeapon>(DamageCauser))
				{
					WeaponItemDefinition = Weapon->WeaponData;
				}
			}

			KillerController->ClientNotifyWon(KillerController->Pawn, WeaponItemDefinition, KilledPlayerState->DeathInfo.DeathCause);
			KillerController->ClientNotifyTeamWon(KillerController->Pawn, WeaponItemDefinition, KilledPlayerState->DeathInfo.DeathCause);
			KillerController->PlayWinEffects(KillerController->Pawn, WeaponItemDefinition, KilledPlayerState->DeathInfo.DeathCause, false);

			for (AFortPlayerControllerAthena* AlivePlayer : Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode)->AlivePlayers)
			{
				if (AFortInventory* Inventory = AlivePlayer->WorldInventory)
				{
					UAthenaGadgetItemDefinition* VictoryCrown = StaticFindObject<UAthenaGadgetItemDefinition>(
						TEXT("/VictoryCrownsGameplay/Items/AGID_VictoryCrown.AGID_VictoryCrown"));

					FFortItemEntry* ItemEntry = FortInventory::GetItemEntry(Inventory, VictoryCrown);

					if (ItemEntry == NULL)
					{
						FortInventory::CreateItem(Inventory, VictoryCrown);
					}
				}
			}
		}
	}
}

void FortAthenaAIBotController::Setup()
{
	UKismetDetourLibrary::InitializeDetour<AFortAthenaAIBotController, MinHook>(0x5FAC0B4, OnPawnAISpawned, &Original::OnPawnAISpawned);
	UKismetDetourLibrary::InitializeDetour<AFortAthenaAIBotController, MinHook>(0x5DDD648, GetSquadLeader, &Original::GetSquadLeader);
	UKismetDetourLibrary::InitializeDetour<AFortAthenaAIBotController, MinHook>(0x5DE38D8, OnPossesedPawnDied, &Original::OnPossesedPawnDied);
}

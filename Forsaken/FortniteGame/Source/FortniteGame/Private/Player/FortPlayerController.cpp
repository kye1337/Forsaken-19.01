#include "Player/FortPlayerController.h"
#include "Items/FortInventory.h"
#include <ranges>
#include <algorithm>

void FortPlayerController::ServerExecuteInventoryItem(AFortPlayerController* PlayerController, const FGuid& ItemGuid)
{
	AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);

	if (Pawn == NULL)
		return;

	AFortInventory* WorldInventory = PlayerController->WorldInventory;

	if (WorldInventory == NULL)
		return;

	FFortItemEntry* ItemEntry = FortInventory::GetItemEntry(WorldInventory, ItemGuid);

	if (ItemEntry == NULL)
		return;

	UFortItemDefinition* ItemDefinition = ItemEntry->ItemDefinition;

	if (ItemDefinition == NULL)
		return;

	if (UFortDecoItemDefinition* DecoItemDefinition = Cast<UFortDecoItemDefinition>(ItemDefinition))
	{
		Pawn->PickUpActor(NULL, DecoItemDefinition);
		Pawn->CurrentWeapon->ItemEntryGuid = ItemGuid;

		if (AFortDecoTool_ContextTrap* DecoTool = Cast<AFortDecoTool_ContextTrap>(Pawn->CurrentWeapon))
		{
			DecoTool->ContextTrapItemDefinition = Cast<UFortContextTrapItemDefinition>(ItemDefinition);
		}
	}
	else if (UFortGadgetItemDefinition* GadgetItemDefinition = Cast<UFortGadgetItemDefinition>(ItemDefinition))
	{
		Pawn->EquipWeaponDefinition(GadgetItemDefinition->GetWeaponItemDefinition(), ItemGuid, ItemEntry->TrackerGuid, false);
	}

	UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition);

	if (WeaponItemDefinition == NULL)
		return;

	Pawn->EquipWeaponDefinition(WeaponItemDefinition, ItemGuid, ItemEntry->TrackerGuid, false);
}

void FortPlayerController::ServerCreateBuildingActor(AFortPlayerController* PlayerController, const FCreateBuildingActorData& CreateBuildingData)
{
	if (PlayerController->IsInAircraft())
		return;

	AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerController);

	if (PlayerControllerAthena == NULL)
		return;

	AFortInventory* WorldInventory = PlayerControllerAthena->WorldInventory;

	if (WorldInventory == NULL)
		return;

	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

	if (GameState == NULL)
		return;

	UClass* BuildingSMActorClass = PlayerControllerAthena->BroadcastRemoteClientInfo->RemoteBuildableClass.Get();

	if (BuildingSMActorClass == NULL)
		return;

	TArray<ABuildingActor*> ExistingBuildings;
	EFortBuildPreviewMarkerOptionalAdjustment MarkerOptionalAdjustment;

	EFortStructuralGridQueryResults QueryResults = GameState->StructuralSupportSystem->CanAddBuildingActorClassToGrid(UWorld::GetWorld(), BuildingSMActorClass, CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot,
		CreateBuildingData.bMirrored, &ExistingBuildings, &MarkerOptionalAdjustment, false);

	if (QueryResults == EFortStructuralGridQueryResults::CanAdd)
	{
		for (ABuildingActor* ExistingBuilding : ExistingBuildings)
		{
			ExistingBuilding->K2_DestroyActor();
		}

		ABuildingSMActor* BuildingSMActor = SpawnActor<ABuildingSMActor>(BuildingSMActorClass, CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot);

		BuildingSMActor->InitializeKismetSpawnedBuildingActor(BuildingSMActor, PlayerController, true, NULL);

		BuildingSMActor->bPlayerPlaced = true;

		if (AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState))
		{
			BuildingSMActor->PlacedByPlayer(PlayerState);

			BuildingSMActor->SetTeam(PlayerState->TeamIndex);
			BuildingSMActor->OnRep_Team();
		}

		UFortResourceItemDefinition* ResourceItemDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingSMActor->ResourceType); // scuffed but proper was crashing

		if (ResourceItemDefinition == NULL)
			return;

		FFortItemEntry* ItemEntry = FortInventory::GetItemEntry(WorldInventory, ResourceItemDefinition);

		if (ItemEntry == NULL)
			return;

		FortInventory::RemoveItem(WorldInventory, ItemEntry->ItemGuid, 10);
	}

	if (ExistingBuildings.IsValid())
		ExistingBuildings.Free();
}

void FortPlayerController::ServerBeginEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit)
{
	if (BuildingActorToEdit == NULL || !BuildingActorToEdit->CanBePlayerEdited(PlayerController))
		return;

	AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);

	if (Pawn == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	AFortInventory* WorldInventory = PlayerController->WorldInventory;

	if (WorldInventory == NULL)
		return;

	FFortItemEntry* ItemEntry = FortInventory::GetItemEntry(WorldInventory, UFortEditToolItemDefinition::StaticClass());

	if (ItemEntry == NULL)
		return;

	UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry->ItemDefinition);

	if (WeaponItemDefinition == NULL)
		return;

	AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(Pawn->EquipWeaponDefinition(WeaponItemDefinition, ItemEntry->ItemGuid, ItemEntry->TrackerGuid, false));

	if (EditingTool == NULL)
		return;

	BuildingActorToEdit->SetEditingPlayer(PlayerState);

	EditingTool->EditActor = BuildingActorToEdit;
	EditingTool->OnRep_EditActor();
}

void FortPlayerController::ServerEditBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit, TSubclassOf<ABuildingSMActor> NewBuildingClass, uint8 RotationIterations, bool bMirrored)
{
	if (BuildingActorToEdit == NULL || BuildingActorToEdit->EditingPlayer != PlayerController->PlayerState || BuildingActorToEdit->bDestroyed)
		return;

	BuildingActorToEdit->SetEditingPlayer(NULL);

	int32 CurrentBuildingLevel = BuildingActorToEdit->CurrentBuildingLevel;
	BuildingActorToEdit->ReplaceBuildingActor(EBuildingReplacementType::BRT_Edited, NewBuildingClass, CurrentBuildingLevel, RotationIterations, bMirrored, PlayerController);
}

void FortPlayerController::ServerEndEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToStopEditing)
{
	if (BuildingActorToStopEditing == NULL || BuildingActorToStopEditing->EditingPlayer != PlayerController->PlayerState || BuildingActorToStopEditing->bDestroyed)
		return;

	AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);

	if (Pawn == NULL)
		return;

	AFortInventory* WorldInventory = PlayerController->WorldInventory;

	if (WorldInventory == NULL)
		return;

	FFortItemEntry* ItemEntry = FortInventory::GetItemEntry(WorldInventory, UFortEditToolItemDefinition::StaticClass());

	if (ItemEntry == NULL)
		return;

	UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry->ItemDefinition);

	if (WeaponItemDefinition == NULL)
		return;

	AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(Pawn->EquipWeaponDefinition(WeaponItemDefinition, ItemEntry->ItemGuid, ItemEntry->TrackerGuid, false));

	if (EditingTool == NULL)
		return;

	BuildingActorToStopEditing->SetEditingPlayer(NULL);

	EditingTool->EditActor = NULL;
	EditingTool->OnRep_EditActor();
}

void FortPlayerController::ServerAttemptInventoryDrop(AFortPlayerController* PlayerController, const FGuid& ItemGuid, int32 Count, bool bTrash)
{
	std::cout << __FUNCTION__ << std::endl;

	FFortItemEntry* ItemEntry = FortInventory::GetItemEntry(PlayerController->WorldInventory, ItemGuid);

	if (ItemEntry == NULL)
		return;

	FFortItemEntry NewEntryData = *ItemEntry;
	NewEntryData.Count = Count;

	FortPickupCreationData CreationData = FortPickupCreationData(UWorld::GetWorld(), &NewEntryData, PlayerController->MyFortPawn->K2_GetActorLocation(), FRotator(), NULL, NULL, NULL, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::TossedByPlayer);

	auto Pickup = AFortPickup::CreateFromData(&CreationData);

	Pickup->PawnWhoDroppedPickup = PlayerController->MyFortPawn;

	auto hello = PlayerController->MyFortPawn->K2_GetActorLocation() + (PlayerController->MyFortPawn->GetActorForwardVector() * UKismetMathLibrary::RandomFloatInRange(500.f, 600.f));

	Pickup->TossPickup(hello, PlayerController->MyFortPawn, 0, true, true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::TossedByPlayer);

	FortInventory::RemoveItem(PlayerController->WorldInventory, ItemGuid, Count);
}

void FortPlayerController::ServerPlayEmoteItem(AFortPlayerController* PlayerController, UFortMontageItemDefinitionBase* EmoteAsset, float EmoteRandomNumber)
{
	AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);

	if (Pawn == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	UAbilitySystemComponent* AbilitySystemComponent = PlayerState->AbilitySystemComponent;

	if (AbilitySystemComponent == NULL)
		return;

	UClass* EmoteAbilityClass = NULL;

	if (UAthenaSprayItemDefinition* SprayItemDefinition = Cast<UAthenaSprayItemDefinition>(EmoteAsset))
	{
		EmoteAbilityClass = UGAB_Spray_Generic_C::StaticClass(); // todo
	}
	else if (UAthenaToyItemDefinition* ToyItemDefinition = Cast<UAthenaToyItemDefinition>(EmoteAsset))
	{
		EmoteAbilityClass = ToyItemDefinition->ToySpawnAbility.Get();
	}
	else if (UAthenaDanceItemDefinition* DanceItemDefinition = Cast<UAthenaDanceItemDefinition>(EmoteAsset))
	{
		EmoteAbilityClass = UGAB_Emote_Generic_C::StaticClass();

		Pawn->bEmoteUsesSecondaryFire = DanceItemDefinition->bUsesSecondaryFireInput;

		Pawn->bMovingEmote = DanceItemDefinition->bMovingEmote;
		Pawn->bMovingEmoteSkipLandingFX = DanceItemDefinition->bMovingEmoteSkipLandingFX;
		Pawn->bMovingEmoteForwardOnly = DanceItemDefinition->bMoveForwardOnly;
		Pawn->bMovingEmoteFollowingOnly = DanceItemDefinition->bMoveFollowingOnly;

		Pawn->EmoteWalkSpeed = DanceItemDefinition->WalkForwardSpeed;

		if (DanceItemDefinition->bGroupEmote)
		{
			Pawn->bLockGroupEmoteLeaderRotation = DanceItemDefinition->bLockGroupEmoteLeaderRotation;
		}
	}

	if (EmoteAbilityClass == NULL)
		return;

	UGameplayAbility* GameplayAbility = Cast<UGameplayAbility>(EmoteAbilityClass->DefaultObject);

	if (GameplayAbility == NULL)
		return;

	FGameplayAbilitySpec Spec;
	Spec.GameplayAbilitySpec(GameplayAbility, 1, -1, EmoteAsset);

	AbilitySystemComponent->GiveAbilityAndActivateOnce(&Spec.Handle, Spec);

	const UFortItemDefinition* LastReplicatedEmote = Pawn->LastReplicatedEmoteExecuted;

	Pawn->LastReplicatedEmoteExecuted = EmoteAsset;
	Pawn->OnRep_LastReplicatedEmoteExecuted(LastReplicatedEmote);
}

void FortPlayerController::ServerRepairBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToRepair)
{
	if (BuildingActorToRepair == NULL || BuildingActorToRepair->bActorIsBeingDestroyed || BuildingActorToRepair->bDestroyed)
		return;

	int32 RepairCost = PlayerController->PayBuildingRepairCost(BuildingActorToRepair);

	BuildingActorToRepair->RepairBuilding(PlayerController, RepairCost);
}

void FortPlayerController::ServerCheat(AFortPlayerController* PlayerController, const FString& Msg)
{
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (!PlayerState)
		return;

	std::vector<std::string> Arguments;
	auto OldMsg = Msg.ToString();

	auto ReceivingController = PlayerController;
	auto ReceivingPlayerState = PlayerState;
	auto ReceivingPawn = ReceivingController->Pawn;
	auto ReceivingASC = ((AFortPlayerStateAthena*)ReceivingController->PlayerState)->AbilitySystemComponent;

	auto firstBackslash = OldMsg.find_first_of("\\");
	auto lastBackslash = OldMsg.find_last_of("\\");

	auto& ClientConnections = UWorld::GetWorld()->NetDriver->ClientConnections;

	if (firstBackslash != std::string::npos && lastBackslash != std::string::npos)
	{
		if (firstBackslash != lastBackslash)
		{
			std::string player = OldMsg;

			player = player.substr(firstBackslash + 1, lastBackslash - firstBackslash - 1);

			for (int i = 0; i < ClientConnections.Num(); ++i)
			{
				auto CurrentPlayerController = (AFortPlayerControllerAthena*)ClientConnections[i]->PlayerController;

				if (!CurrentPlayerController)
					continue;

				auto CurrentPlayerState = (AFortPlayerStateAthena*)CurrentPlayerController->PlayerState;

				if (!CurrentPlayerState)
					continue;

				auto CurrentASC = PlayerState->AbilitySystemComponent;
				auto CurrentPawn = CurrentPlayerController->Pawn;

				FString PlayerName = CurrentPlayerState->GetPlayerName();

				if (PlayerName.ToString() == player)
				{
					ReceivingController = CurrentPlayerController;
					ReceivingPlayerState = CurrentPlayerState;

					ReceivingPawn = CurrentPawn;
					ReceivingASC = CurrentASC;

					PlayerName.Clear();
					break;
				}

				PlayerName.Clear();
			}
		}
		else
		{
			//SendMessageToConsole(PlayerController, L"Warning: You have a backslash but no ending backslash, was this by mistake? Executing on you.");
		}
	}

	if (!ReceivingController || !ReceivingPlayerState)
	{
		//SendMessageToConsole(PlayerController, L"Unable to find player!");
		return;
	}

	{
		auto Message = Msg.ToString();

		size_t start = Message.find('\\');

		while (start != std::string::npos)
		{
			size_t end = Message.find('\\', start + 1);

			if (end == std::string::npos)
				break;

			Message.replace(start, end - start + 2, "");
			start = Message.find('\\');
		}

		int zz = 0;

		while (Message.find(" ") != std::string::npos)
		{
			auto arg = Message.substr(0, Message.find(' '));
			Arguments.push_back(arg);
			Message.erase(0, Message.find(' ') + 1);
			zz++;
		}
		{
			Arguments.push_back(Message);
			zz++;
		}
	}

	auto NumArgs = Arguments.size() == 0 ? 0 : Arguments.size() - 1;

	auto& Command = Arguments[0];
	std::transform(Command.begin(), Command.end(), Command.begin(), ::tolower);

	if (Command == "giveitem" || Command == "give" || Command == "grant" || Command == "grantitem")
	{
		if (NumArgs < 1)
		{
			//SendMessageToConsole(PlayerController, L"Please provide a WID!");
			return;
		}

		auto& weaponName = Arguments[1];
		int count = 1;

		try
		{
			if (NumArgs >= 2)
				count = std::stoi(Arguments[2]);
		}
		catch (...) {}

		UFortWorldItemDefinition* WorldItemDefinition = StaticFindObject<UFortWorldItemDefinition>(std::wstring(weaponName.begin(), weaponName.end()).c_str(), ANY_PACKAGE);

		if (!WorldItemDefinition)
		{
			//SendMessageToConsole(PlayerController, L"Invalid WID!\n");
			return;
		}

		FortInventory::CreateItem(ReceivingController->WorldInventory, WorldItemDefinition, count);

		//SendMessageToConsole(PlayerController, L"Granted item!\n");
	}
	else if (Command == "bugitgo")
	{
		if (Arguments.size() <= 3)
		{
			//SendMessageToConsole(PlayerController, L"Please provide X, Y, and Z!\n");
			return;
		}

		float X{}, Y{}, Z{};

		try { X = std::stof(Arguments[1]); }
		catch (...) {}
		try { Y = std::stof(Arguments[2]); }
		catch (...) {}
		try { Z = std::stof(Arguments[3]); }
		catch (...) {}

		auto Pawn = Cast<AFortPlayerPawnAthena>(ReceivingController->Pawn);

		if (!Pawn)
		{
			//SendMessageToConsole(PlayerController, L"No pawn to teleport!");
			return;
		}

		Pawn->K2_TeleportTo(FVector(X, Y, Z), Pawn->K2_GetActorRotation());
	}
	else if (Command == "tp" || Command == "teleport")
	{
		if (UCheatManager* CheatManager = ReceivingController->CheatManager = NewObject<UCheatManager>(ReceivingController))
		{
			CheatManager->Teleport();
			CheatManager = NULL;
		}
	}
	else if (Command == "fly")
	{
		auto Pawn = (AFortPlayerPawnAthena*)ReceivingController->Pawn;

		if (!Pawn)
		{
			return;
		}

		Pawn->SetActorEnableCollision(true);

		if (UCharacterMovementComponent* CharacterMovement = Pawn->CharacterMovement)
		{
			if (CharacterMovement->bCheatFlying = ~CharacterMovement->bCheatFlying)
			{
				CharacterMovement->SetMovementMode(EMovementMode::MOVE_Flying, 0);
			}
			else
			{
				CharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking, 0);
			}
		}
	}

	else if (Command == "slomo" || Command == "slowmo")
	{
		if (UCheatManager* CheatManager = ReceivingController->CheatManager = NewObject<UCheatManager>(ReceivingController))
		{
			float Slomo = 0.f;

			if (NumArgs >= 1)
			{
				try { Slomo = std::stof(Arguments[1]); }
				catch (...) {}
			}

			CheatManager->Slomo(Slomo);
			CheatManager = NULL;
		}
	}
	else if (Command == "startaircraft")
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), TEXT("startaircraft"), nullptr);
	}
	else if (Command == "pausesafezone")
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), TEXT("pausesafezone"), nullptr);
	}
	else if (Command == "tpbot")
	{
		UAthenaAIServicePlayerBots* AIServicePlayerBots = UAthenaAIBlueprintLibrary::GetAIServicePlayerBots(UWorld::GetWorld());

		for (auto PlayerBot : AIServicePlayerBots->PlayerBots)
		{
			auto Controller = PlayerBot.BotController;

			Controller->PlayerBotPawn->K2_TeleportTo(ReceivingController->MyFortPawn->K2_GetActorLocation(), FRotator());
		}
	}
	else if (Command == "god")
	{
		auto Pawn = Cast<AFortPlayerPawnAthena>(ReceivingController->Pawn);

		if (Pawn != NULL)
		{
			Pawn->bIsInvulnerable = ~Pawn->bIsInvulnerable;
		}
	}
}

static inline void (*_ClientOnPawnDied)(AFortPlayerControllerAthena* PlayerController, const FFortPlayerDeathReport& DeathReport);
static inline void ClientOnPawnDied(AFortPlayerControllerAthena* PlayerController, const FFortPlayerDeathReport& DeathReport)
{
	static void (*RemoveFromAlivePlayers)(AFortGameModeAthena*, AFortPlayerController*, APlayerState*, APawn*, UFortWeaponItemDefinition*, uint8_t, bool) = decltype(RemoveFromAlivePlayers)(InSDKUtils::GetImageBase() + 0x606f83c);

	RemoveFromAlivePlayers(Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode), PlayerController, NULL, DeathReport.KillerPawn, NULL, 1, 0);

	static FName NAME_Spectating = UKismetStringLibrary::Conv_StringToName(L"Spectating");

	PlayerController->StateName = NAME_Spectating;

	return _ClientOnPawnDied(PlayerController, DeathReport);
}

void FortPlayerController::Setup()
{
	UKismetDetourLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x22C, ServerExecuteInventoryItem);
	UKismetDetourLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x24D, ServerCreateBuildingActor);
	UKismetDetourLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x254, ServerBeginEditingBuildingActor);
	UKismetDetourLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x24F, ServerEditBuildingActor);
	UKismetDetourLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x252, ServerEndEditingBuildingActor);
	UKismetDetourLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x11d0 / 8, ServerAttemptInventoryDrop);
	UKismetDetourLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0xf40 / 8, ServerPlayEmoteItem);
	UKismetDetourLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x1248 / 8, ServerRepairBuildingActor);
	UKismetDetourLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0xf30 / 8, ServerCheat);
	UKismetDetourLibrary::InitializeDetour<AFortPlayerControllerAthena, MinHook>(0x6bf8a6c, ClientOnPawnDied, &_ClientOnPawnDied);
}

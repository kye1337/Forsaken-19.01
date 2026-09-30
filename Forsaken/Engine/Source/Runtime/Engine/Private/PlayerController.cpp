#include "GameFramework/PlayerController.h"
#include "Items/FortInventory.h"

void PlayerController::ServerAcknowledgePossession(APlayerController* PlayerController, APawn* P)
{
	PlayerController->AcknowledgedPawn = P;

	AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerController);

	if (PlayerControllerAthena == NULL)
		return;

	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

	if (GameMode == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(PlayerState);

	IAbilitySystemInterface* Interface = GetInterfaceAddress<IAbilitySystemInterface>(PlayerState);

	if (Interface == NULL)
		return;

	UFortGlobals::InitializePlayerGameplayAbilities(Interface);

	if (!PlayerControllerAthena->bHasInitializedWorldInventory)
	{
		for (const FItemAndCount& StartingItem : GameMode->StartingItems)
		{
			if (StartingItem.Item->IsA(UFortSmartBuildingItemDefinition::StaticClass()))
				continue;

			FortInventory::CreateItem(PlayerControllerAthena->WorldInventory, StartingItem.Item, StartingItem.Count);
		}

		FortInventory::CreateItem(PlayerControllerAthena->WorldInventory, PlayerControllerAthena->CosmeticLoadoutPC.Pickaxe->WeaponDefinition);

		PlayerControllerAthena->bHasInitializedWorldInventory = true;
	}

	static bool bFirst = false;

	auto GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

	if (!bFirst)
	{
		bFirst = true;

		float Duration = 90.f;
		float EarlyDuration = Duration;

		float TimeSeconds = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());

		GameState->WarmupCountdownEndTime = TimeSeconds + Duration;
		GameMode->WarmupCountdownDuration = Duration;

		GameState->WarmupCountdownStartTime = TimeSeconds;
		GameMode->WarmupEarlyCountdownDuration = EarlyDuration;

		auto PatrolPaths = GetAllActorsOfClassEasy<AFortAthenaPatrolPath>();

		for (auto& PatrolPath : PatrolPaths)
		{
			for (auto Tag : PatrolPath->GameplayTags.GameplayTags)
			{
				std::cout << "TagName: " << Tag.TagName.ToString() << std::endl;

				/*if (Tag.TagName.ToString().contains("Labrador"))
				{
					auto LLAMAFRIEND = StaticLoadObject<UClass>(L"/Labrador/AISpawnerData/BP_AISpawnerData_Labrador.BP_AISpawnerData_Labrador_C");
					auto LLAMAFRIENDLISt = ((UFortAthenaAIBotSpawnerData*)LLAMAFRIEND)->CreateComponentListFromClass(LLAMAFRIEND, UWorld::GetWorld());

					((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(LLAMAFRIENDLISt, PatrolPath->GetTransform());
				}
				else if (Tag.TagName.ToString().contains("TameFoot"))
				{
					auto Hi = StaticLoadObject<UClass>(L"/TameFoot/NPCs/TameFoot/BP_AIBotSpawnerData_TameFoot.BP_AIBotSpawnerData_TameFoot_C");
					auto List = ((UFortAthenaAIBotSpawnerData*)Hi)->CreateComponentListFromClass(Hi, UWorld::GetWorld());

					std::cout << "HI IM TAMEFOOT" << std::endl;

					((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(List, PatrolPath->GetTransform());
				}
				else if (Tag.TagName.ToString().contains("TheVisitor"))
				{
					auto Hi = StaticLoadObject<UClass>(L"/NPCLibraryS19/NPCs/TheVisitor/BP_AIBotSpawnerData_NPC_S19_TheVisitor.BP_AIBotSpawnerData_NPC_S19_TheVisitor_C");
					auto List = ((UFortAthenaAIBotSpawnerData*)Hi)->CreateComponentListFromClass(Hi, UWorld::GetWorld());

					std::cout << "HI IM Visitor" << std::endl;

					((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(List, PatrolPath->GetTransform());
				}*/
			}
		}
	}

	if (auto XPComp = PlayerControllerAthena->XPComponent)
	{
		PlayerState->SeasonLevelUIDisplay = UKismetMathLibrary::RandomIntegerInRange(1, 300);
		PlayerState->OnRep_SeasonLevelUIDisplay();
	}
}

void PlayerController::Setup()
{
	UKismetDetourLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x122, ServerAcknowledgePossession);
}

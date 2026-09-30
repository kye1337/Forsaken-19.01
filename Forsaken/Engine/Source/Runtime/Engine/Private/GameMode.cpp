#include "GameFramework/GameMode.h"

bool GameMode::ReadyToStartMatch(AGameMode* GameMode)
{
	AFortGameModeAthena* GameModeAthena = Cast<AFortGameModeAthena>(GameMode);

	if (GameModeAthena == NULL)
		return false;

	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameModeAthena->GameState);

	if (GameState == NULL)
		return false;

	static bool bFoundations = false;

	if (!bFoundations)
	{
		bFoundations = TRUE;

		TArray<ABuildingFoundation*> Foundations = GetAllActorsOfClassEasy<ABuildingFoundation>();

		for (ABuildingFoundation* BuildingFoundation : Foundations)
		{
			if (BuildingFoundation == NULL || BuildingFoundation->DynamicFoundationType != EDynamicFoundationType::StartDisabled)
				continue;

			BuildingFoundation->bServerStreamedInLevel = true;
			BuildingFoundation->DynamicFoundationType = EDynamicFoundationType::Static;
			BuildingFoundation->OnRep_ServerStreamedInLevel();

			BuildingFoundation->FoundationEnabledState = EDynamicFoundationEnabledState::Enabled;

			BuildingFoundation->DynamicFoundationRepData.EnabledState = EDynamicFoundationEnabledState::Enabled;
			BuildingFoundation->DynamicFoundationTransform = BuildingFoundation->GetTransform();
			BuildingFoundation->DynamicFoundationRepData.Rotation = UKismetMathLibrary::Quat_Rotator(BuildingFoundation->DynamicFoundationTransform.Rotation);
			BuildingFoundation->OnRep_DynamicFoundationRepData();

			BuildingFoundation->FlushNetDormancy();
			BuildingFoundation->ForceNetUpdate();

		}

		Foundations.Free();
	}

	AFortAthenaMapInfo* MapInfo = GameState->MapInfo;

	if (MapInfo == NULL)
		return false;

	if (!GameModeAthena->bWorldIsReady)
	{
		UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultDuo.Playlist_DefaultDuo");

		GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
		GameState->CurrentPlaylistInfo.MarkArrayDirty();
		GameState->OnRep_CurrentPlaylistInfo();

		GameState->CurrentPlaylistId = Playlist->PlaylistId;
		GameState->OnRep_CurrentPlaylistId();

		GameModeAthena->CurrentPlaylistId = Playlist->PlaylistId;
		GameModeAthena->CurrentPlaylistName = Playlist->PlaylistName;

		GameModeAthena->AISettings = Playlist->AISettings;
		GameModeAthena->AISettings->AIServices[1] = UAthenaAIServicePlayerBots::StaticClass();

		GameState->AirCraftBehavior = Playlist->AirCraftBehavior;

		GameModeAthena->WarmupRequiredPlayerCount = 1;

		GameState->Client_RefreshEventCalendar();

		for (TSoftObjectPtr<UWorld>& AdditionalLevel : Playlist->AdditionalLevels)
		{
			FAdditionalLevelStreamed PlaylistLevel = FAdditionalLevelStreamed();

			PlaylistLevel.LevelName = AdditionalLevel.ObjectID.AssetPathName;
			PlaylistLevel.bIsServerOnly = false;

			bool bOutSuccess;
			ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), AdditionalLevel, FVector(), FRotator(), &bOutSuccess, FString(), NULL);

			GameState->AdditionalPlaylistLevelsStreamed.Add(PlaylistLevel);
		}

		for (TSoftObjectPtr<UWorld>& AdditionalLevelServerOnly : Playlist->AdditionalLevelsServerOnly)
		{
			FAdditionalLevelStreamed PlaylistLevel = FAdditionalLevelStreamed();

			PlaylistLevel.LevelName = AdditionalLevelServerOnly.ObjectID.AssetPathName;
			PlaylistLevel.bIsServerOnly = true;

			bool bOutSuccess;
			ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), AdditionalLevelServerOnly, FVector(), FRotator(), &bOutSuccess, FString(), NULL);

			GameState->AdditionalPlaylistLevelsStreamed.Add(PlaylistLevel);
		}

		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;
		std::cout << "0x" << std::hex << InSDKUtils::GetImageBase() << std::endl;

		GameState->OnRep_AdditionalPlaylistLevelsStreamed();
		GameState->OnFinishedStreamingAdditionalPlaylistLevel();

		if (GameModeAthena->AIDirector == NULL && Playlist->AISettings && Playlist->AISettings->bAllowAIDirector)
		{
			GameModeAthena->AIDirector = SpawnActor<AFortAIDirector>(AFortAIDirector::StaticClass(), FVector(), FRotator(), GameModeAthena);
			GameModeAthena->AIDirector->Activate();
		}

		if (GameModeAthena->AIGoalManager == NULL && Playlist->AISettings && Playlist->AISettings->bAllowAIGoalManager)
		{
			GameModeAthena->AIGoalManager = SpawnActor<AFortAIGoalManager>(AFortAIGoalManager::StaticClass(), FVector(), FRotator(), GameModeAthena);
		}

		if (GameModeAthena->SpawningPolicyManager == NULL)
		{
			GameModeAthena->SpawningPolicyManager = SpawnActor<AFortAthenaSpawningPolicyManager>(AFortAthenaSpawningPolicyManager::StaticClass(), FVector(), FRotator(), GameModeAthena);
			GameModeAthena->SpawningPolicyManager->GameModeAthena = GameModeAthena;
			GameModeAthena->SpawningPolicyManager->GameStateAthena = GameState;
		}

		static UAthenaBattleBusItemDefinition* BBID_WinterBus = StaticLoadObject<UAthenaBattleBusItemDefinition>(
			TEXT("/Game/Athena/Items/Cosmetics/BattleBuses/BBID_WinterBus.BBID_WinterBus"));

		if (BBID_WinterBus)
		{
			if (UFortAssetManager* AssetManager = Cast<UFortAssetManager>(UFortEngine::GetEngine()->AssetManager))
			{
				AssetManager->GameDataCosmetics->DefaultBattleBusSkin = BBID_WinterBus;
			}
		}


		static UClass* SupplyDropClass = StaticLoadObject<UClass>(TEXT("/Game/Athena/SupplyDrops/AthenaSupplyDrop_Holiday.AthenaSupplyDrop_Holiday_C"));

		if (SupplyDropClass)
		{
			MapInfo->SupplyDropInfoList[0]->SupplyDropClass = SupplyDropClass;
		}


		FURL ListenURL = FURL();

		ListenURL.Port = 7777;

		if (World::Listen(UWorld::GetWorld(), ListenURL))
			SetConsoleTitleA("Forsaken | Listening");

		GameModeAthena->bWorldIsReady = true;
	}

	return GameModeAthena->AlivePlayers.Num() > 0;
}

void GameMode::Setup()
{
	UKismetDetourLibrary::InitializeDetour<AFortGameModeAthena, VFT>(0x113, ReadyToStartMatch);
}

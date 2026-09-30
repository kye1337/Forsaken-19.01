#include "Athena/FortGameModeAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "FortniteGame/Source/FortniteAI/Public/AI/FortAthenaAIBotController.h"

void FortGameModeAthena::StartNewSafeZonePhase(AFortGameModeAthena* GameMode, int32 NewSafeZonePhase) // todo rewrite
{
	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

	if (GameState == NULL || GameState->MapInfo == NULL)
		return Original::StartNewSafeZonePhase(GameMode, NewSafeZonePhase);

	FFortSafeZoneDefinition* SafeZoneDefinition = &GameState->MapInfo->SafeZoneDefinition;

	TArray<float>& CachedWaitTime = *(TArray<float>*)(__int64(SafeZoneDefinition) + 0x238);
	TArray<float>& CachedShrinkTime = *(TArray<float>*)(__int64(SafeZoneDefinition) + 0x248);

	static bool bInitializeSafeZones = false;

	if (!bInitializeSafeZones)
	{
		bInitializeSafeZones = true;

		if (UCurveTable* GameData = GetGameData())
		{
			static const FName NAME_ShrinkTime = UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.ShrinkTime");
			static const FName NAME_WaitTime = UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.WaitTime");

			for (int32 i = 0; i < CachedShrinkTime.Num(); i++)
			{
				UDataTableFunctionLibrary::EvaluateCurveTableRow(GameData, NAME_ShrinkTime, i, nullptr, &CachedShrinkTime[i], FString());
			}

			for (int32 i = 0; i < CachedWaitTime.Num(); i++)
			{
				UDataTableFunctionLibrary::EvaluateCurveTableRow(GameData, NAME_WaitTime, i, nullptr, &CachedWaitTime[i], FString());
			}
		}
	}

	Original::StartNewSafeZonePhase(GameMode, NewSafeZonePhase);

	GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + CachedShrinkTime[GameMode->SafeZonePhase];
	GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime + CachedWaitTime[GameMode->SafeZonePhase];
}

void FortGameModeAthena::PlaceBotOnTeam(AFortGameModeAthena* GameMode, AFortPlayerStateAthena* FPSA, AFortTeamInfoAthena* TeamToBePutOn, uint8 BotSquadId)
{
	Original::PlaceBotOnTeam(GameMode, FPSA, TeamToBePutOn, BotSquadId);

    static __int64(*TPlayerListFindOrAdd)(__int64, int) = decltype(TPlayerListFindOrAdd)(InSDKUtils::GetImageBase() + 0x11BA2AC);
    static __int64(*TPlayerListAdd)(__int64, __int64) = decltype(TPlayerListAdd)(InSDKUtils::GetImageBase() + 0x11BA08C);

    __int64 TeamIdxPlayerList = TPlayerListFindOrAdd((int64(GameMode->GameState) + 0x1668), FPSA->TeamIndex);
	TPlayerListAdd(TeamIdxPlayerList, __int64(FPSA));

    __int64 SquadIdPlayerList = TPlayerListFindOrAdd((int64(GameMode->GameState) + 0x16a0), FPSA->SquadId);
	TPlayerListAdd(SquadIdPlayerList, __int64(FPSA));
}

void FortGameModeAthena::StartAircraftPhase(AFortGameModeAthena* GameMode, bool bGoStraightToSafeZone)
{
	Original::StartAircraftPhase(GameMode, bGoStraightToSafeZone);

	for (AFortPlayerControllerAthena* AlivePlayer : GameMode->AlivePlayers)
	{
		FortPlayerControllerAthena::RemoveAllInventoryItems(AlivePlayer);
	}
}

void FortGameModeAthena::Setup()
{
	UKismetDetourLibrary::InitializeDetour<AFortGameModeAthena, MinHook>(0x6078e88, StartNewSafeZonePhase, &Original::StartNewSafeZonePhase);
    UKismetDetourLibrary::InitializeDetour<AFortGameModeAthena, MinHook>(0x606E478, PlaceBotOnTeam, &Original::PlaceBotOnTeam);
    UKismetDetourLibrary::InitializeDetour<AFortGameModeAthena, MinHook>(0x6076B88, StartAircraftPhase, &Original::StartAircraftPhase);
}

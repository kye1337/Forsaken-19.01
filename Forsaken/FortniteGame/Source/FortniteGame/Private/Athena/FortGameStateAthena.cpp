#include "Athena/FortGameStateAthena.h"
#include "AI/FortAthenaAIBotController.h"

void FortGameStateAthena::PlacePlayersInAircraft(AFortGameStateAthena* GameState)
{
	Original::PlacePlayersInAircraft(GameState);

	UAthenaAIServicePlayerBots* AIServicePlayerBots = UAthenaAIBlueprintLibrary::GetAIServicePlayerBots(UWorld::GetWorld());

	if (AIServicePlayerBots == NULL)
		return;

	AFortAthenaAircraft* Aircraft = GameState->GetAircraft(0);

	if (Aircraft == NULL)
		return;

	for (const FFortServerBotInfo& PlayerBot : AIServicePlayerBots->PlayerBots)
	{
		AFortAthenaAIBotController* BotController = PlayerBot.BotController;

		if (BotController == NULL || !BotController->bIsAnAthenaGameParticipant)
			continue;

		BotController->EnterAircraft(Aircraft);

		FortAthenaAIBotController::RemoveAllInventoryItems(BotController);
	}
}

void FortGameStateAthena::Setup()
{
	UKismetDetourLibrary::InitializeDetour<AFortGameStateAthena, MinHook>(0x143FBF8, PlacePlayersInAircraft, &Original::PlacePlayersInAircraft);
}

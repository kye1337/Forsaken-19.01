#include "GameFramework/GameModeBase.h"

APawn* GameModeBase::SpawnDefaultPawnFor(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot)
{
	return GameMode->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());
}

void GameModeBase::HandleStartingNewPlayer(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	Original::HandleStartingNewPlayer(GameMode, NewPlayer);

	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameMode->GameState);

	if (GameState == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(NewPlayer->PlayerState);

	if (PlayerState == NULL)
		return;

	PlayerState->SquadId = PlayerState->TeamIndex - 3;
	PlayerState->OnRep_SquadId();

	PlayerState->WorldPlayerId = PlayerState->PlayerId;

	FGameMemberInfo Member{};

	Member.ReplicationID = -1;
	Member.ReplicationKey = -1;
	Member.MostRecentArrayReplicationKey = -1;

	Member.SquadId = PlayerState->SquadId;
	Member.TeamIndex = PlayerState->TeamIndex;
	Member.MemberUniqueId = PlayerState->UniqueId;

	GameState->GameMemberInfoArray.Members.Add(Member);
	GameState->GameMemberInfoArray.MarkArrayDirty();

	GameState->NotifyGameMemberAdded(PlayerState->SquadId, PlayerState->TeamIndex, &PlayerState->UniqueId);

	if (UNetDriver* NetDriver = UWorld::GetWorld()->NetDriver)
	{
		if (UFortReplicationGraph* ReplicationGraph = Cast<UFortReplicationGraph>(NetDriver->ReplicationDriver))
		{
			//ReplicationGraph->OnPlayerStateSquadAssign(PlayerState, PlayerState->SquadId);
		}
	}
}

void GameModeBase::Setup()
{
	UKismetDetourLibrary::InitializeDetour<AFortGameModeAthena, VFT>(0xd8, SpawnDefaultPawnFor);
	UKismetDetourLibrary::InitializeDetour<AFortGameModeAthena, VFT>(0x6f0 / 8, HandleStartingNewPlayer, &Original::HandleStartingNewPlayer);
}

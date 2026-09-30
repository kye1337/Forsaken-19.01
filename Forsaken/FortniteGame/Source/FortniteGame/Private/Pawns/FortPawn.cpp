#include "Pawns/FortPawn.h"

void FortPawn::EmoteStopped(UObject* Context, FFrame& Stack, void* Ret)
{
	UPROPERTY(UFortItemDefinition*, MontageItemDef);

	Original::EmoteStopped(Context, Stack, Ret);

	AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Context);

	if (Pawn == NULL)
		return;

	if (AFortAthenaAIBotController* BotController = Cast<AFortAthenaAIBotController>(Pawn->Controller))
		return;

	if (Pawn->bIsPlayingEmote)
		return;

	const UFortItemDefinition* LastReplicatedEmote = Pawn->LastReplicatedEmoteExecuted;

	Pawn->LastReplicatedEmoteExecuted = NULL;
	Pawn->OnRep_LastReplicatedEmoteExecuted(LastReplicatedEmote);
}

void FortPawn::MovingEmoteStopped(UObject* Context, FFrame& Stack, void* Ret)
{
	std::cout << __FUNCTION__ << std::endl;

	Original::MovingEmoteStopped(Context, Stack, Ret);

	AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Context);

	if (Pawn == NULL)
		return;

	if (AFortAthenaAIBotController* BotController = Cast<AFortAthenaAIBotController>(Pawn->Controller))
		return;

	Pawn->bMovingEmote = false;
	Pawn->bMovingEmoteSkipLandingFX = false;
	Pawn->bMovingEmoteForwardOnly = false;
	Pawn->bMovingEmoteFollowingOnly = false;
}

void FortPawn::Setup()
{
	UFunction* execEmoteStopped = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortPawn.EmoteStopped"));
	UFunction* execMovingEmoteStopped = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortPawn.MovingEmoteStopped"));

	//UKismetDetourLibrary::InitializeDetour<AFortPawn, Exec>(execEmoteStopped, EmoteStopped, &Original::EmoteStopped);
	UKismetDetourLibrary::InitializeDetour<AFortPawn, Exec>(execMovingEmoteStopped, MovingEmoteStopped, &Original::MovingEmoteStopped);
}

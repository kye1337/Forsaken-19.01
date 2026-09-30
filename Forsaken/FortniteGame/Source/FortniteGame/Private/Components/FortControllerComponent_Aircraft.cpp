#include "Components/FortControllerComponent_Aircraft.h"

void FortControllerComponent_Aircraft::ServerAttemptAircraftJump(UFortControllerComponent_Aircraft* ControllerComponent, const FRotator& ClientRotation)
{
	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

	if (GameMode == NULL)
		return;

	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(ControllerComponent->GetOwner());

	if (PlayerController == NULL)
		return;

	GameMode->RestartPlayer(PlayerController);

	PlayerController->ClientSetRotation(ClientRotation, true);
}

void FortControllerComponent_Aircraft::Setup()
{
	UKismetDetourLibrary::InitializeDetour<UFortControllerComponent_Aircraft, VFT>(0x9E, ServerAttemptAircraftJump);
}

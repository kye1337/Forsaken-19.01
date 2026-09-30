#include "Missions/FortMissionLibrary.h"

bool FortMissionLibrary::TeleportPlayerPawn(UObject* Context, FFrame& Stack, bool* Ret)
{
	UPROPERTY(UObject*, WorldContextObject);
	UPROPERTY(AFortPlayerPawn*, PlayerPawn);
	UPROPERTY(FVector, DestLocation);
	UPROPERTY(FRotator, DestRotation);
	UPROPERTY(bool, bIgnoreCollision);
	UPROPERTY(bool, bIgnoreSupplementalKillVolumeSweep);

	Original::TeleportPlayerPawn(Context, Stack, Ret);

	if (PlayerPawn == NULL)
		return *Ret = false;

	PlayerPawn->K2_TeleportTo(DestLocation, DestRotation);

	return *Ret = true;
}

void FortMissionLibrary::Setup()
{
	UFunction* execTeleportPlayerPawn = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortMissionLibrary.TeleportPlayerPawn"));

	UKismetDetourLibrary::InitializeDetour<UFortKismetLibrary, Exec>(execTeleportPlayerPawn, TeleportPlayerPawn, &Original::TeleportPlayerPawn);
}

#include "Building/BuildingSMActor.h"

void BuildingSMActor::AttemptSpawnResources(ABuildingSMActor* BuildingActor, AFortPlayerPawn* InstigatorPawn, float ActualDamageDealt, bool bJustHitWeakspot)
{
	AFortPlayerController* PlayerController = Cast<AFortPlayerController>(InstigatorPawn->Controller);

	if (PlayerController == NULL)
		return;

	AFortInventory* WorldInventory = PlayerController->WorldInventory;

	if (WorldInventory == NULL)
		return;

	UFortResourceItemDefinition* ResourceItemDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingActor->ResourceType);

	if (ResourceItemDefinition == NULL)
		return;

	float Average;
	UFortKismetLibrary::EvaluateCurveTableRow(BuildingActor->BuildingResourceAmountOverride, 0.f, &Average, FString());

	int32 ResourceAmount = UKismetMathLibrary::Round(Average / (BuildingActor->GetMaxHealth() / ActualDamageDealt)); // this is definetly scuffed

	if (ResourceAmount <= 0)
		return;

	PlayerController->ClientReportDamagedResourceBuilding(BuildingActor, BuildingActor->ResourceType, ResourceAmount, BuildingActor->bDestroyed, bJustHitWeakspot);

	FortInventory::AddItem(WorldInventory, ResourceItemDefinition, ResourceAmount, 0, 0);
}

void BuildingSMActor::Setup()
{
	for (int32 i = 0; i < UObject::GObjects->Num(); ++i)
	{
		UObject* Object = UObject::GObjects->GetByIndex(i);

		if (Object == NULL)
			continue;

		if (Object->IsA(ABuildingSMActor::StaticClass()))
		{
			UKismetDetourLibrary::VirtualSwap(Object->VTable, 0x17c, AttemptSpawnResources);
		}
	}
}

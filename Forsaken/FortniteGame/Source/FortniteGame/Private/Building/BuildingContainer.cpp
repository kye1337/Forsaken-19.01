#include "Building/BuildingContainer.h"

void BuildingContainer::OnAuthorityRandomUpgradeApplied(ABuildingContainer* Container, const FName* UpgradeTierGroup)
{
	Original::OnAuthorityRandomUpgradeApplied(Container, UpgradeTierGroup);

	int32 ChosenRandomUpgrade = Container->ChosenRandomUpgrade;

	if (ChosenRandomUpgrade < 0 || ChosenRandomUpgrade >= Container->AlternateMeshes.Num())
		return;

	FTierMeshSets& AlternateMesh = Container->AlternateMeshes[ChosenRandomUpgrade];

	Container->ReplicatedLootTier = AlternateMesh.Tier;
	Container->OnRep_LootTier();
}

void BuildingContainer::Setup()
{
	UKismetDetourLibrary::InitializeDetour<ABuildingContainer, MinHook>(0x69B8300, OnAuthorityRandomUpgradeApplied, &Original::OnAuthorityRandomUpgradeApplied);
}

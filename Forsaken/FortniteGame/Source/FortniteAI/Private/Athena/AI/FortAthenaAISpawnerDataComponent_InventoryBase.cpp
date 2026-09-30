#include "FortniteGame/Source/FortniteAI/Public/Athena/AI/FortAthenaAISpawnerDataComponent_InventoryBase.h"

void FortAthenaAISpawnerDataComponent_InventoryBase::GetInventoryItems(UFortAthenaAISpawnerDataComponent_InventoryBase* InventoryBase, TArray<FItemAndCount>* OutList)
{
	UFortAthenaAISpawnerDataComponent_AIBotInventory* AIBotInventory = Cast<UFortAthenaAISpawnerDataComponent_AIBotInventory>(InventoryBase);

	for (const FItemAndCount& Item : AIBotInventory->Items)
	{
		if (Item.Item == NULL)
			continue;

		OutList->Add(Item);
	}
}

void FortAthenaAISpawnerDataComponent_InventoryBase::Setup()
{
	UKismetDetourLibrary::InitializeDetour<UFortAthenaAISpawnerDataComponent_InventoryBase, MinHook>(0x6A119AC, GetInventoryItems);
}

#include "Items/FortInventoryOwnerInterface.h"
#include "FortniteGame/Source/FortniteAI/Public/AI/FortAthenaAIBotController.h"
#include "FortniteGame/Source/FortniteAI/Public/AI/Inventory/FortBotInventoryInfo.h"

bool FortInventoryOwnerInterface::CreateInventory(IFortInventoryOwnerInterface* InventoryOwner)
{
	AFortAthenaAIBotController* BotController = (AFortAthenaAIBotController*)(__int64(InventoryOwner) - 0x350);

	if (BotController == NULL)
		return false;

	if (BotController->Inventory == NULL)
	{
		BotController->Inventory = SpawnActor<AFortInventory>(AFortInventory::StaticClass(), FVector(), FRotator(), BotController);
	}

	return BotController->Inventory != NULL;
}

void FortInventoryOwnerInterface::InitializeInventory(IFortInventoryOwnerInterface* InventoryOwner, const TArray<FItemAndCount>* ItemList)
{
	AFortAthenaAIBotController* BotController = (AFortAthenaAIBotController*)(__int64(InventoryOwner) - 0x350);

	if (BotController == NULL)
		return;

	for (const FItemAndCount& Item : *ItemList)
	{
		UFortWorldItem* NewItem = FortAthenaAIBotController::CreateItem(BotController, Item.Item, Item.Count, 0, 0);
	}
}

bool FortInventoryOwnerInterface::RemoveInventoryItem(IFortInventoryOwnerInterface* InventoryOwner, const FGuid& ItemGuid, int32 Count, bool bForceRemoval, bool bForcePersistWhenEmpty)
{
	AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)(__int64(InventoryOwner) - 0x708);

	if (PlayerController == NULL)
		return false;

	AFortInventory* WorldInventory = PlayerController->WorldInventory;

	if (WorldInventory == NULL)
		return false;

	FortInventory::RemoveItem(WorldInventory, ItemGuid, Count, bForceRemoval);

	return true;
}

void FortInventoryOwnerInterface::Setup()
{
	UObject* FortPlayerControllerInventoryOwnerInterfaceObject = (UObject*)(__int64(AFortPlayerControllerAthena::GetDefaultObj()) + 0x708);
	UObject* FortBotControllerInventoryOwnerInterfaceObject = (UObject*)(__int64(AFortAthenaAIBotController::GetDefaultObj()) + 0x350);

	UKismetDetourLibrary::VirtualSwap(FortPlayerControllerInventoryOwnerInterfaceObject->VTable, 0x2f, RemoveInventoryItem);
	UKismetDetourLibrary::VirtualSwap(FortBotControllerInventoryOwnerInterfaceObject->VTable, 0x3, CreateInventory);
	UKismetDetourLibrary::VirtualSwap(FortBotControllerInventoryOwnerInterfaceObject->VTable, 0x4, InitializeInventory);
}

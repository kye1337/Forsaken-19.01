#include "FortniteGame/Source/FortniteAI/Public/AI/Inventory/FortBotInventoryInfo.h"

void FortBotInventoryInfo::SetItem(FFortBotInventoryInfo* SlotItem, UFortWorldItem* NewItem)
{
	SlotItem->FortItem = NewItem;

	if (NewItem)
	{
		UFortItemDefinition* ItemDefinition = NewItem->GetItemDefinitionBP();

		SlotItem->ItemDefinition = ItemDefinition;

		if (ItemDefinition)
		{
			SlotItem->ItemType = ItemDefinition->GetItemType();
			SlotItem->WeaponType = AFortWeapon::FindWeaponTypeForStats(ItemDefinition);
			return;
		}
	}
	else
	{
		SlotItem->ItemDefinition = NULL;
	}

	SlotItem->ItemType = EFortItemType::EFortItemType_MAX;
	SlotItem->WeaponType = EFortWeaponType::MAX;
}

#pragma once

#include "CoreGlobals.h"

class FortPlayerController
{
private:
	static void ServerExecuteInventoryItem(AFortPlayerController* PlayerController, const FGuid& ItemGuid);
	static void ServerCreateBuildingActor(AFortPlayerController* PlayerController, const FCreateBuildingActorData& CreateBuildingData);
	static void ServerBeginEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit);
	static void ServerEditBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit, TSubclassOf<ABuildingSMActor> NewBuildingClass, uint8 RotationIterations, bool bMirrored);
	static void ServerEndEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToStopEditing);
	static void ServerAttemptInventoryDrop(AFortPlayerController* PlayerController, const FGuid& ItemGuid, int32 Count, bool bTrash);
	static void ServerPlayEmoteItem(AFortPlayerController* PlayerController, UFortMontageItemDefinitionBase* EmoteAsset, float EmoteRandomNumber);
	static void ServerRepairBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToRepair);
	static void ServerCheat(AFortPlayerController* PlayerController, const FString& Msg);
public:
	static void Setup();
};

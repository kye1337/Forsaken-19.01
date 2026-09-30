#include "CoreGlobals.h"

#include "Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/Abilities/GameplayAbility.h"

#include "Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemComponent.h"

#include "Engine/Restricted/NotForLicensees/Plugins/OnlineGameplayFramework/Source/McpProfileSys/Public/McpProfileGroup.h"

#include "Engine/NetDriver.h"
#include "Engine/World.h"

#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"

#include "Engine/Source/Runtime/NavigationSystem/Public/NavigationSystem.h"

#include "FortniteGame/Source/FortniteAI/Public/AI/FortAthenaAIBotController.h"

#include "Athena/AI/Systems/AthenaAIServicePlayerBots.h"

#include "FortniteGame/Source/FortniteAI/Public/Athena/AI/FortAthenaAISpawnerDataComponent_AIBotCosmeticBase.h"
#include "FortniteGame/Source/FortniteAI/Public/Athena/AI/FortAthenaAISpawnerDataComponent_InventoryBase.h"

#include "Athena/Markers/AthenaMarkerComponent.h"

#include "Athena/FortAthenaVehicleSpawner.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"

#include "Building/BuildingContainer.h"
#include "Building/BuildingSMActor.h"

#include "Components/FortControllerComponent_Aircraft.h"

#include "Items/FortInventory.h"
#include "Items/FortInventoryOwnerInterface.h"
#include "Items/FortLootPackage.h"
#include "Items/FortWorldItem.h"

#include "Missions/FortMissionLibrary.h"

#include "Pawns/FortPawn.h"
#include "Pawns/FortPhysicsPawn.h"
#include "Pawns/FortPlayerPawn.h"

#include "Player/FortPlayerController.h"

#include "FortKismetLibrary.h"

#include "FortniteGame/Source/FortniteAI/Public/AI/Inventory/FortBotInventoryInfo.h"

#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationLibrary.h"
#include <algorithm>

static void UnregisterChestLoot(UAthenaAIServiceLoot* AIServiceLoot, ABuildingContainer* LootActor)
{
    static float RemoveInvalidChestFromOctreeProbability = AIServiceLoot->RemoveInvalidChestFromOctreeProbability.GetValueAtLevel(NULL);

    bool bRemoveInvalidChest = (rand() * 0.000030518509) <= RemoveInvalidChestFromOctreeProbability;

    if (bRemoveInvalidChest)
        AIServiceLoot->LootOctree->RemoveLootActor(LootActor);
}

static inline void SpawnLoot(ABuildingContainer* BuildingContainer)
{
    ++BuildingContainer->bAlreadySearched;
    ++BuildingContainer->SearchBounceData.SearchAnimationCount;

    BuildingContainer->OnRep_bAlreadySearched();
    BuildingContainer->BounceContainer();

    TArray<FFortItemEntry> LootToDrop;

    if (UFortLootPackage::PickLootDrops(&LootToDrop, BuildingContainer->SearchLootTierGroup, Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState)->WorldLevel, BuildingContainer->ReplicatedLootTier))
    {
        for (const FFortItemEntry& Entry : LootToDrop)
        {
            FVector LocationToSpawnLoot = BuildingContainer->K2_GetActorLocation() + (BuildingContainer->GetActorForwardVector() *
                BuildingContainer->LootSpawnLocation_Athena.X) + (BuildingContainer->GetActorRightVector() *
                    BuildingContainer->LootSpawnLocation_Athena.Y) + (BuildingContainer->GetActorUpVector() *
                        BuildingContainer->LootSpawnLocation_Athena.Z);

            FortPickupCreationData CreationData = FortPickupCreationData(UWorld::GetWorld(), &Entry, LocationToSpawnLoot, FRotator(), nullptr, nullptr, BuildingContainer, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);

            AFortPickup* Pickup = AFortPickup::CreateFromData(&CreationData);

            if (Pickup == NULL)
                continue;

            int32 Step = (int32)std::clamp((float)rand() * 0.0003357036f, 0.f, 10.f);

            UFortKismetLibrary::TossPickupFromContainer(UWorld::GetWorld(), BuildingContainer, Pickup, 10.0f, Step, BuildingContainer->LootTossConeHalfAngle_Athena,
                BuildingContainer->LootTossDirection_Athena, BuildingContainer->LootTossSpeed_Athena, BuildingContainer->bForceHidePickupMinimapIndicator);

            Pickup->bTossedFromContainer = true;
            Pickup->OnRep_TossedFromContainer();
        }
    }

    LootToDrop.Free();

    UAthenaAIServiceLoot* AIServiceLoot = UAthenaAIBlueprintLibrary::GetAIServiceLoot(UWorld::GetWorld());

    if (AIServiceLoot != NULL)
    {
        UnregisterChestLoot(AIServiceLoot, BuildingContainer);
    }
}

static inline void (*_PostUpdate)(ABuildingSMActor* BuildingSMActor, EFortBuildingPersistentState PersistentState, void* ReservedRandomValue);
static inline void PostUpdate(ABuildingSMActor* BuildingSMActor, EFortBuildingPersistentState PersistentState, void* ReservedRandomValue)
{
    if (ABuildingContainer* BuildingContainer = Cast<ABuildingContainer>(BuildingSMActor))
    {
        for (const TPair<FName, FName>& RedirectAthenaLootTierGroup : Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode)->RedirectAthenaLootTierGroups)
        {
            if (RedirectAthenaLootTierGroup.Key() != BuildingContainer->SearchLootTierGroup)
                continue;

            BuildingContainer->SearchLootTierGroup = RedirectAthenaLootTierGroup.Value();
            BuildingContainer->DestructionLootTierGroup = RedirectAthenaLootTierGroup.Value();
        }

        if (BuildingContainer->bStartAlreadySearched_Athena)
        {
            SpawnLoot(BuildingContainer);
        }
    }

    _PostUpdate(BuildingSMActor, PersistentState, ReservedRandomValue);
}

void ServerHandlePickupInfo(AFortPlayerPawn* Pawn, AFortPickup* PickUp, const FFortPickupRequestInfo& Params_0)
{
    PickUp->SetPickupTarget(Pawn, Params_0.FlyTime / Pawn->PickupSpeedMultiplier, Params_0.Direction);
}

static void (*_GivePickupTo)(AFortPickup* Pickup, IFortInventoryOwnerInterface* InventoryOwner, bool DestroyAfterPickup);
void GivePickupTo(AFortPickup* Pickup, IFortInventoryOwnerInterface* InventoryOwner, bool DestroyAfterPickup)
{
    _GivePickupTo(Pickup, InventoryOwner, DestroyAfterPickup);

    AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(Pickup->PickupLocationData.PickupTarget);

    if (Pawn == NULL)
        return;

    if (AFortPlayerController* PlayerController = Cast<AFortPlayerController>(Pawn->Controller))
    {
        AFortInventory* Inventory = PlayerController->WorldInventory;

        if (Inventory == NULL)
            return;

        FortInventory::AddItem(Inventory, Pickup->PrimaryPickupItemEntry.ItemDefinition, Pickup->PrimaryPickupItemEntry.Count, Pickup->PrimaryPickupItemEntry.LoadedAmmo, Pickup->PrimaryPickupItemEntry.PhantomReserveAmmo);
    }
    else if (AFortAthenaAIBotController* BotController = Cast<AFortAthenaAIBotController>(Pawn->Controller))
    {
        FortAthenaAIBotController::CreateItem(BotController, Pickup->PrimaryPickupItemEntry.ItemDefinition, Pickup->PrimaryPickupItemEntry.Count, Pickup->PrimaryPickupItemEntry.LoadedAmmo, Pickup->PrimaryPickupItemEntry.PhantomReserveAmmo);
    }
}

void AssessLootInteractionInfo(UFortAthenaBTService_PickUpLoot* BTService_PickUpLoot, bool* bOutSkipInteractionExecution, UBlackboardComponent* BBComp, AFortAthenaAIBotController* BotController, AActor* LootActor)
{
    if (AFortPickup* Pickup = Cast<AFortPickup>(LootActor))
    {
        BotController->PlayerBotPawn->ServerHandlePickup(Pickup, Pickup->DefaultFlyTime, Pickup->PickupLocationData.StartDirection, true);
    }
    else if (ABuildingContainer* Container = Cast<ABuildingContainer>(LootActor))
    {
        static void (*SpawnLoot)(ABuildingContainer*) = decltype(SpawnLoot)(InSDKUtils::GetImageBase() + 0x623591C);
        SpawnLoot(Container);
    }

    *bOutSkipInteractionExecution = true;
}

void SpawnDecoInternal(AFortDecoTool* DecoTool, FVector& Location, FRotator& Rotation, ABuildingSMActor* AttachedActor, EBuildingAttachmentType InBuildingAttachmentType)
{
    AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(DecoTool->GetOwner());

    if (Pawn == NULL)
        return;

    AFortPlayerController* PlayerController = Cast<AFortPlayerController>(Pawn->Controller);

    if (PlayerController == NULL)
        return;

    AFortInventory* WorldInventory = PlayerController->WorldInventory;

    if (WorldInventory == NULL)
        return;

    UFortTrapItemDefinition* TrapItemDefinition = Cast<UFortTrapItemDefinition>(DecoTool->ItemDefinition);

    if (TrapItemDefinition == NULL)
        return;

    UFortTrapItemDefinition* NewTrapItemDefinition = TrapItemDefinition;

    if (AFortDecoTool_ContextTrap* ContextDecoTool = Cast<AFortDecoTool_ContextTrap>(DecoTool))
    {
        switch (InBuildingAttachmentType)
        {
        case EBuildingAttachmentType::ATTACH_Floor:
            NewTrapItemDefinition = ContextDecoTool->ContextTrapItemDefinition->FloorTrap;
            break;
        case EBuildingAttachmentType::ATTACH_Wall:
            NewTrapItemDefinition = ContextDecoTool->ContextTrapItemDefinition->WallTrap;
            break;
        case EBuildingAttachmentType::ATTACH_Ceiling:
            NewTrapItemDefinition = ContextDecoTool->ContextTrapItemDefinition->CeilingTrap;
            break;
        case EBuildingAttachmentType::ATTACH_StairsBothSides:
            NewTrapItemDefinition = ContextDecoTool->ContextTrapItemDefinition->StairTrap;
            break;
        }
    }

    if (NewTrapItemDefinition == NULL)
        return;

    UClass* BlueprintClass = Cast<UClass>(NewTrapItemDefinition->BlueprintClass.LoadSynchronous());

    if (BlueprintClass == NULL)
        return;

    DecoTool->SpawnDeco(BlueprintClass, Location, Rotation, AttachedActor, InBuildingAttachmentType, 0);

    if (TrapItemDefinition->bConsumeWhenPlaced)
    {
        FFortItemEntry* ItemEntry = FortInventory::GetItemEntry(WorldInventory, TrapItemDefinition);

        if (ItemEntry == NULL)
            return;

        FortInventory::RemoveItem(WorldInventory, ItemEntry->ItemGuid, 1);
    }
}

uint8 PickTeam(AFortGameModeAthena* GameMode, uint8 PreferredTeam, AFortPlayerController* ControllerToPickFor)
{
    int32 Ret = 3; // ? this isnt even static bro what are u doing
    return Ret++;
}

void (*_SpawnPickupInWorldWithLootTierAsync)(UObject* Context, FFrame& Stack, void* Ret);
void SpawnPickupInWorldWithLootTierAsync(UObject* Context, FFrame& Stack, void* Ret)
{
    UPROPERTY(FName, LootTierName);
    UPROPERTY(FVector, Position);
    UPROPERTY(int32, OverrideMaxStackCount);
    UPROPERTY(bool, bToss);
    UPROPERTY(EFortPickupSourceTypeFlag, SourceType);
    UPROPERTY(EFortPickupSpawnSource, Source);

    std::cout << __FUNCTION__ << std::endl;

    AFortAIPawn* Pawn = Cast<AFortAIPawn>(Context);

    TArray<FFortItemEntry> LootToDrop;

    if (UFortLootPackage::PickLootDrops(&LootToDrop, LootTierName, Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState)->WorldLevel, 0))
    {
        for (FFortItemEntry& ItemEntry : LootToDrop)
        {
            FortPickupCreationData CreationData = FortPickupCreationData(UWorld::GetWorld(), &ItemEntry, Position, FRotator(), NULL, NULL, NULL, SourceType, Source);

            AFortPickup* Pickup = AFortPickup::CreateFromData(&CreationData);

            Pickup->TossPickup(Position, Pawn, OverrideMaxStackCount, bToss, true, SourceType, Source);
        }
    }

    _SpawnPickupInWorldWithLootTierAsync(Context, Stack, Ret);
}

static void (*GetPlayerViewPointOriginal)(AFortPlayerControllerAthena* PlayerController, FVector& Location, FRotator& Rotation);
void GetPlayerViewPointHook(AFortPlayerControllerAthena* PlayerController, FVector& Location, FRotator& Rotation)
{
    static FName NAME_Spectating = UKismetStringLibrary::Conv_StringToName(L"Spectating");

    if (PlayerController->StateName.ComparisonIndex == NAME_Spectating.ComparisonIndex)
    {
        Location = PlayerController->LastSpectatorSyncLocation;
        Rotation = PlayerController->LastSpectatorSyncRotation;
    }
    else if (PlayerController->GetViewTarget())
    {
        Location = PlayerController->GetViewTarget()->K2_GetActorLocation();
        Rotation = PlayerController->GetControlRotation();
    }
    else
    {
        return GetPlayerViewPointOriginal(PlayerController, Location, Rotation);
    }
}

void (*_OnUnEquip)(AFortWeapon* Weapon);
void OnUnEquip(AFortWeapon* Weapon)
{
    //static bool (*RemoveItemAbilitySet)(IFortInventoryOwnerInterface* InventoryOwner, UFortAbilitySet* AbilitySet, UFortItem* Item) =
    //    decltype(RemoveItemAbilitySet)(InSDKUtils::GetImageBase() + 0x65244A0);

    //auto Pawn = Cast<AFortPlayerPawnAthena>(Weapon->GetOwner());
    //auto Controller = Cast<AFortPlayerControllerAthena>(Pawn->Controller);

    //auto InventoryOwner = (IFortInventoryOwnerInterface*)(__int64(Controller) + 0x708);

    //for (FFortAbilitySetHandle& Abilityset : Weapon->EquippedAbilitySetHandles)
    //{
    //    if (Abilityset.l == NULL)
    //        continue;
    //}

}

AFortPickup* (*_SpawnPickup)(UObject* Context, FFrame& Stack, AFortPickup** Ret);
AFortPickup* SpawnPickup(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
    UPROPERTY(UFortWorldItemDefinition*, ItemDefinition);
    UPROPERTY(int32, NumberToSpawn);
    UPROPERTY(AFortPawn*, TriggeringPawn);
    UPROPERTY(FVector, Position);
    UPROPERTY(FVector, Direction);

    FFortItemEntry ItemEntry;
    ItemEntry.FortItemEntry(ItemDefinition, NumberToSpawn, 0, FGuid());

    if (UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
    {
        ItemEntry.LoadedAmmo = WeaponItemDefinition->GetClipSize();

        if (WeaponItemDefinition->bUsesPhantomReserveAmmo)
            ItemEntry.PhantomReserveAmmo = (WeaponItemDefinition->GetInitialClips() - 1) * WeaponItemDefinition->GetClipSize();
    }

    FortPickupCreationData CreationData = FortPickupCreationData(UWorld::GetWorld(), &ItemEntry, Position, FRotator(Direction.X, Direction.Y, Direction.Z), NULL, NULL, NULL, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::SupplyDrop);

    AFortPickup* Pickup = AFortPickup::CreateFromData(&CreationData);

    Pickup->TossPickup(Position, NULL, 0, true, true, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::SupplyDrop);

    _SpawnPickup(Context, Stack, Ret);

    return *Ret = Pickup;
}

void (*_AddActorsToIndicatedList)(UObject* Context, FFrame& Stack, void* Ret);
void AddActorsToIndicatedList(UObject* Context, FFrame& Stack, void* Ret)
{
    UPROPERTY(AController*, InstigatingController);
    TARRAY(AActor*, IndicatedActors);
    UPROPERTY(FIndicatedActorData, IndicatedActorData);
    UPROPERTY(bool, bAddAsUnique);
    UPROPERTY(bool, bAllowOwningPlayer);
    UPROPERTY(bool, bReplaceExistingEntry);
    UPROPERTY(bool, bRefreshExistingEntry);

    _AddActorsToIndicatedList(Context, Stack, Ret);

    AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(InstigatingController);

    if (PlayerController == NULL)
        return;

    AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);

    if (Pawn == NULL)
        return;

    AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

    if (PlayerState == NULL)
        return;

    for (AActor* IndicatedActor : IndicatedActors)
    {
        if (!bAllowOwningPlayer && Pawn == IndicatedActor)
            continue;

        float TimeSeconds = UFortKismetLibrary::GetServerWorldTimeSeconds(UWorld::GetWorld());

        FIndicatedActorInfoEntry Entry{};

        Entry.ReplicationID = -1;
        Entry.ReplicationKey = -1;
        Entry.MostRecentArrayReplicationKey = -1;

        Entry.Actor = IndicatedActor;
        Entry.StartTime = TimeSeconds;
        Entry.EndTime = TimeSeconds + 99999;
        Entry.Data = IndicatedActorData;
        Entry.bReplaceExistingWhenAdded = bReplaceExistingEntry;
        Entry.bRefreshExistingWhenAdded = bRefreshExistingEntry;

        PlayerController->IndicatedActorManagementComponent->IndicatedActorList.Entries.Add(Entry);
        PlayerController->IndicatedActorManagementComponent->IndicatedActorList.MarkArrayDirty();

        for (AController* TeamMember : PlayerState->PlayerTeam->TeamMembers)
        {
            AFortPlayerControllerAthena* TeamMemberAthena = Cast<AFortPlayerControllerAthena>(TeamMember);

            if (TeamMemberAthena == NULL || TeamMemberAthena == PlayerController)
                continue;

            TeamMemberAthena->IndicatedActorManagementComponent->IndicatedActorList.Entries.Add(Entry);
            TeamMemberAthena->IndicatedActorManagementComponent->IndicatedActorList.MarkArrayDirty();
        }
    }
}

void (*_RemoveActorFromIndicatedList)(UObject* Context, FFrame& Stack, void* Ret);
void RemoveActorFromIndicatedList(UObject* Context, FFrame& Stack, void* Ret)
{
    UPROPERTY(AController*, InstigatingController);
    UPROPERTY(AActor*, IndicatedActor);
    UPROPERTY(bool, bIncludeSquad);

    _RemoveActorFromIndicatedList(Context, Stack, Ret);

    AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(InstigatingController);

    if (PlayerController == NULL)
        return;

    AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

    if (PlayerState == NULL)
        return;

    for (int32 i = 0; i < PlayerController->IndicatedActorManagementComponent->IndicatedActorList.Entries.Num(); ++i)
    {
        FIndicatedActorInfoEntry& Entry = PlayerController->IndicatedActorManagementComponent->IndicatedActorList.Entries[i];

        if (Entry.Actor == IndicatedActor)
        {
            PlayerController->IndicatedActorManagementComponent->IndicatedActorList.Entries.Remove(i);
            PlayerController->IndicatedActorManagementComponent->IndicatedActorList.MarkArrayDirty();
        }

        if (bIncludeSquad)
        {
            for (AController* TeamMember : PlayerState->PlayerTeam->TeamMembers)
            {
                AFortPlayerControllerAthena* TeamMemberAthena = Cast<AFortPlayerControllerAthena>(TeamMember);

                if (TeamMemberAthena == NULL || TeamMemberAthena == PlayerController)
                    continue;

                if (Entry.Actor == IndicatedActor)
                {
                    TeamMemberAthena->IndicatedActorManagementComponent->IndicatedActorList.Entries.Remove(i);
                    TeamMemberAthena->IndicatedActorManagementComponent->IndicatedActorList.MarkArrayDirty();
                }
            }
        }
    }
}

void (*_ServerAttemptInteract)(UFortControllerComponent_Interaction* Controller, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, ETInteractionType InteractType, UObject* OptionalObjectData, EInteractionBeingAttempted InteractionBeingAttempted, int32 RequestId);
void ServerAttemptInteract(UFortControllerComponent_Interaction* Controller, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, ETInteractionType InteractType, UObject* OptionalObjectData, EInteractionBeingAttempted InteractionBeingAttempted, int32 RequestId)
{
    _ServerAttemptInteract(Controller, ReceivingActor, InteractComponent, InteractType, OptionalObjectData, InteractionBeingAttempted, RequestId);

    if (auto ConvComp = Cast<UFortNonPlayerConversationParticipantComponent>(ReceivingActor->GetComponentByClass(UFortNonPlayerConversationParticipantComponent::StaticClass())))
    {
        std::cout << "start" << std::endl;

        UFortPlayerConversationComponent_C* PlayerComp = (UFortPlayerConversationComponent_C*)Controller->GetOwner()->GetComponentByClass(UFortPlayerConversationComponent_C::StaticClass());
        if (!PlayerComp)
            PlayerComp = (UFortPlayerConversationComponent_C*)((AFortPlayerControllerAthena*)Controller->GetOwner())->Pawn->GetComponentByClass(UFortPlayerConversationComponent_C::StaticClass());

        if (ConvComp && PlayerComp && ConvComp->ConversationsActive == 0)
        {
            ConversationLibrary::StartConversation(ConvComp->ConversationEntryTag, PlayerComp->GetOwner(), ConvComp->InteractorParticipantTag, ReceivingActor, ConvComp->SelfParticipantTag);
        }
    }
}

void ServerNotifyStartLongUse(UFortControllerComponent_Interaction* ControllerComponent, AActor* ReceivingActor)
{
    if (AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(ReceivingActor))
    {
        std::cout << "Pawn" << std::endl;

        if (!Pawn->bImprovedDBNOEnabled)
        {
            std::cout << "No imrpove" << std::endl;
            Pawn->bImprovedDBNOEnabled = true;
            Pawn->OnRep_ImprovedDBNOEnabled();
        }

        int32& DBNORevivingActorsCount = *(int32*)(__int64(Pawn) + 0x433b);

        std::cout << DBNORevivingActorsCount << std::endl;

        DBNORevivingActorsCount++;

        if (Pawn->IsDBNO())
        {
            std::cout << "Increase" << std::endl;

            //Pawn->bImprovedDBNOEnabled
            //int32& DBNORevivingActorsCount = *(int32*)(__int64(Pawn) + 0x433b);
            //DBNORevivingActorsCount++;
        }
    }
}

static inline void (*ProcessEventOriginal)(UObject* Object, UFunction* Function, void* Params);
static inline void ProcessEventHook(UObject* Object, UFunction* Function, void* Params)
{
    auto FunctionName = Function->GetName(); // UKismetSystemLibrary::GetPathName(Function).ToString();
    auto FunctionFullName = Function->GetFullName();
    auto ObjectName = Object->GetName();

    if (!strstr(FunctionName.c_str(), ("EvaluateGraphExposedInputs")) &&
        !strstr(FunctionName.c_str(), ("Tick")) &&
        !strstr(FunctionName.c_str(), ("OnSubmixEnvelope")) &&
        !strstr(FunctionName.c_str(), ("OnSubmixSpectralAnalysis")) &&
        !strstr(FunctionName.c_str(), ("OnMouse")) &&
        !strstr(FunctionName.c_str(), ("Pulse")) &&
        !strstr(FunctionName.c_str(), ("BlueprintUpdateAnimation")) &&
        !strstr(FunctionName.c_str(), ("BlueprintPostEvaluateAnimation")) &&
        !strstr(FunctionName.c_str(), ("BlueprintModifyCamera")) &&
        !strstr(FunctionName.c_str(), ("BlueprintModifyPostProcess")) &&
        !strstr(FunctionName.c_str(), ("Loop Animation Curve")) &&
        !strstr(FunctionName.c_str(), ("UpdateTime")) &&
        !strstr(FunctionName.c_str(), ("GetMutatorByClass")) &&
        !strstr(FunctionName.c_str(), ("UpdatePreviousPositionAndVelocity")) &&
        !strstr(FunctionName.c_str(), ("IsCachedIsProjectileWeapon")) &&
        !strstr(FunctionName.c_str(), ("LockOn")) &&
        !strstr(FunctionName.c_str(), ("GetAbilityTargetingLevel")) &&
        !strstr(FunctionName.c_str(), ("ReadyToEndMatch")) &&
        !strstr(FunctionName.c_str(), ("ReceiveDrawHUD")) &&
        !strstr(FunctionName.c_str(), ("OnUpdateDirectionalLightForTimeOfDay")) &&
        !strstr(FunctionName.c_str(), ("GetSubtitleVisibility")) &&
        !strstr(FunctionName.c_str(), ("GetValue")) &&
        !strstr(FunctionName.c_str(), ("InputAxisKeyEvent")) &&
        !strstr(FunctionName.c_str(), ("ServerTouchActiveTime")) &&
        !strstr(FunctionName.c_str(), ("SM_IceCube_Blueprint_C")) &&
        !strstr(FunctionName.c_str(), ("OnHovered")) &&
        !strstr(FunctionName.c_str(), ("OnCurrentTextStyleChanged")) &&
        !strstr(FunctionName.c_str(), ("OnButtonHovered")) &&
        !strstr(FunctionName.c_str(), ("ExecuteUbergraph_ThreatPostProcessManagerAndParticleBlueprint")) &&
        !strstr(FunctionName.c_str(), "PinkOatmeal") &&
        !strstr(FunctionName.c_str(), "CheckForDancingAtFish") &&
        !strstr(FunctionName.c_str(), ("UpdateCamera")) &&
        !strstr(FunctionName.c_str(), ("GetMutatorContext")) &&
        !strstr(FunctionName.c_str(), ("CanJumpInternal")) &&
        !strstr(FunctionName.c_str(), ("OnDayPhaseChanged")) &&
        !strstr(FunctionName.c_str(), ("Chime")) &&
        !strstr(FunctionName.c_str(), ("ServerMove")) &&
        !strstr(FunctionName.c_str(), ("OnVisibilitySetEvent")) &&
        !strstr(FunctionName.c_str(), "ReceiveHit") &&
        !strstr(FunctionName.c_str(), "ReadyToStartMatch") &&
        !strstr(FunctionName.c_str(), "K2_GetComponentToWorld") &&
        !strstr(FunctionName.c_str(), "ClientAckGoodMove") &&
        !strstr(FunctionName.c_str(), "Prop_WildWest_WoodenWindmill_01") &&
        !strstr(FunctionName.c_str(), "ContrailCheck") &&
        !strstr(FunctionName.c_str(), "B_StockBattleBus_C") &&
        !strstr(FunctionName.c_str(), "Subtitles.Subtitles_C.") &&
        !strstr(FunctionName.c_str(), "/PinkOatmeal/PinkOatmeal_") &&
        !strstr(FunctionName.c_str(), "BP_SpectatorPawn_C") &&
        !strstr(FunctionName.c_str(), "FastSharedReplication") &&
        !strstr(FunctionName.c_str(), "OnCollisionHitEffects") &&
        !strstr(FunctionName.c_str(), "BndEvt__SkeletalMesh") &&
        !strstr(FunctionName.c_str(), ".FortAnimInstance.AnimNotify_") &&
        !strstr(FunctionName.c_str(), "OnBounceAnimationUpdate") &&
        !strstr(FunctionName.c_str(), "ShouldShowSoundIndicator") &&
        !strstr(FunctionName.c_str(), "Primitive_Structure_AmbAudioComponent_C") &&
        !strstr(FunctionName.c_str(), "PlayStoppedIdleRotationAudio") &&
        !strstr(FunctionName.c_str(), "UpdateOverheatCosmetics") &&
        !strstr(FunctionName.c_str(), "StormFadeTimeline__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "BindVolumeEvents") &&
        !strstr(FunctionName.c_str(), "UpdateStateEvent") &&
        !strstr(FunctionName.c_str(), "VISUALS__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "Flash__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "SetCollisionEnabled") &&
        !strstr(FunctionName.c_str(), "SetIntensity") &&
        !strstr(FunctionName.c_str(), "Storm__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "CloudsTimeline__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "SetRenderCustomDepth") &&
        !strstr(FunctionName.c_str(), "K2_UpdateCustomMovement") &&
        !strstr(FunctionName.c_str(), "AthenaHitPointBar_C.Update") &&
        !strstr(FunctionName.c_str(), "ExecuteUbergraph_Farm_WeatherVane_01") &&
        !strstr(FunctionName.c_str(), "HandleOnHUDElementVisibilityChanged") &&
        !strstr(FunctionName.c_str(), "ExecuteUbergraph_Fog_Machine") &&
        !strstr(FunctionName.c_str(), "ReceiveBeginPlay") &&
        !strstr(FunctionName.c_str(), "OnMatchStarted") &&
        !strstr(FunctionName.c_str(), "CustomStateChanged") &&
        !strstr(FunctionName.c_str(), "OnBuildingActorInitialized") &&
        !strstr(FunctionName.c_str(), "OnWorldReady") &&
        !strstr(FunctionName.c_str(), "OnAttachToBuilding") &&
        !strstr(FunctionName.c_str(), "Clown Spinner") &&
        !strstr(FunctionName.c_str(), "K2_GetActorLocation") &&
        !strstr(FunctionName.c_str(), "GetViewTarget") &&
        !strstr(FunctionName.c_str(), "GetAllActorsOfClass") &&
        !strstr(FunctionName.c_str(), "OnUpdateMusic") &&
        !strstr(FunctionName.c_str(), "Check Closest Point") &&
        !strstr(FunctionName.c_str(), "OnSubtitleChanged__DelegateSignature") &&
        !strstr(FunctionName.c_str(), "OnServerBounceCallback") &&
        !strstr(FunctionName.c_str(), "BlueprintGetInteractionTime") &&
        !strstr(FunctionName.c_str(), "OnServerStopCallback") &&
        !strstr(FunctionName.c_str(), "Light Flash Timeline__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "MainFlightPath__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "PlayStartedIdleRotationAudio") &&
        !strstr(FunctionName.c_str(), "BGA_Athena_FlopperSpawn_") &&
        !strstr(FunctionName.c_str(), "CheckShouldDisplayUI") &&
        !strstr(FunctionName.c_str(), "Timeline_0__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "ClientMoveResponsePacked") &&
        !strstr(FunctionName.c_str(), "ExecuteUbergraph_B_Athena_FlopperSpawnWorld_Placement") &&
        !strstr(FunctionName.c_str(), "Countdown__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "OnParachuteTrailUpdated") &&
        !strstr(FunctionName.c_str(), "Moto FadeOut__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "ExecuteUbergraph_Apollo_GasPump_Valet") &&
        !strstr(FunctionName.c_str(), "GetOverrideMeshMaterial") &&
        !strstr(FunctionName.c_str(), "VendWobble__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "WaitForPawn") &&
        !strstr(FunctionName.c_str(), "FragmentMovement__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "TrySetup") &&
        !strstr(FunctionName.c_str(), "Fade Doused Smoke__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "SetPlayerToSkydive") &&
        !strstr(FunctionName.c_str(), "BounceCar__UpdateFunc") &&
        !strstr(FunctionName.c_str(), "BP_CalendarDynamicPOISelect") &&
        !strstr(FunctionName.c_str(), "OnComponentHit_Event_0") &&
        !strstr(FunctionName.c_str(), "HandleSimulatingComponentHit") &&
        !strstr(FunctionName.c_str(), "CBGA_GreenGlop_WithGrav_C") &&
        !strstr(FunctionName.c_str(), "WarmupCountdownEndTimeUpdated") &&
        !strstr(FunctionName.c_str(), "BP_CanInteract") &&
        !strstr(FunctionName.c_str(), "AthenaHitPointBar_C") &&
        !strstr(FunctionName.c_str(), "ServerFireAIDirectorEvent") &&
        !strstr(FunctionName.c_str(), "BlueprintThreadSafeUpdateAnimation") &&
        !strstr(FunctionName.c_str(), "On Amb Zap Spawn") &&
        !strstr(FunctionName.c_str(), "ServerSetPlayerCanDBNORevive") &&
        !strstr(FunctionName.c_str(), "BGA_Petrol_Pickup_C") &&
        !strstr(FunctionName.c_str(), "GetMutatorsForContextActor") &&
        !strstr(FunctionName.c_str(), "GetControlRotation") &&
        !strstr(FunctionName.c_str(), "K2_GetComponentLocation") &&
        !strstr(FunctionName.c_str(), "MoveFromOffset__UpdateFunc") &&
        !strstr(FunctionFullName.c_str(), "PinkOatmeal_GreenGlop_C") &&
        !strstr(ObjectName.c_str(), "CBGA_GreenGlop_WithGrav_C") &&
        !strstr(ObjectName.c_str(), "FlopperSpawn") &&
        !strstr(FunctionFullName.c_str(), "GCNL_EnvCampFire_Fire_C") &&
        !strstr(FunctionName.c_str(), "BlueprintGetAllHighlightableComponents") &&
        !strstr(FunctionFullName.c_str(), "Primitive_Structure_AmbAudioComponent") &&
        !strstr(FunctionName.c_str(), "ServerTriggerCombatEvent") &&
        !strstr(FunctionName.c_str(), "SpinCubeTimeline__UpdateFunc") &&
        !strstr(ObjectName.c_str(), "FortPhysicsObjectComponent") &&
        !strstr(FunctionName.c_str(), "GetTextValue") &&
        !strstr(FunctionName.c_str(), "ServerUpdateLevelVisibility") &&
        !strstr(FunctionName.c_str(), "UpdatePlayerPositionsWithinTheMID") &&
        !strstr(FunctionName.c_str(), "ExecuteUbergraph_BGA_Petrol_Pickup"))
    {
        std::cout << "[Diddy] Function called: " << FunctionFullName << " with " << ObjectName << std::endl;
    }

    return ProcessEventOriginal(Object, Function, Params);
}

static void (*RegisterComponent)(UActorComponent*) = decltype(RegisterComponent)(InSDKUtils::GetImageBase() + 0xF22AB4);
static void (*SetupAttachment)(USceneComponent*, USceneComponent*, FName) = decltype(SetupAttachment)(InSDKUtils::GetImageBase() + 0xF273DC);
static void (*UpdateBounds)(USceneComponent*) = decltype(UpdateBounds)(InSDKUtils::GetImageBase() + 0xAF1970);
static void (*CreatePhysicsMeshes)(UBodySetup*) = decltype(CreatePhysicsMeshes)(InSDKUtils::GetImageBase() + 0xC8AA04);

void (*_PostInitializeComponents)(AFortPoiVolume* PoiVolume);

void PostInitializeComponents(AFortPoiVolume* PoiVolume)
{
    if (PoiVolume->LocationTags.GameplayTags.Num() == 0 || !PoiVolume->LocationTags.GameplayTags[0].TagName.ComparisonIndex)
        return _PostInitializeComponents(PoiVolume);

    if (PoiVolume->PoiCollisionComp == NULL)
        std::cout << "WE ARE SO FUCKED I LIKE DICK" << std::endl;

    PoiVolume->BrushComponent = NewObject<UBrushComponent>(PoiVolume);
    PoiVolume->BrushComponent->Brush = PoiVolume->Brush;

    RegisterComponent(PoiVolume->BrushComponent);
    SetupAttachment(PoiVolume->PoiCollisionComp, PoiVolume->BrushComponent, PoiVolume->BrushComponent->AttachSocketName);

    FBoxSphereBounds& Bounds = *(FBoxSphereBounds*)(__int64(PoiVolume->Brush) + 0x230);

    FKBoxElem BoxElem = FKBoxElem();

    static UFortAssetManager* AssetManager = Cast<UFortAssetManager>(UFortEngine::GetEngine()->AssetManager);

    if (AssetManager != NULL)
    {
        static UFortQuestIndicatorData* IndicatorData = AssetManager->GameDataCommon->QuestIndicatorData;

        if (IndicatorData != NULL)
        {
            const TArray<FFortChallengeMapPoiData>& PoiDataArray = IndicatorData->ChallengeMapPoiData;

            for (const FFortChallengeMapPoiData& PoiData : PoiDataArray)
            {
                if (PoiData.LocationTag == PoiVolume->LocationTags.GameplayTags[0])
                {
                    std::cout << PoiData.Text.ToString().c_str() << ": " << PoiData.WorldLocation.X << " " << PoiData.WorldLocation.Y << " " << PoiData.WorldLocation.Z << std::endl;

                    FHitResult Hi;
                    PoiVolume->BrushComponent->K2_SetWorldLocation(PoiData.WorldLocation, false, &Hi, true);

                    BoxElem.Center = PoiData.WorldLocation;
                    break;
                }
            }
        }
    }

    BoxElem.X = Bounds.BoxExtent.X;
    BoxElem.Y = Bounds.BoxExtent.Y;
    BoxElem.Z = Bounds.BoxExtent.Z;

    PoiVolume->BrushComponent->BrushBodySetup = NewObject<UBodySetup>(PoiVolume);
    PoiVolume->BrushComponent->BrushBodySetup->AggGeom.BoxElems.Add(BoxElem);

    UpdateBounds(PoiVolume->BrushComponent); // fixes the tags shit i think but i think we got wrong offset
    CreatePhysicsMeshes(PoiVolume->BrushComponent->BrushBodySetup);

    PoiVolume->RootComponent = PoiVolume->BrushComponent;

	_PostInitializeComponents(PoiVolume);
}

void (*_OnServerStopCallback)(AFortPickup* Pickup, FHitResult* Hit);

void OnServerStopCallback(AFortPickup* Pickup, FHitResult* Hit) // 0x6532B8C
{
    _OnServerStopCallback(Pickup, Hit);

    UAthenaAIServiceLoot* AIServiceLoot = UAthenaAIBlueprintLibrary::GetAIServiceLoot(UWorld::GetWorld());

    if (AIServiceLoot != NULL && Pickup->RootComponent != NULL && !Pickup->bActorIsBeingDestroyed)
    {        
        static void (*RegisterPickupLoot)(UAthenaAIServiceLoot*, AFortPickup*) = 
            decltype(RegisterPickupLoot)(InSDKUtils::GetImageBase() + 0x5FC27BC);

        RegisterPickupLoot(AIServiceLoot, Pickup);
    }
}

FPoiDiscoverabilityDataEntry* UFortControllerComponent_MapDiscoverability::FindDiscoverDataForLocation(struct FGameplayTag* a2)
{
    for (FPoiDiscoverabilityDataEntry& Entry : this->DiscoveredData.Entries)
    {
        if (Entry.POITag == *a2)
            return &Entry;
    }

    return NULL;
}

void UFortControllerComponent_MapDiscoverability::HandleVisitedPOI(struct FGameplayTag* a2)
{
    static void (*_)(class UFortControllerComponent_MapDiscoverability* a1, struct FGameplayTag* a2)
        = decltype(_)(InSDKUtils::GetImageBase() + 0x5FD47A0);

    _(this, a2);
}

void UFortControllerComponent_MapDiscoverability::HandleDiscoveredPOI(struct FGameplayTag* a2)
{
    static void (*_)(class UFortControllerComponent_MapDiscoverability* a1, struct FGameplayTag* a2)
        = decltype(_)(InSDKUtils::GetImageBase() + 0x5FD30EC);

    _(this, a2);
}

bool AFortPoiVolume::CanRecordPlayerVisits(AFortPlayerController* PlayerController)
{
    static bool (*_)(AFortPlayerController* PlayerController)
        = decltype(_)(InSDKUtils::GetImageBase() + 0x60F79D4);

    return _(PlayerController);
}

bool (*_IsLocationBeingFreshlyVisitedByController)(AFortPoiManager* PoiManager, AFortPlayerControllerGameplay* Controller, FGameplayTag* Location);

bool IsLocationBeingFreshlyVisitedByController(AFortPoiManager* PoiManager, AFortPlayerControllerGameplay* Controller, FGameplayTag* Location) 
{
    std::cout << __FUNCTION__ << std::endl;

	bool Ret = _IsLocationBeingFreshlyVisitedByController(PoiManager, Controller, Location); // call first because technically we are under the function now where CheckPOIOnVisit is called

	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(Controller);

    if (PlayerController == NULL)
        return Ret;

    UFortControllerComponent_MapDiscoverability* MapDiscoverability = PlayerController->DiscoverabilityComponent;

    if (MapDiscoverability == NULL || !AFortPoiVolume::CanRecordPlayerVisits(PlayerController))
		return Ret;

	FPoiDiscoverabilityDataEntry* DiscoverabilityDataEntry = MapDiscoverability->FindDiscoverDataForLocation(Location);

    if (DiscoverabilityDataEntry != NULL && DiscoverabilityDataEntry->bDiscovered)
    {
        MapDiscoverability->HandleVisitedPOI(Location);
	}
    else
    {
        MapDiscoverability->HandleDiscoveredPOI(Location);
    }

    return Ret;
}

void (*_Destroyed)(AFortPickup* Pickup);
void Destroyed(AFortPickup* Pickup)
{
    UAthenaAIServiceLoot* AIServiceLoot = UAthenaAIBlueprintLibrary::GetAIServiceLoot(UWorld::GetWorld());

    if (AIServiceLoot != NULL)
    {
        AIServiceLoot->LootOctree->RemoveLootActor(Pickup);
    }

    _Destroyed(Pickup);
}

void (*_GetPoiGridTagsForLocation)(AFortPoiManager* PoiManager, FVector* WorldLocation, FGameplayTagContainer* OutGameplayTags);

void GetPoiGridTagsForLocation(AFortPoiManager* PoiManager, FVector* WorldLocation, FGameplayTagContainer* OutGameplayTags)
{
	_GetPoiGridTagsForLocation(PoiManager, WorldLocation, OutGameplayTags);

    //OutGameplayTags->GameplayTags.Add(PoiManager->AllPoiVolumes[5]->LocationTags.GameplayTags[0]);

    if (OutGameplayTags->GameplayTags.Num() && OutGameplayTags->GameplayTags[0].TagName.ComparisonIndex)
        std::cout << __FUNCTION__ << "(): GameplayTags.Num(): " << OutGameplayTags->GameplayTags[0].TagName.ToString().c_str() << std::endl;
}

DWORD WINAPI LaunchWindowsStartup(LPVOID)
{
    AllocConsole();

    FILE* f;
    freopen_s(&f, "CONOUT$", "w+", stdout);

    SetConsoleTitleA("Forsaken | Initializing");

    Sleep(5000);

    UKismetDetourLibrary::Setup();

    GameplayAbility::Setup();

    AbilitySystemComponent::Setup();

    McpProfileGroup::Setup();

    NetDriver::Setup();
    World::Setup();

    GameMode::Setup();
    GameModeBase::Setup();
    PlayerController::Setup();

    NavigationSystemV1::Setup();

    FortAthenaAIBotController::Setup();

    AthenaAIServicePlayerBots::Setup();

    FortAthenaAISpawnerDataComponent_AIBotCosmeticBase::Setup();
    FortAthenaAISpawnerDataComponent_InventoryBase::Setup();

    AthenaMarkerComponent::Setup();

    FortAthenaVehicleSpawner::Setup();
    FortGameModeAthena::Setup();
    FortGameStateAthena::Setup();

    BuildingContainer::Setup();
    BuildingSMActor::Setup();

    FortControllerComponent_Aircraft::Setup();

    FortInventoryOwnerInterface::Setup();
    FortWorldItem::Setup();

    FortMissionLibrary::Setup();

    FortPawn::Setup();
    FortPhysicsPawn::Setup();
    FortPlayerPawn::Setup();

    FortPlayerController::Setup();

    FortKismetLibrary::Setup();

    //BuildingGameplayActorSpawnMachine::Setup();

    UKismetMemoryLibrary::Patch<bool>(0xB4FF9E1, false);
    UKismetMemoryLibrary::Patch<bool>(0xB4FF9E0, true);

    UKismetMemoryLibrary::Patch<uint8_t>(0x60F7108, 0x75); // POI MANAGER

    UKismetDetourLibrary::InitializeDetour<AFortPoiVolume, MinHook>(0x1F06020, GetPoiGridTagsForLocation, &_GetPoiGridTagsForLocation);
    UKismetDetourLibrary::InitializeDetour<AFortPoiVolume, MinHook>(0x60FD8E8, IsLocationBeingFreshlyVisitedByController, &_IsLocationBeingFreshlyVisitedByController); // this is actually a shitty stub

    UKismetDetourLibrary::InitializeDetour<AFortPoiVolume, MinHook>(0x19B5B44, PostInitializeComponents, &_PostInitializeComponents);

    UKismetDetourLibrary::InitializeDetour<UAbilitySystemComponent, MinHook>(0x6532B8C, OnServerStopCallback, &_OnServerStopCallback);

    UKismetDetourLibrary::InitializeDetour<UAbilitySystemComponent, MinHook>(0x66032DC, UKismetDetourLibrary::ForceReturnTrue); // validateplayer
    UKismetDetourLibrary::InitializeDetour<UAbilitySystemComponent, MinHook>(0x7ABA87C, UKismetDetourLibrary::ForceReturnTrue); // kickplayer
    UKismetDetourLibrary::InitializeDetour<UAbilitySystemComponent, MinHook>(0x4f0f2cc, UKismetDetourLibrary::ForceReturnTrue); // canactivate
    UKismetDetourLibrary::InitializeDetour<UAbilitySystemComponent, MinHook>(0x5D627A0, UKismetDetourLibrary::ForceReturnTrue); // Bot shooting scuffed

    UKismetDetourLibrary::InitializeDetour<UAbilitySystemComponent, MinHook>(0x623591C, SpawnLoot); // canactivate
    UKismetDetourLibrary::InitializeDetour<UAbilitySystemComponent, MinHook>(0x6265508, PostUpdate, &_PostUpdate); // canactivate
    UKismetDetourLibrary::InitializeDetour<AFortPlayerPawnAthena, VFT>(0x1100 / 8, ServerHandlePickupInfo);
    UKismetDetourLibrary::InitializeDetour<AFortPickupAthena, VFT>(0x6d0 / 8, GivePickupTo, &_GivePickupTo);
    UKismetDetourLibrary::InitializeDetour<UFortAthenaBTService_PickUpLoot, VFT>(0x65, AssessLootInteractionInfo);
    UKismetDetourLibrary::InitializeDetour<AFortGameModeAthena, MinHook>(0x606df1c, PickTeam);
    UKismetDetourLibrary::InitializeDetour<AFortGameModeAthena, MinHook>(0xc77314, GetPlayerViewPointHook, &GetPlayerViewPointOriginal);
    //UKismetDetourLibrary::InitializeDetour<AFortGameModeAthena, MinHook>(0xB52C28, ProcessEventHook, &ProcessEventOriginal);
    UKismetDetourLibrary::InitializeDetour<AFortDecoTool_ContextTrap, VFT>(0x178, SpawnDecoInternal);
    UKismetDetourLibrary::InitializeDetour<AFortTrapTool, VFT>(0x178, SpawnDecoInternal);
    UKismetDetourLibrary::InitializeDetour<UFortControllerComponent_Interaction, VFT>(0x500 / 8, ServerAttemptInteract, &_ServerAttemptInteract);
    UKismetDetourLibrary::InitializeDetour<AFortPickup, VFT>(0xb6, Destroyed, &_Destroyed);
    //UKismetDetourLibrary::InitializeDetour<UFortControllerComponent_Interaction, VFT>(0x4e0 / 8, ServerNotifyStartLongUse, &_ServerAttemptInteract);

    UFunction* execSpawnPickup = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortAthenaSupplyDrop.SpawnPickup"));

    UKismetDetourLibrary::InitializeDetour<AFortAthenaSupplyDrop, Exec>(execSpawnPickup, SpawnPickup, &_SpawnPickup);

    UFunction* execGiveItemToInventoryOwner = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortAIPawn.SpawnPickupInWorldWithLootTierAsync"));

    UKismetDetourLibrary::InitializeDetour<AFortGameSession, Exec>(execGiveItemToInventoryOwner, SpawnPickupInWorldWithLootTierAsync, &_SpawnPickupInWorldWithLootTierAsync);

    UFunction* execAdd = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary.AddActorsToIndicatedList"));
    UFunction* execREmove = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary.RemoveActorFromIndicatedList"));

    UKismetDetourLibrary::InitializeDetour<AFortGameSession, Exec>(execAdd, AddActorsToIndicatedList, &_AddActorsToIndicatedList);
    UKismetDetourLibrary::InitializeDetour<AFortGameSession, Exec>(execREmove, RemoveActorFromIndicatedList, &_RemoveActorFromIndicatedList);

    UKismetMemoryLibrary::Patch<uint8_t>(0x45e0cc0, RETN); // AFortGameState::StoreGameSessionID
    UKismetMemoryLibrary::Patch<uint8_t>(0x19F9538, RETN); // requesr exit
    UKismetMemoryLibrary::Patch<uint8_t>(0xF3DBCD + 1, 0x85);

    for (int32 i = 0; i < UObject::GObjects->Num(); ++i)
    {
        UObject* Object = UObject::GObjects->GetByIndex(i);

        if (Object == NULL)
            continue;

        UKismetDetourLibrary::VirtualSwap(Object->VTable, 0x20, UKismetDetourLibrary::ForceReturnTrue);
    }

    UKismetDetourLibrary::Finalize();

    UFortEngine::GetEngine()->GameInstance->LocalPlayers.Remove(0);
    UGameplayStatics::OpenLevel(UWorld::GetWorld(), UKismetStringLibrary::Conv_StringToName(L"Artemis_Terrain"), false, FString());

    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), TEXT("log LogFortUIDirector off"), nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), TEXT("log LogFortQuest VeryVerbose"), nullptr);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CreateThread(0, 0, LaunchWindowsStartup, 0, 0, 0);

    return TRUE;
}

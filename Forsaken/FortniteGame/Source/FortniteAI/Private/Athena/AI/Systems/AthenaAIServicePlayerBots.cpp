#include "Athena/AI/Systems/AthenaAIServicePlayerBots.h"

void AthenaAIServicePlayerBots::WaitForMatchAssignmentReady(UAthenaAIServicePlayerBots* AIServicePlayerBots, void* FlowHandle)
{
    AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

    if (GameMode == NULL)
        return;

    *(int32*)(__int64(AIServicePlayerBots) + 0xB18) = 1;

    AIServicePlayerBots->ThankBusDriverProbability.Value = 0.15f;
    AIServicePlayerBots->ThankBusDriverMinTime.Value = 1.f;
    AIServicePlayerBots->ThankBusDriverMaxTime.Value = 10.f;
    AIServicePlayerBots->bDoSpawnBotFlow = true;
    AIServicePlayerBots->bWaitForNavmeshToBeLoaded = false;
    AIServicePlayerBots->UseZiplines.Value = true;
    AIServicePlayerBots->UseCustomSupportedItemList.Value = false;
    AIServicePlayerBots->GamePhaseToStartSpawning = EAthenaGamePhase::Warmup;
    AIServicePlayerBots->DefaultBotAISpawnerData = StaticLoadObject<UClass>(TEXT("/Game/Athena/AI/Phoebe/BP_AISpawnerData_Phoebe.BP_AISpawnerData_Phoebe_C"));

    FMMRSpawningInfo SpawningInfo = FMMRSpawningInfo();

    SpawningInfo.AISpawnerData = AIServicePlayerBots->DefaultBotAISpawnerData;
    SpawningInfo.BotSpawningDataInfoTargetELO = 1000.f;
    SpawningInfo.BotSpawningDataInfoWeight = 100.f;
    SpawningInfo.NumBotsToSpawn = 100 /*GameMode->MaxPlayerCount*/ - UKismetMathLibrary::RandomIntegerInRange(5, 20); // todo

    AIServicePlayerBots->CachedMMRSpawningInfo.SpawningInfos.Add(SpawningInfo);

    Original::WaitForMatchAssignmentReady(AIServicePlayerBots, FlowHandle);
}

void AthenaAIServicePlayerBots::CacheFallbackBuildingFoundations(UAthenaAIServicePlayerBots* AIServicePlayerBots)
{
    TArray<ABuildingFoundation*> Foundations = GetAllActorsOfClassEasy<ABuildingFoundation>();

    for (ABuildingFoundation* Foundation : Foundations)
    {
        AIServicePlayerBots->CachedBuildingFoundations.Add(Foundation);
    }

    Foundations.Free();

    Original::CacheFallbackBuildingFoundations(AIServicePlayerBots);
}

void AthenaAIServicePlayerBots::CacheBusDropPOILocations(UAthenaAIServicePlayerBots* AIServicePlayerBots)
{
    std::cout << "BEFORE - CachedValidPOIVolumeLocations.Num(): " << AIServicePlayerBots->CachedValidPOIVolumeLocations.Num() << std::endl;
    std::cout << "BEFORE - BattleBusTagQueryPOIList.Num(): " << AIServicePlayerBots->BattleBusTagQueryPOIList.Num() << std::endl;
    std::cout << "BEFORE - SecondaryBattleBusTagQueryPOIList.Num(): " << AIServicePlayerBots->SecondaryBattleBusTagQueryPOIList.Num() << std::endl;
    std::cout << "BEFORE - OnGroundTagQueryPOIList.Num(): " << AIServicePlayerBots->OnGroundTagQueryPOIList.Num() << std::endl;

    TArray<AFortPoiVolume*> PoiVolumes = GetAllActorsOfClassEasy<AFortPoiVolume>();

    for (AFortPoiVolume* PoiVolume : PoiVolumes)
    {
        TArray<FGameplayTag>& GameplayTags = PoiVolume->LocationTags.GameplayTags;

        if (GameplayTags.Num())
        {
            const FGameplayTag& LocationTag = GameplayTags[0];

            const FString Location = UKismetStringLibrary::Conv_NameToString(LocationTag.TagName);

            if (!UKismetStringLibrary::StartsWith(Location, TEXT("athena.location.meta."), ESearchCase::IgnoreCase) &&
                !UKismetStringLibrary::StartsWith(Location, TEXT("athena.location.biome."), ESearchCase::IgnoreCase) &&
                !UKismetStringLibrary::StartsWith(Location, TEXT("athena.quests."), ESearchCase::IgnoreCase))
            {
                FCachedPOIVolumeLocations CachedPOIVolumeLocation = FCachedPOIVolumeLocations();

                CachedPOIVolumeLocation.POIVolume = PoiVolume;

                AIServicePlayerBots->CachedValidPOIVolumeLocations.Add(CachedPOIVolumeLocation);
            }
        }
    }

    PoiVolumes.Free();

    for (FCachedPOIVolumeLocations& CachedPOIVolumeLocation : AIServicePlayerBots->CachedValidPOIVolumeLocations)
    {
        static auto PopulatePOIFilterQuery = [](FGameplayTagQuery& POIFilterQuery, const FGameplayTag& LocationTag) -> void
            {
                if (LocationTag.TagName.ComparisonIndex != 0)
                {
                    POIFilterQuery.TagDictionary.Add(LocationTag);
                    POIFilterQuery.AutoDescription
                        = FString::Printf(TEXT(" ANY( %s )"), UKismetStringLibrary::Conv_NameToString(LocationTag.TagName).CStr());
                }

                POIFilterQuery.TokenStreamVersion = 0;
                POIFilterQuery.QueryTokenStream.Reset();
                POIFilterQuery.QueryTokenStream.Add(0);
                POIFilterQuery.QueryTokenStream.Add(1);
                POIFilterQuery.QueryTokenStream.Add(1);
                POIFilterQuery.QueryTokenStream.Add(0);
            };

        AFortPoiVolume* PoiVolume = CachedPOIVolumeLocation.POIVolume;

        if (PoiVolume == NULL)
            continue;

        TArray<FGameplayTag>& GameplayTags = CachedPOIVolumeLocation.POIVolume->LocationTags.GameplayTags;

        if (GameplayTags.Num() == 0)
            continue;

        const FGameplayTag& LocationTag = GameplayTags[0];

        const FString Location = UKismetStringLibrary::Conv_NameToString(LocationTag.TagName);

        FBattleBusPOI BattleBusPOI = FBattleBusPOI();
        {
            //std::cout << "Adding " << Location.ToString().c_str() << "to BattleBusTagQueryPOIList & SecondaryBattleBusTagQueryPOIList!" << std::endl;

            BattleBusPOI.IsEnabled.Value = 1.0f;

            PopulatePOIFilterQuery(BattleBusPOI.POIFilterQuery, LocationTag);

            AIServicePlayerBots->BattleBusTagQueryPOIList.Add(BattleBusPOI);
            AIServicePlayerBots->SecondaryBattleBusTagQueryPOIList.Add(BattleBusPOI);
        }

        if (UKismetStringLibrary::StartsWith(Location, TEXT("athena.location.poi."), ESearchCase::IgnoreCase))
        {
            FNavigationPOI NavigationPOI = FNavigationPOI();
            {
                //std::cout << "Adding " << Location.ToString().c_str() << "to OnGroundTagQueryPOIList!" << std::endl;

                NavigationPOI.IsEnabled.Value = 1.0f;
                NavigationPOI.ValidPOIVolumeList.Add(PoiVolume);

                PopulatePOIFilterQuery(NavigationPOI.POIFilterQuery, LocationTag);

                AIServicePlayerBots->OnGroundTagQueryPOIList.Add(NavigationPOI);
            }
        }
    }

    Original::CacheBusDropPOILocations(AIServicePlayerBots);

    std::cout << "AFTER - BattleBusTagQueryPOIList.Num(): " << AIServicePlayerBots->BattleBusTagQueryPOIList.Num() << std::endl;
    std::cout << "AFTER - SecondaryBattleBusTagQueryPOIList.Num(): " << AIServicePlayerBots->SecondaryBattleBusTagQueryPOIList.Num() << std::endl;
    std::cout << "AFTER - OnGroundTagQueryPOIList.Num(): " << AIServicePlayerBots->OnGroundTagQueryPOIList.Num() << std::endl;
    std::cout << "AFTER - CachedValidPOIVolumeLocations.Num(): " << AIServicePlayerBots->CachedValidPOIVolumeLocations.Num() << std::endl;
}

void AthenaAIServicePlayerBots::Setup()
{
    UKismetDetourLibrary::InitializeDetour<UAthenaAIServicePlayerBots, MinHook>(0x5FC8BE8, WaitForMatchAssignmentReady, &Original::WaitForMatchAssignmentReady);
    UKismetDetourLibrary::InitializeDetour<UAthenaAIServicePlayerBots, MinHook>(0x5FB35B4, CacheFallbackBuildingFoundations, &Original::CacheFallbackBuildingFoundations);
    UKismetDetourLibrary::InitializeDetour<UAthenaAIServicePlayerBots, MinHook>(0x5FB3400, CacheBusDropPOILocations, &Original::CacheBusDropPOILocations);
}

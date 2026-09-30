#include "FortniteGame/Source/FortniteAI/Public/Athena/AI/FortAthenaAISpawnerDataComponent_AIBotCosmeticBase.h"

void FortAthenaAISpawnerDataComponent_AIBotCosmeticBase::GetLoadout(UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase* AIBotCosmeticBase, FFortAthenaLoadout* OutLoadout)
{
	std::cout << __FUNCTION__ << std::endl;

	UFortAthenaAISpawnerDataComponent_CosmeticLoadout* CosmeticLoadout = Cast<UFortAthenaAISpawnerDataComponent_CosmeticLoadout>(AIBotCosmeticBase);

	if (CosmeticLoadout == NULL)
		return;

	*OutLoadout = CosmeticLoadout->CosmeticLoadout;
}

void (*_GetCustomCharacterParts)(UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase* AIBotCosmeticBase, FFrame& Stack, void* Ret);
void FortAthenaAISpawnerDataComponent_AIBotCosmeticBase::GetCustomCharacterParts(UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase* AIBotCosmeticBase, FFrame& Stack, void* Ret)
{
	TArray<class UCustomCharacterPart*>* OutCustomCharacterPartsBuf;

	auto& OutCustomCharacterParts = Stack.StepCompiledInRef<TArray<class UCustomCharacterPart*>>(&OutCustomCharacterPartsBuf);

	Stack.StepCompiledIn(&OutCustomCharacterParts);

	_GetCustomCharacterParts(AIBotCosmeticBase, Stack, Ret);

	UFortAthenaAISpawnerDataComponent_CosmeticLoadout* CosmeticLoadout = Cast<UFortAthenaAISpawnerDataComponent_CosmeticLoadout>(AIBotCosmeticBase);

	if (CosmeticLoadout == NULL)
		return;


	for (auto& part : CosmeticLoadout->CustomCharacterParts)
	{
		OutCustomCharacterParts.Add(part);
	}

	std::cout << OutCustomCharacterParts.Num() << "!!!!!!!!!!!!!!!!!!!" << std::endl;
}

void FortAthenaAISpawnerDataComponent_AIBotCosmeticBase::OnSpawned(UFortAthenaAISpawnerDataComponent_CosmeticLoadout* AIBotCosmeticBase, AFortPlayerPawn* PawnAI)
{
	static void (*ApplyCustomizationToCharacter)(AFortPlayerState*, AFortAthenaAIBotController*, AFortPlayerPawn*) =
		decltype(ApplyCustomizationToCharacter)(InSDKUtils::GetImageBase() + 0x67A4C64);

	ApplyCustomizationToCharacter(Cast<AFortPlayerState>(PawnAI->PlayerState), Cast<AFortAthenaAIBotController>(PawnAI->Controller), PawnAI);
}

void FortAthenaAISpawnerDataComponent_AIBotCosmeticBase::Setup()
{
	//UKismetDetourLibrary::InitializeDetour<UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase, MinHook>(0x6A109B4, GetLoadout);
	//UKismetDetourLibrary::InitializeDetour<UFortAthenaAISpawnerDataComponent_CosmeticLoadout, VFT>(0x56, OnSpawned);

	UFunction* execGetCustomCharacterParts = StaticFindObject<UFunction>(TEXT("/Script/FortniteGame.FortAthenaAISpawnerDataComponent_AIBotCosmeticBase.GetCustomCharacterParts"));

	//UKismetDetourLibrary::InitializeDetour<UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase, Exec>(execGetCustomCharacterParts, GetCustomCharacterParts, &_GetCustomCharacterParts);
}

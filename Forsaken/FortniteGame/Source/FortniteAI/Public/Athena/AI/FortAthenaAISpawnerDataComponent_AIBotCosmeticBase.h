#pragma once

#include "CoreGlobals.h"

class FortAthenaAISpawnerDataComponent_AIBotCosmeticBase
{
private:
	static void GetLoadout(UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase* AIBotCosmeticBase, FFortAthenaLoadout* OutLoadout);
	static void GetCustomCharacterParts(UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase* AIBotCosmeticBase, FFrame& Stack, void* Ret);

	static void OnSpawned(UFortAthenaAISpawnerDataComponent_CosmeticLoadout* AIBotCosmeticBase, AFortPlayerPawn* PawnAI);
public:
	static void Setup();
};

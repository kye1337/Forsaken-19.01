#pragma once

#include "CoreGlobals.h"

class FortAthenaAIBotController
{
private:
	class Original
	{
	public:
		static inline void (*OnPawnAISpawned)(AActor* Actor, AFortPawn* AIPawn);
		static inline AFortAthenaAIBotController* (*GetSquadLeader)(AFortAthenaAIBotController* BotController);
		static inline void (*OnPossesedPawnDied)(AFortAthenaAIBotController* BotController, AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser, const FVector& HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, const FVector& Momentum);
	};
public:
	static UFortWorldItem* CreateItem(AFortAthenaAIBotController* BotController, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo, int32 PhantomReserveAmmo);
	static void RemoveAllInventoryItems(AFortAthenaAIBotController* BotController);
private:
	static void OnPawnAISpawned(AActor* Actor, AFortPawn* AIPawn);
	static AFortAthenaAIBotController* GetSquadLeader(AFortAthenaAIBotController* BotController);
	static void OnPossesedPawnDied(AFortAthenaAIBotController* BotController, AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser, const FVector& HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, const FVector& Momentum);
public:
	static void Setup();
};

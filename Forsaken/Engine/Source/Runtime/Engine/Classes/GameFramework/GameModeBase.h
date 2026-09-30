#pragma once

#include "CoreGlobals.h"

class GameModeBase
{
private:
	class Original
	{
	public:
		static inline void (*HandleStartingNewPlayer)(AGameModeBase* GameMode, APlayerController* NewPlayer);
	};
private:
	static APawn* SpawnDefaultPawnFor(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot);
	static void HandleStartingNewPlayer(AGameModeBase* GameMode, APlayerController* NewPlayer);
public:
	static void Setup();
};

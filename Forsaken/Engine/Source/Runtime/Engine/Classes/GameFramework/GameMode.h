#pragma once

#include "CoreGlobals.h"
#include "Engine/World.h"

class GameMode
{
private:
	static bool ReadyToStartMatch(AGameMode* GameMode);
public:
	static void Setup();
};

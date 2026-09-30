#pragma once

#include "CoreGlobals.h"
#include "Engine/EngineBaseTypes.h"

class World
{
public:
	static bool Listen(UWorld* InWorld, FURL& InURL);
private:
	static ENetMode GetNetMode()
	{
		return NM_DedicatedServer;
	}
public:
	static void Setup();
};

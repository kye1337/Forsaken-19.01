#pragma once

#include "CoreGlobals.h"

class FortMissionLibrary
{
private:
	class Original
	{
	public:
		static inline bool (*TeleportPlayerPawn)(UObject* Context, FFrame& Stack, bool* Ret);
	};
private:
	static bool TeleportPlayerPawn(UObject* Context, FFrame& Stack, bool* Ret);
public:
	static void Setup();
};

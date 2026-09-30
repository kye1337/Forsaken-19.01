#pragma once

#include "CoreGlobals.h"

class GameplayAbility
{
private:
	class Original
	{
	public:
		static inline void (*K2_ExecuteGameplayCue)(UObject* Context, FFrame& Stack, void* Ret);
	};
private:
	static void K2_ExecuteGameplayCue(UObject* Context, FFrame& Stack, void* Ret);
public:
	static void Setup();
};

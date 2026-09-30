#pragma once

#include "CoreGlobals.h"

class NavigationSystemV1
{
private:
	class Original
	{
	public:
		static inline void (*InitializeForWorld)(UNavigationSystemV1* NavigationSystem, UWorld& World, EFNavigationSystemRunMode Mode);
	};
private:
	static void InitializeForWorld(UNavigationSystemV1* NavigationSystem, UWorld& World, EFNavigationSystemRunMode Mode);
public:
	static void Setup();
};
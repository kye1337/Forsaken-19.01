#pragma once

#include "CoreGlobals.h"

class BuildingContainer
{
private:
	class Original
	{
	public:
		static inline void (*OnAuthorityRandomUpgradeApplied)(ABuildingContainer* Container, const FName* UpgradeTierGroup);
	};
private:
	static void OnAuthorityRandomUpgradeApplied(ABuildingContainer* Container, const FName* UpgradeTierGroup);
public:
	static void Setup();
};

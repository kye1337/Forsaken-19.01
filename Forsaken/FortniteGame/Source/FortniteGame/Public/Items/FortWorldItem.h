#pragma once

#include "CoreGlobals.h"
#include "FortInventory.h"

class FortWorldItem
{
private:
	static void SetLoadedAmmo(UFortWorldItem* WorldItem, int32 InCount);
	static void SetPhantomReserveAmmo(UFortWorldItem* WorldItem, int32 InCount);
public:
	static void Setup();
};

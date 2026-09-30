#pragma once

#include "CoreGlobals.h"

class NetDriver
{
private:
	class Original
	{
	public:
		static inline void (*TickFlush)(UNetDriver* NetDriver, float DeltaSeconds);
	};
private:
	static void TickFlush(UNetDriver* NetDriver, float DeltaSeconds);
public:
	static void Setup();
};

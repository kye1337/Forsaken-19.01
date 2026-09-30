#pragma once

#include "CoreGlobals.h"

class FortGameStateAthena
{
private:
	class Original
	{
	public:
		static inline void (*PlacePlayersInAircraft)(AFortGameStateAthena* GameState);
	};
private:
	static void PlacePlayersInAircraft(AFortGameStateAthena* GameState);
public:
	static void Setup();
};

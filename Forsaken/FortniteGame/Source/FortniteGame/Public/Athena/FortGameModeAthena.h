#pragma once

#include "CoreGlobals.h"

class FortGameModeAthena
{
private:
	class Original
	{
	public:
		static inline void (*PlaceBotOnTeam)(AFortGameModeAthena* GameMode, AFortPlayerStateAthena* FPSA, AFortTeamInfoAthena* TeamToBePutOn, uint8 BotSquadId);
		static inline void (*StartNewSafeZonePhase)(AFortGameModeAthena* GameMode, int32 NewSafeZonePhase);
		static inline void (*StartAircraftPhase)(AFortGameModeAthena* GameMode, bool bGoStraightToSafeZone);
	};
private:
	static void PlaceBotOnTeam(AFortGameModeAthena* GameMode, AFortPlayerStateAthena* FPSA, AFortTeamInfoAthena* TeamToBePutOn, uint8 BotSquadId);
	static void StartNewSafeZonePhase(AFortGameModeAthena* GameMode, int32 NewSafeZonePhase);
	static void StartAircraftPhase(AFortGameModeAthena* GameMode, bool bGoStraightToSafeZone);
public:
	static void Setup();
};

#pragma once

#include "CoreGlobals.h"

class AthenaAIServicePlayerBots
{
private:
	class Original
	{
	public:
		static inline void (*WaitForMatchAssignmentReady)(UAthenaAIServicePlayerBots* AIServicePlayerBots, void* FlowHandle);
		static inline void (*CacheFallbackBuildingFoundations)(UAthenaAIServicePlayerBots* AIServicePlayerBots);
		static inline void (*CacheBusDropPOILocations)(UAthenaAIServicePlayerBots* AIServicePlayerBots);

	};
private:
	static void WaitForMatchAssignmentReady(UAthenaAIServicePlayerBots* AIServicePlayerBots, void* FlowHandle);
	static void CacheFallbackBuildingFoundations(UAthenaAIServicePlayerBots* AIServicePlayerBots);
	static void CacheBusDropPOILocations(UAthenaAIServicePlayerBots* AIServicePlayerBots);

public:
	static void Setup();
};

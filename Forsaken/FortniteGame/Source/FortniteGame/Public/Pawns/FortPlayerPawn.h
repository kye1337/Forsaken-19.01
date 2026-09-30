#pragma once

#include "CoreGlobals.h"

class FortPlayerPawn
{
private:
	class Original
	{
	public:
		static inline void (*TryToAutoPickup)(AFortPlayerPawn* Pawn, AFortPickup* Pickup);
	};
private:
	static void ServerReviveFromDBNO(AFortPlayerPawn* Pawn, AController* EventInstigator);
	static void TryToAutoPickup(AFortPlayerPawn* Pawn, AFortPickup* Pickup);
	static void ServerSendZiplineState(AFortPlayerPawn* Pawn, const FZiplinePawnState& InZiplineState);
public:
	static void Setup();
};

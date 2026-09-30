#pragma once

#include "CoreGlobals.h"

class AthenaMarkerComponent
{
private:
	static void ServerAddMapMarker(UAthenaMarkerComponent* MarkerComponent, const FFortClientMarkerRequest& MarkerRequest);
	static void ServerRemoveMapMarker(UAthenaMarkerComponent* MarkerComponent, FMarkerID MarkerID, ECancelMarkerReason CancelReason);
public:
	static void Setup();
};

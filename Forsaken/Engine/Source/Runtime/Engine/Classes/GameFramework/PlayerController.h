#pragma once

#include "CoreGlobals.h"

class PlayerController
{
private:
	static void ServerAcknowledgePossession(APlayerController* PlayerController, APawn* P);
public:
	static void Setup();
};

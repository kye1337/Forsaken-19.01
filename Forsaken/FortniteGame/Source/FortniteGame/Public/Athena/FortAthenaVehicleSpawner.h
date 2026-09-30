#pragma once

#include "CoreGlobals.h"

class FortAthenaVehicleSpawner
{
private:
	class Original
	{
	public:
		static inline void (*BeginPlay)(AFortAthenaVehicleSpawner* VehicleSpawner);
	};
private:
	static void SpawnVehicle(AFortAthenaVehicleSpawner* VehicleSpawner);
	static void BeginPlay(AFortAthenaVehicleSpawner* VehicleSpawner);
public:
	static void Setup();
};

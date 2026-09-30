#include "Athena/FortAthenaVehicleSpawner.h"

void FortAthenaVehicleSpawner::SpawnVehicle(AFortAthenaVehicleSpawner* VehicleSpawner)
{
	FVector VehicleLocation = VehicleSpawner->K2_GetActorLocation();
	FRotator VehicleRotation = VehicleSpawner->K2_GetActorRotation();

	UClass* VehicleClass = VehicleSpawner->GetVehicleClass();

	SpawnActor<AFortAthenaVehicle>(VehicleClass, VehicleLocation, VehicleRotation);
}

void FortAthenaVehicleSpawner::BeginPlay(AFortAthenaVehicleSpawner* VehicleSpawner)
{
	Original::BeginPlay(VehicleSpawner);

	SpawnVehicle(VehicleSpawner);
}

void FortAthenaVehicleSpawner::Setup()
{
	UKismetDetourLibrary::InitializeDetour<AFortAthenaVehicleSpawner, VFT>(0x6f, BeginPlay, &Original::BeginPlay);
}

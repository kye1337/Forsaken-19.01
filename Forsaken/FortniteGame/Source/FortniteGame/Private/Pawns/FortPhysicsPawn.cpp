#include "Pawns/FortPhysicsPawn.h"

void FortPhysicsPawn::ServerMove(AFortPhysicsPawn* PhysicsPawn, FReplicatedPhysicsPawnState& InState)
{
	UFortVehicleSkelMeshComponent* MeshComponent = Cast<UFortVehicleSkelMeshComponent>(PhysicsPawn->RootComponent);

	if (MeshComponent == NULL)
		return;

	PhysicsPawn->ReplicatedMovement.LinearVelocity = InState.LinearVelocity;
	PhysicsPawn->ReplicatedMovement.AngularVelocity = InState.AngularVelocity;
	PhysicsPawn->ReplicatedMovement.Location = InState.Translation;
	PhysicsPawn->ReplicatedMovement.Rotation = UKismetMathLibrary::Quat_Rotator(InState.Rotation);
	PhysicsPawn->OnRep_ReplicatedMovement();

	MeshComponent->SetAllPhysicsLinearVelocity(InState.LinearVelocity, false);
	MeshComponent->SetAllPhysicsAngularVelocityInRadians(InState.AngularVelocity, false);
	MeshComponent->K2_SetWorldLocationAndRotation(InState.Translation, PhysicsPawn->ReplicatedMovement.Rotation, false, NULL, true);
}

void FortPhysicsPawn::Setup()
{
	for (int32 i = 0; i < UObject::GObjects->Num(); ++i)
	{
		UObject* Object = UObject::GObjects->GetByIndex(i);

		if (Object == NULL)
			continue;

		if (Object->IsA(AFortPhysicsPawn::StaticClass()))
		{
			UKismetDetourLibrary::VirtualSwap(Object->VTable, 0x102, ServerMove);
		}
	}
}

#include "Athena/Markers/AthenaMarkerComponent.h"

void AthenaMarkerComponent::ServerAddMapMarker(UAthenaMarkerComponent* MarkerComponent, const FFortClientMarkerRequest& MarkerRequest)
{
	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(MarkerComponent->GetOwner());

	if (PlayerController == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	FFortWorldMarkerData MarkerData = FFortWorldMarkerData();

	FMarkerID MarkerID{};
	MarkerID.PlayerId = PlayerState->PlayerId;
	MarkerID.InstanceID = MarkerRequest.InstanceID;

	MarkerData.MarkerID = MarkerID;
	MarkerData.Owner = PlayerState;
	MarkerData.MarkerType = MarkerRequest.MarkerType;

	MarkerData.BasePosition = MarkerRequest.BasePosition;
	MarkerData.BasePositionOffset = MarkerRequest.BasePositionOffset;
	MarkerData.WorldNormal = MarkerRequest.WorldNormal;

	if (MarkerRequest.MarkedActor)
	{
		MarkerComponent->BuildMarkerDataForActor(MarkerRequest.MarkedActor, &MarkerData);

		if (AFortPickup* Pickup = Cast<AFortPickup>(MarkerRequest.MarkedActor))
		{
			MarkerData.ItemDefinition = Pickup->PrimaryPickupItemEntry.ItemDefinition;
			MarkerData.ItemCount = Pickup->PrimaryPickupItemEntry.Count;
		}
	}

	MarkerData.bIncludeSquad = MarkerRequest.bIncludeSquad;
	MarkerData.bHasCustomDisplayInfo = false;
	MarkerData.bUseHoveredMarkerDetail = MarkerRequest.bUseHoveredMarkerDetail;

	for (AController* TeamMember : PlayerState->PlayerTeam->TeamMembers)
	{
		AFortPlayerControllerAthena* TeamMemberAthena = Cast<AFortPlayerControllerAthena>(TeamMember);

		if (TeamMember == NULL || TeamMemberAthena == PlayerController)
			continue;

		UAthenaMarkerComponent* TeamMemberMarkerComponent = TeamMemberAthena->MarkerComponent;

		if (TeamMemberMarkerComponent == NULL)
			continue;

		TeamMemberMarkerComponent->LocalPlaceMapMarker(MarkerData);

		TeamMemberMarkerComponent->MarkerStream.MarkArrayDirty();
	}
}

void AthenaMarkerComponent::ServerRemoveMapMarker(UAthenaMarkerComponent* MarkerComponent, FMarkerID MarkerID, ECancelMarkerReason CancelReason)
{
	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(MarkerComponent->GetOwner());

	if (PlayerController == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	for (AController* TeamMember : PlayerState->PlayerTeam->TeamMembers)
	{
		AFortPlayerControllerAthena* TeamMemberAthena = Cast<AFortPlayerControllerAthena>(TeamMember);

		if (TeamMember == NULL || TeamMemberAthena == PlayerController)
			continue;

		UAthenaMarkerComponent* TeamMemberMarkerComponent = TeamMemberAthena->MarkerComponent;

		if (TeamMemberMarkerComponent == NULL)
			continue;

		UFortWorldMarker* WorldMarker = NULL;

		for (FFortWorldMarkerData& Marker : TeamMemberMarkerComponent->MarkerStream.Markers)
		{
			if (Marker.MarkerID == MarkerID)
			{
				WorldMarker = Marker.MarkerInstance;
				break;
			}
		}

		if (WorldMarker == NULL)
			continue;

		WorldMarker->NotifyRemove();

		for (int32 i = 0; i < TeamMemberMarkerComponent->MarkerStream.Markers.Num(); ++i)
		{
			UFortWorldMarker* MarkerInstance = TeamMemberMarkerComponent->MarkerStream.Markers[i].MarkerInstance;

			if (MarkerInstance == WorldMarker)
			{
				TeamMemberMarkerComponent->MarkerStream.Markers.Remove(i);
				TeamMemberMarkerComponent->MarkerStream.MarkArrayDirty();
				break;
			}
		}
	}
}

void AthenaMarkerComponent::Setup()
{
	//UKismetDetourLibrary::InitializeDetour<UAthenaMarkerComponent, VFT>(0x460 / 8, ServerAddMapMarker);
	//UKismetDetourLibrary::InitializeDetour<UAthenaMarkerComponent, VFT>(0x458 / 8, ServerRemoveMapMarker);
}

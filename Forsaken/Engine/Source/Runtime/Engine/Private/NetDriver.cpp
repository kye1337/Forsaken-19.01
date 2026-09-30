#include "Engine/NetDriver.h"

void NetDriver::TickFlush(UNetDriver* NetDriver, float DeltaSeconds)
{
	if (UReplicationDriver* ReplicationDriver = NetDriver->ReplicationDriver)
	{
		ReplicationDriver->ServerReplicateActors();
	}

	if (GetAsyncKeyState(VK_F5))
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"startaircraft", NULL);
	}

	Original::TickFlush(NetDriver, DeltaSeconds);
}

void NetDriver::Setup()
{
	UKismetDetourLibrary::InitializeDetour<UNetDriver, MinHook>(0xB4E820, TickFlush, &Original::TickFlush);
}

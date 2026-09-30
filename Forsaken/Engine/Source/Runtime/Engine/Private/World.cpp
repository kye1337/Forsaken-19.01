#include "Engine/World.h"

bool World::Listen(UWorld* InWorld, FURL& InURL)
{
	if (InWorld->NetDriver)
		return false;

	UEngine* Engine = UEngine::GetEngine();

	if (Engine == NULL)
		return false;

	static FName NAME_GameNetDriver = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver"); // todo

	UNetDriver* NetDriver = Engine->CreateNetDriver(InWorld, NAME_GameNetDriver);

	if (NetDriver == NULL)
		return false;

	NetDriver->NetDriverName = NAME_GameNetDriver;
	NetDriver->World = InWorld;

	FString Error;

	if (!NetDriver->InitListen(InWorld, InURL, false, Error))
		return false;

	NetDriver->SetWorld(InWorld);

	InWorld->NetDriver = NetDriver;

	for (FLevelCollection& LevelCollection : InWorld->LevelCollections)
	{
		LevelCollection.NetDriver = NetDriver;
	}

	return true;
}

void World::Setup()
{
	UKismetDetourLibrary::InitializeDetour<UWorld, MinHook>(0xe3f070, GetNetMode);
}

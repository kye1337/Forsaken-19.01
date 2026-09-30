#include "Engine/Source/Runtime/NavigationSystem/Public/NavigationSystem.h"

void NavigationSystemV1::InitializeForWorld(UNavigationSystemV1* NavigationSystem, UWorld& World, EFNavigationSystemRunMode Mode)
{
	NavigationSystem->bAutoCreateNavigationData = true;

	Original::InitializeForWorld(NavigationSystem, World, Mode);
}

void NavigationSystemV1::Setup()
{
	UKismetDetourLibrary::InitializeDetour<UAthenaNavSystem, VFT>(0x5c, InitializeForWorld, &Original::InitializeForWorld);
}

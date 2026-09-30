#pragma once

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "stdio.h"

#pragma comment(lib, "Engine/Source/ThirdParty/MinHook/1.3.3/lib/minhook.lib")
#include "Engine/Source/ThirdParty/MinHook/1.3.3/include/MinHook.h"

#include "SDK/Source/Runtime/DevelopmentKit/SDK.hpp"
using namespace SDK;

#include "Kismet/KismetDetourLibrary.h"
#include "Kismet/KismetMemoryLibrary.h"

#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

#define ANY_PACKAGE (UObject*)-1

#define UPROPERTY(TYPE, NAME) \
	TYPE NAME = {}; \
	Stack.StepCompiledIn(&NAME)

#define TARRAY(ElementType, VarName) \
	TArray<ElementType> VarName##TempBuf; \
	TArray<ElementType>& VarName = Stack.StepCompiledInRef<TArray<ElementType>>(&VarName##TempBuf)

template <typename T>
static T* GetInterfaceAddress(UObject* Object)
{
	return ((T * (*)(UObject*, UClass*))(InSDKUtils::GetImageBase() + 0xB9E9B4))(Object, T::StaticClass());
}

static UObject* (*_StaticFindObject)(UClass* Class, UObject* Package, const TCHAR* OrigInName, bool ExactClass) = decltype(_StaticFindObject)(__int64(GetModuleHandleW(0)) + 0xb09f90);
static UObject* (*_StaticLoadObject)(UClass* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* Filename, uint32_t LoadFlags, UObject* Sandbox, bool bAllowObjectReconciliation) = decltype(_StaticLoadObject)(__int64(GetModuleHandleW(0)) + 0x145fc7c);

template <typename T = UObject>
inline T* StaticFindObject(const TCHAR* ObjectPath, UObject* Package = nullptr, UClass* Class = nullptr)
{
	return (T*)_StaticFindObject(Class, Package, ObjectPath, false);
}

template <typename T = UObject>
inline T* StaticLoadObject(const TCHAR* Path, UClass* InClass = T::StaticClass(), UObject* Outer = nullptr)
{
	return (T*)_StaticLoadObject(InClass, Outer, Path, nullptr, 0, nullptr, false);
}

template <typename T = AActor>
inline TArray<T*> GetAllActorsOfClassEasy(UClass* Class = T::StaticClass())
{
	TArray<T*> Actors;
	UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), Class, (TArray<AActor*>*) & Actors);

	return Actors;
}

template <typename T = UObject>
inline T* NewObject(UObject* Outer = NULL, UClass* ObjectClass = T::StaticClass())
{
	return Cast<T>(UGameplayStatics::SpawnObject(ObjectClass, Outer));
}

template <typename T = UObject>
inline T* GetComponentByClass(AActor* Actor, UClass* ComponentClass = T::StaticClass())
{
	return Cast<T>(Actor->GetComponentByClass(ComponentClass));
}

inline UCurveTable* GetGameData()
{
	static UCurveTable* Ret = NULL;

	if (Ret)
	{
		return Ret;
	}

	if (AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState))
	{
		if (UFortPlaylistAthena* Playlist = GameState->CurrentPlaylistInfo.BasePlaylist)
		{
			Ret = Playlist->GameData.Get();

			if (Ret == NULL)
			{
				FString AssetPathNameStr = UKismetStringLibrary::Conv_NameToString(Playlist->GameData.ObjectID.AssetPathName);

				Ret = StaticLoadObject<UCurveTable>(AssetPathNameStr.CStr());

				AssetPathNameStr.Free();
			}
		}

		if (Ret == NULL)
		{
			Ret = StaticLoadObject<UCurveTable>(TEXT("/Game/Balance/AthenaGameData.AthenaGameData"));
		}
	}

	return Ret;
}

template<typename T = AActor>
static T* SpawnActor(UClass* InClass, FVector Location, FRotator Rotation = FRotator{ 0, 0, 0 }, AActor* Owner = NULL)
{
	FTransform Transform{};
	Transform.Scale3D = FVector{ 1, 1, 1 };
	Transform.Translation = Location;
	Transform.Rotation = UKismetMathLibrary::Conv_RotatorToTransform(Rotation).Rotation;

	AActor* Actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), InClass, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, Owner);

	if (Actor)
		return (T*)UGameplayStatics::FinishSpawningActor(Actor, Transform);

	return NULL;
}

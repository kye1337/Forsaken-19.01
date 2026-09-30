#include "Kismet/KismetDetourLibrary.h"

void UKismetDetourLibrary::InitializeDetour_Internal(UObject* Object, void* Target, void* Detour, void** Original, EDetourType DetourType)
{
	switch (DetourType)
	{

	case VFT:
	{
		if (Original)
			*Original = Object->VTable[(int)Target];

		DWORD dwProt;
		VirtualProtect(&Object->VTable[(int)Target], sizeof(void*), PAGE_EXECUTE_READWRITE, &dwProt);

		Object->VTable[(int)Target] = Detour;

		DWORD dwTemp;
		VirtualProtect(&Object->VTable[(int)Target], sizeof(void*), dwProt, &dwTemp);

		break;
	}

	case MinHook:
	{
		MH_CreateHook((void*)((uintptr_t)InSDKUtils::GetImageBase() + (uintptr_t)Target), Detour, (void**)Original);

		break;
	}

	case Exec:
	{
		UFunction* Func = reinterpret_cast<UFunction*>(Target);

		if (!Func)
			break;

		UFunction::FNativeFuncPtr& ExecFunction = Func->ExecFunction;

		if (Original)
			*Original = ExecFunction;

		ExecFunction = (UFunction::FNativeFuncPtr)Detour;
		break;
	}
	
	case Rel32Lea:
	case Rel32:
	{
		uint64 InstructionAddr = InSDKUtils::GetImageBase() + (uint64)Target;

		auto Impl = (uint8*)(InstructionAddr);

		auto NearPage = AllocateNearbyPage(Impl);

		if (!NearPage)
		{
			return;
		}

		uint8_t Shellcode[] =
		{
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

		if (Detour != NULL)
		{
			memcpy(Shellcode + 6, &Detour, 8);
			memcpy(NearPage, Shellcode, sizeof(Shellcode));

			auto Offset = NearPage - (Impl + (int)DetourType + 4);

			memcpy(Impl + DetourType, &Offset, sizeof(int));
		}
		else
		{
			memset(Impl, 0x90, sizeof(int) + 1);
		}
	}
	}
}

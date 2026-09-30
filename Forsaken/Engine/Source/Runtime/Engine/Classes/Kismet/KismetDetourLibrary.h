#pragma once

#include "CoreGlobals.h"

enum EDetourType
{
    VFT,

    Rel32,

    Exec,

    Rel32Lea,

    MinHook
};

class UKismetDetourLibrary
{
public:
    template<typename Class = UObject>
    static void VirtualSwap(void** VTable, int Index, void* Detour, void** Original = nullptr)
    {
        if (Original)
            *Original = VTable[Index];

        DWORD dwProt;
        VirtualProtect(&VTable[Index], sizeof(void*), PAGE_EXECUTE_READWRITE, &dwProt);

        VTable[Index] = Detour;

        DWORD dwTemp;
        VirtualProtect(&VTable[Index], sizeof(void*), dwProt, &dwTemp);
    }
private:
    static uint8_t* AllocateNearbyPage(void* targetAddr)
    {
        SYSTEM_INFO SysInfo;
        GetSystemInfo(&SysInfo);

        const uint64_t PageSize = SysInfo.dwPageSize;
        const uint64_t StartAddr = (uint64_t(targetAddr) & ~(PageSize - 1));
        const uint64_t MinAddr = min(StartAddr - 0x7FFFFF00, (uint64_t)SysInfo.lpMinimumApplicationAddress);
        const uint64_t MaxAddr = max(StartAddr + 0x7FFFFF00, (uint64_t)SysInfo.lpMaximumApplicationAddress);
        const uint64_t StartPage = (StartAddr - (StartAddr % PageSize));

        for (uint64_t PageOffset = 1; PageOffset; PageOffset++)
        {
            uint64_t ByteOffset = PageOffset * PageSize;
            uint64_t HighAddr = StartPage + ByteOffset;
            uint64_t LowAddr = (StartPage > ByteOffset) ? StartPage - ByteOffset : 0;

            bool NeedsExit = HighAddr > MaxAddr && LowAddr < MinAddr;

            if (HighAddr < MaxAddr)
            {
                if (void* OutAddr = VirtualAlloc((void*)HighAddr, PageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
                {
                    return (uint8_t*)OutAddr;
                }
            }

            if (LowAddr > MinAddr)
            {
                if (void* OutAddr = VirtualAlloc((void*)LowAddr, PageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
                {
                    return (uint8_t*)OutAddr;
                }
            }

            if (NeedsExit)
            {
                break;
            }
        }

        return nullptr;
    }
public:

    template<typename Class = UObject, EDetourType DetourType, typename TargetType, typename OriginalType = void*>
    static void InitializeDetour(TargetType Target, void* Detour, OriginalType* Original = nullptr)
    {
        InitializeDetour_Internal(Class::GetDefaultObj(), (void*)Target, Detour, (void**)Original, DetourType);
    }
private:
    static void InitializeDetour_Internal(UObject* Object, void* Target, void* Detour, void** Original, EDetourType DetourType);
public:
    static void Setup()
    {
        MH_Initialize();
    }

    static void Finalize()
    {
        MH_EnableHook(MH_ALL_HOOKS);
    }
public:
    static bool ForceReturnTrue()
    {
        return true;
    }

    static bool ForceReturnFalse()
    {
        return false;
    }
};

#pragma once

#include "CoreGlobals.h"

enum MNEMONIC : uint8_t
{
    JMP_REL8 = 0xEB,
    JMP_REL32 = 0xE9,
    JMP_EAX = 0xE0,
    CALL = 0xE8,
    LEA = 0x8D,
    CDQ = 0x99,
    CMOVL = 0x4C,
    CMOVS = 0x48,
    CMOVNS = 0x49,
    NOP = 0x90,
    RET = 0xC3,
    INT3 = 0xCC,
    RETN_REL8 = 0xC2,
    RETN = 0xC3,
    POP = 0x58,
    MAXOP = 0x5F,
    CMOVNO = 0x41,
    NONE = 0x00,
    PUSH = 0x40,
    JE = 0x74,
    JMP_NOP = 0xE990
};

class UKismetMemoryLibrary
{
public:
    template<class T>
    static void Patch(uint64 Target, T Byte)
    {
        Target += InSDKUtils::GetImageBase();

        DWORD dwProt;
        VirtualProtect((PVOID)Target, sizeof(T), PAGE_EXECUTE_READWRITE, &dwProt);

        *(T*)(Target) = Byte;

        DWORD dwTemp;
        VirtualProtect((PVOID)Target, sizeof(T), dwProt, &dwTemp);
    }
};

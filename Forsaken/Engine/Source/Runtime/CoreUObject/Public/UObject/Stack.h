#pragma once

#include "CoreGlobals.h"

typedef uint32 CodeSkipSizeType;

typedef TArray<CodeSkipSizeType> FlowStackType;

class FOutputDevice
{
public:
	bool bSuppressEventTag;
	bool bAutoEmitLineTerminator;
};

#define RESULT_PARAM Z_Param__Result
#define RESULT_DECL void*const RESULT_PARAM

struct FFrame : public FOutputDevice
{
public:
	void** VFT;

	UFunction* Node;
	UObject* Object;
	uint8* Code;
	uint8* Locals;

	UProperty* MostRecentProperty;
	uint8* MostRecentPropertyAddress;
	//uint8* MostRecentPropertyContainer;

	FlowStackType FlowStack;

	FFrame* PreviousFrame;

	void* OutParms;

	UField* PropertyChainForCompiledIn;

	UFunction* CurrentNativeFunction;

	bool bArrayContextFailed;

	FORCEINLINE void StepExplicitProperty(void* const Result, void* Property)
	{
		static void (*StepExplicitPropertyOriginal)(__int64 frame, void* const Result, void* Property) = decltype(StepExplicitPropertyOriginal)(__int64(GetModuleHandleW(0)) + 0xb85914);
		StepExplicitPropertyOriginal(__int64(this), Result, Property);
	}

	FORCEINLINE void Step(UObject* Context, RESULT_DECL)
	{
		static void (*StepOriginal)(__int64 frame, UObject * Context, RESULT_DECL) = decltype(StepOriginal)(__int64(GetModuleHandleW(0)) + 0xb534ac);
		StepOriginal(__int64(this), Context, RESULT_PARAM);
	}

	FORCEINLINE void StepCompiledIn(void* const Result, bool bPrint = false) // https://github.com/EpicGames/UnrealEngine/blob/cdaec5b33ea5d332e51eee4e4866495c90442122/Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h#L444
	{
		if (Code)
		{
			Step(Object, Result);
		}
		else
		{
			UField* Property = PropertyChainForCompiledIn;
			PropertyChainForCompiledIn = Property->Next;
			StepExplicitProperty(Result, Property);
		}
	}

	template<typename TNativeType>
	FORCEINLINE TNativeType& StepCompiledInRef(void* const TemporaryBuffer)
	{
		MostRecentPropertyAddress = nullptr;

		if (Code)
		{
			Step(Object, TemporaryBuffer);
		}
		else
		{
			UField* Property = PropertyChainForCompiledIn;
			PropertyChainForCompiledIn = Property->Next;
			StepExplicitProperty(TemporaryBuffer, Property);
		}

		return (MostRecentPropertyAddress != NULL) ? *(TNativeType*)(MostRecentPropertyAddress) : *(TNativeType*)TemporaryBuffer;
	}
};

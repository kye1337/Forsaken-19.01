#pragma once

#include "CoreGlobals.h"

template<class T>
FORCEINLINE T* Cast(UObject* Src)
{
	return Src && (Src->IsA(T::StaticClass())) ? (T*)Src : NULL;
}

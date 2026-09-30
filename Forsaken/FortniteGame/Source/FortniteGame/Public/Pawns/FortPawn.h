#pragma once

#include "CoreGlobals.h"

class FortPawn
{
private:
	class Original
	{
	public:
		static inline void (*EmoteStopped)(UObject* Context, FFrame& Stack, void* Ret);
		static inline void (*MovingEmoteStopped)(UObject* Context, FFrame& Stack, void* Ret);
	};
private:
	static void EmoteStopped(UObject* Context, FFrame& Stack, void* Ret);
	static void MovingEmoteStopped(UObject* Context, FFrame& Stack, void* Ret);
public:
	static void Setup();
};

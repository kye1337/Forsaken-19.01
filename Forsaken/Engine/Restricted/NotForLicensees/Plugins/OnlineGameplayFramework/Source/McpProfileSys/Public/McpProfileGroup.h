#pragma once

#include "CoreGlobals.h"

class McpProfileGroup
{
private:
	class Original
	{
	public:
		static inline void (*SendRequestNow)(UMcpProfileGroup* ProfileGroup, const void* HttpRequest, FBaseUrlContext::EContextCredentials ContextCredentials);
	};
private:
	static void SendRequestNow(UMcpProfileGroup* ProfileGroup, const void* HttpRequest, FBaseUrlContext::EContextCredentials ContextCredentials);
public:
	static void Setup();
};

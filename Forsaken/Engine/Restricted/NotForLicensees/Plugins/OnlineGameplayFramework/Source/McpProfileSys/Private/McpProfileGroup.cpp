#include "Engine/Restricted/NotForLicensees/Plugins/OnlineGameplayFramework/Source/McpProfileSys/Public/McpProfileGroup.h"

void McpProfileGroup::SendRequestNow(UMcpProfileGroup* ProfileGroup, const void* HttpRequest, FBaseUrlContext::EContextCredentials ContextCredentials)
{
	Original::SendRequestNow(ProfileGroup, HttpRequest, FBaseUrlContext::CXC_Public);
}

void McpProfileGroup::Setup()
{
	UKismetDetourLibrary::InitializeDetour<UMcpProfileGroup, MinHook>(0xF54B24, SendRequestNow, &Original::SendRequestNow);
}

#pragma once

#include "CoreGlobals.h"

class ConversationRegistry
{
public:
	static TArray<FGuid> GetEntryPointGUIDs(UConversationRegistry* ConversationRegistry, FGameplayTag EntryPoint);
	static TArray<FGuid> GetOutputLinkGUIDs(UConversationRegistry* ConversationRegistry, FGameplayTag EntryPoint);
	static TArray<FGuid> GetOutputLinkGUIDs(UConversationRegistry* ConversationRegistry, const FGuid& SourceGUID);
	static TArray<FGuid> GetOutputLinkGUIDs(UConversationRegistry* ConversationRegistry, const TArray<FGuid>& SourceGUIDs);
};
#pragma once

#include "CoreGlobals.h"

class ConversationLibrary
{
public:
	static UConversationInstance* StartConversation(FGameplayTag ConversationEntryTag, AActor* Instigator, FGameplayTag InstigatorTag, AActor* Target, FGameplayTag TargetTag);
};

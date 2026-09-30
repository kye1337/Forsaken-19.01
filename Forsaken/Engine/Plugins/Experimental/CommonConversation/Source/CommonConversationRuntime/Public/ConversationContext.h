#pragma once

#include "CoreGlobals.h"

class ConversationContextHelpers
{
public:
	static void MakeConversationParticipant(const FConversationContext& Context, AActor* ParticipantActor, FGameplayTag ParticipantTag);
};

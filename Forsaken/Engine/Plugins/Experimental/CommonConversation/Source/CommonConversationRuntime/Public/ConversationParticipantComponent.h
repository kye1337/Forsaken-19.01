#pragma once

#include "CoreGlobals.h"

class ConversationParticipantComponent
{
public:
	static void ServerNotifyConversationStarted(UConversationParticipantComponent* ParticipantComponent, UConversationInstance* Conversation, FGameplayTag AsParticipant);
	static void ServerNotifyConversationEnded(UConversationParticipantComponent* ParticipantComponent, UConversationInstance* Conversation);
};

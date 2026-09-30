#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationContext.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationInstance.h"

void ConversationContextHelpers::MakeConversationParticipant(const FConversationContext& Context, AActor* ParticipantActor, FGameplayTag ParticipantTag)
{
	if (ParticipantActor == NULL)
		return;

	if (!ParticipantTag.TagName.ComparisonIndex)
		return;

	if (UConversationInstance* Conversation = Context.ActiveConversation)
	{
		ConversationInstance::ServerAssignParticipant(Conversation, ParticipantTag, ParticipantActor);
	}
}

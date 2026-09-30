#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationParticipantComponent.h"

void ConversationParticipantComponent::ServerNotifyConversationStarted(UConversationParticipantComponent* ParticipantComponent, UConversationInstance* Conversation, FGameplayTag AsParticipant)
{
	AActor* Owner = ParticipantComponent->GetOwner();

	if (Owner->GetLocalRole() == ENetRole::ROLE_Authority)
	{
		ParticipantComponent->Auth_CurrentConversation = Conversation;
		ParticipantComponent->Auth_Conversations.Add(Conversation);

		ParticipantComponent->ClientUpdateParticipants(ParticipantComponent->Auth_CurrentConversation->Participants);

		ParticipantComponent->ConversationsActive++;

		ParticipantComponent->ClientStartConversation(AsParticipant);

		if (Owner->GetRemoteRole() == ENetRole::ROLE_AutonomousProxy)
		{
			ParticipantComponent->ClientUpdateConversations(ParticipantComponent->ConversationsActive);
		}
	}
}

void ConversationParticipantComponent::ServerNotifyConversationEnded(UConversationParticipantComponent* ParticipantComponent, UConversationInstance* Conversation)
{
	AActor* Owner = ParticipantComponent->GetOwner();

	if (Owner->GetLocalRole() == ENetRole::ROLE_Authority)
	{
		if (ParticipantComponent->Auth_CurrentConversation == Conversation)
		{
			ParticipantComponent->Auth_CurrentConversation = NULL;

			for (int32 i = 0; i < ParticipantComponent->Auth_Conversations.Num(); ++i)
			{
				UConversationInstance* ConversationInstance = ParticipantComponent->Auth_Conversations[i];

				if (ConversationInstance == Conversation)
				{
					ParticipantComponent->Auth_Conversations.Remove(i);
					break;
				}
			}

			ParticipantComponent->ConversationsActive--;

			if (Owner->GetRemoteRole() == ENetRole::ROLE_AutonomousProxy)
			{
				ParticipantComponent->ClientUpdateConversations(ParticipantComponent->ConversationsActive);
			}
		}
	}
}

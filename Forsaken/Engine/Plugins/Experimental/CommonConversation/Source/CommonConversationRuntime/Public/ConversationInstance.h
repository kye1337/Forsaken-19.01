#pragma once

#include "CoreGlobals.h"

class ConversationInstance
{
public:
	static void ServerRemoveParticipant(UConversationInstance* Conversation, FGameplayTag ParticipantID);
	static void ServerAssignParticipant(UConversationInstance* Conversation, FGameplayTag ParticipantID, AActor* ParticipantActor);
	static void ServerStartConversation(UConversationInstance* Conversation, FGameplayTag EntryPoint);

	static void TryStartingConversation(UConversationInstance* Conversation);

	static void ServerAdvanceConversation(UConversationInstance* Conversation, const FAdvanceConversationRequest& InChoicePicked);
	static void ServerAbortConversation(UConversationInstance* Conversation);
public:
	static void PauseConversationAndSendClientChoices(UConversationInstance* Conversation, const FConversationContext& Context, const FClientConversationMessage& ClientMessage);
	static void ReturnToLastClientChoice(UConversationInstance* Conversation, const FConversationContext& Context);
	static void ReturnToCurrentClientChoice(UConversationInstance* Conversation, const FConversationContext& Context);
	static void ReturnToStart(UConversationInstance* Conversation, const FConversationContext& Context);

	static void ResetConversationProgress(UConversationInstance* Conversation);
	static void UpdateNextChoices(UConversationInstance* Conversation, FConversationContext& Context);
	static void SetNextChoices(UConversationInstance* Conversation, const TArray<FConversationBranchPoint>& InAllChoices);
	static FConversationBranchPoint* FindBranchPointFromClientChoice(UConversationInstance* Conversation, const FConversationChoiceReference& InChoice);
	static TArray<FGuid> DetermineBranches(UConversationInstance* Conversation, const TArray<FGuid>& SourceList, EConversationRequirementResult MaximumRequirementResult);
	static void ModifyCurrentConversationNode(UConversationInstance* Conversation, const FConversationChoiceReference& NewChoice);
	static void ModifyCurrentConversationNode(UConversationInstance* Conversation, const FConversationBranchPoint& NewBranchPoint);
	static void OnCurrentConversationNodeModified(UConversationInstance* Conversation);
};

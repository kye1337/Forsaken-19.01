#pragma once

#include "CoreGlobals.h"
#include "ConversationTypes.h"

class ConversationTaskNode
{
public:
	static FConversationTaskResult ExecuteTaskNode(UConversationTaskNode* TaskNode, const FConversationContext& Context);
	static FConversationTaskResult ExecuteTaskNodeWithSideEffects(UConversationTaskNode* TaskNode, const FConversationContext& InContext);
	static void GenerateChoicesForDestinations(FConversationBranchPointBuilder& BranchBuilder, FConversationContext& InContext, const TArray<FGuid>& CandidateDestinations);
	static void GatherStaticChoices(UConversationTaskNode* TaskNode, FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& InContext);
};

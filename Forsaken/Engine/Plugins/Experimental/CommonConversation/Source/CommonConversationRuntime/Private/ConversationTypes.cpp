#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTypes.h"

void FConversationBranchPointBuilder::AddChoice(const FConversationContext& InContext, FClientConversationOptionEntry&& InChoice)
{
	FConversationBranchPoint BranchPoint{};
	
	BranchPoint.ClientChoice = InChoice;

	if (!UKismetGuidLibrary::IsValid_Guid(BranchPoint.ClientChoice.ChoiceReference.NodeReference.NodeGUID))
	{
		BranchPoint.ClientChoice.ChoiceReference.NodeReference.NodeGUID = InContext.TaskBeingConsidered->Compiled_NodeGUID;
	}

	BranchPoint.ReturnScopeStack = InContext.ReturnScopeStack;

	BranchPoints.Add(BranchPoint);
}

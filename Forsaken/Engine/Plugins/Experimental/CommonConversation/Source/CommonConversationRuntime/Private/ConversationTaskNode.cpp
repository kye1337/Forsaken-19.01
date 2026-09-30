#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTaskNode.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationInstance.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationRegistry.h"

FConversationTaskResult ConversationTaskNode::ExecuteTaskNode(UConversationTaskNode* TaskNode, const FConversationContext& Context)
{
	FConversationTaskResult Result = TaskNode->ExecuteTaskNode(Context);

	std::cout << TaskNode->Class->Super->GetFullName() << std::endl;

	if (UFortConversationTaskNode_Speech* SpeechNode = Cast<UFortConversationTaskNode_Speech>(TaskNode))
	{
		//Result.Type = EConversationTaskResultType::PauseConversationAndSendClientChoices;
	}

	return Result;
}

FConversationTaskResult ConversationTaskNode::ExecuteTaskNodeWithSideEffects(UConversationTaskNode* TaskNode, const FConversationContext& InContext)
{
	if (InContext.TaskBeingConsidered != TaskNode)
		return FConversationTaskResult();

	TaskNode->EvalWorldContextObj = UWorld::GetWorld();

	FConversationTaskResult Result = FConversationTaskResult();

	if (InContext.bServer)
	{
		Result = ExecuteTaskNode(TaskNode, InContext);

		if (Result.Type != EConversationTaskResultType::Invalid && Result.Type != EConversationTaskResultType::AbortConversation)
		{
			for (UConversationNode* SubNode : TaskNode->SubNodes)
			{
				if (UConversationSideEffectNode* SideEffectNode = Cast<UConversationSideEffectNode>(SubNode))
				{
					SideEffectNode->ServerCauseSideEffect(InContext);
				}
			}

			FConversationParticipants Participants = InContext.ActiveConversation->Participants;

			for (const FConversationParticipantEntry& ParticipantEntry : Participants.List)
			{
				if (UConversationParticipantComponent* Component = GetComponentByClass<UConversationParticipantComponent>(ParticipantEntry.Actor))
				{
					if (Component->GetOwner()->GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
					{
						Component->ClientExecuteTaskAndSideEffects(InContext.ActiveConversation->CurrentBranchPoint.ClientChoice.ChoiceReference.NodeReference);
					}
				}
			}
		}
	}
	else
	{
		TaskNode->ExecuteClientEffects(InContext);

		for (UConversationSubNode* SubNode : TaskNode->SubNodes)
		{
			if (UConversationSideEffectNode* SideEffect = Cast<UConversationSideEffectNode>(SubNode))
			{
				SideEffect->ClientCauseSideEffect(InContext);
			}
		}
	}	

	return Result;
}

void ConversationTaskNode::GenerateChoicesForDestinations(FConversationBranchPointBuilder& BranchBuilder, FConversationContext& InContext, const TArray<FGuid>& CandidateDestinations)
{
	if (!InContext.bServer)
		return;

	UWorld* World = UWorld::GetWorld();

	for (const FGuid& DestinationGUID : CandidateDestinations)
	{
		if (UConversationTaskNode* DestinationTaskNode = Cast<UConversationTaskNode>(InContext.ConversationRegistry->GetRuntimeNodeFromGUID(DestinationGUID)))
		{
			DestinationTaskNode->EvalWorldContextObj = World;

			FConversationContext DestinationContext = InContext.CreateChildContext(DestinationTaskNode);

			const int32 StartingNumber = BranchBuilder.Num();

			static void (*GatherStaticChoices)(UConversationTaskNode * Node, FConversationBranchPointBuilder&, const FConversationContext&) =
				decltype(GatherStaticChoices)(InSDKUtils::GetImageBase() + 0x513E3D0);


			GatherStaticChoices(DestinationTaskNode, BranchBuilder, DestinationContext);

			if (BranchBuilder.Num() == StartingNumber)
			{
				const EConversationRequirementResult RequirementResult = DestinationTaskNode->CheckRequirements(InContext);

				if (RequirementResult == EConversationRequirementResult::Passed)
				{
					FClientConversationOptionEntry DefaultChoice{};
					DefaultChoice.ChoiceReference.NodeReference.NodeGUID = DestinationGUID;
					DefaultChoice.ChoiceType = EConversationChoiceType::ServerOnly;
					BranchBuilder.AddChoice(DestinationContext, (FClientConversationOptionEntry&&)DefaultChoice);
				}
			}
		}
	}
}

void ConversationTaskNode::GatherStaticChoices(UConversationTaskNode* TaskNode, FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& InContext)
{
	if (TaskNode->IsA(UConversationLinkNode::StaticClass()))
	{
		TArray<FGuid> PotentialStartingPoints = ConversationRegistry::GetOutputLinkGUIDs(InContext.ConversationRegistry, { ((UConversationLinkNode*)TaskNode)->RemoteEntryTag });

		if (PotentialStartingPoints.Num() > 0)
		{
			TArray<FGuid> LegalStartingPoints = ConversationInstance::DetermineBranches(InContext.ActiveConversation, PotentialStartingPoints, EConversationRequirementResult::FailedButVisible);
			FConversationContext ReturnScopeContext = InContext;
			ReturnScopeContext.ReturnScopeStack.Add({ TaskNode->Compiled_NodeGUID });
			GenerateChoicesForDestinations(BranchBuilder, ReturnScopeContext, LegalStartingPoints);
		}
		return;
	}

	GatherStaticChoices(TaskNode, BranchBuilder, InContext);
}

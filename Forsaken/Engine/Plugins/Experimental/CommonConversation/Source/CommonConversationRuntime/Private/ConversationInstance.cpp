#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationInstance.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationParticipantComponent.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationRegistry.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTaskNode.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTypes.h"

void ConversationInstance::ServerRemoveParticipant(UConversationInstance* Conversation, FGameplayTag ParticipantID)
{
	for (int32 i = 0; i < Conversation->Participants.List.Num(); ++i)
	{
		FConversationParticipantEntry& It = Conversation->Participants.List[i];

		if (It.ParticipantID.TagName.ComparisonIndex == ParticipantID.TagName.ComparisonIndex)
		{
			if (UConversationParticipantComponent* OldParticipant = GetComponentByClass<UConversationParticipantComponent>(It.Actor))
			{
				if (Conversation->bConversationStarted)
				{
					ConversationParticipantComponent::ServerNotifyConversationEnded(OldParticipant, Conversation);
				}
			}

			Conversation->Participants.List.Remove(i);
			break;
		}
	}
}

void ConversationInstance::ServerAssignParticipant(UConversationInstance* Conversation, FGameplayTag ParticipantID, AActor* ParticipantActor)
{
	if (!ParticipantID.TagName.ComparisonIndex || (ParticipantActor == NULL))
		return;

	ServerRemoveParticipant(Conversation, ParticipantID);

	FConversationParticipantEntry NewEntry{};
	NewEntry.ParticipantID = ParticipantID;
	NewEntry.Actor = ParticipantActor;
	Conversation->Participants.List.Add(NewEntry);

	if (Conversation->bConversationStarted)
	{
		if (UConversationParticipantComponent* ParticipantComponent = GetComponentByClass<UConversationParticipantComponent>(NewEntry.Actor))
		{
			ConversationParticipantComponent::ServerNotifyConversationStarted(ParticipantComponent, Conversation, ParticipantID);
		}
	}
}

void ConversationInstance::ServerStartConversation(UConversationInstance* Conversation, FGameplayTag EntryPoint)
{
	ResetConversationProgress(Conversation);
	Conversation->StartingEntryGameplayTag = EntryPoint;

	UConversationRegistry* ConversationRegistry = UConversationRegistry::GetFromWorld(UWorld::GetWorld());

	TArray<FGuid> PotentialStartingPoints = ConversationRegistry::GetOutputLinkGUIDs(ConversationRegistry, EntryPoint);

	if (PotentialStartingPoints.Num() == 0)
	{
		ServerAbortConversation(Conversation);
		return;
	}
	else
	{
		TArray<FGuid> LegalStartingPoints = DetermineBranches(Conversation, PotentialStartingPoints, EConversationRequirementResult::FailedButVisible);

		if (LegalStartingPoints.Num() == 0)
		{
			ServerAbortConversation(Conversation);
			return;
		}
		else
		{
			const int32 StartingIndex = UKismetMathLibrary::RandomIntegerInRange(0, LegalStartingPoints.Num() - 1);

			FConversationBranchPoint StartingPoint;
			StartingPoint.ClientChoice.ChoiceReference.NodeReference.NodeGUID = LegalStartingPoints[StartingIndex];

			Conversation->StartingBranchPoint = StartingPoint;
			Conversation->CurrentBranchPoint = StartingPoint;
		}
	}

	for (const FConversationParticipantEntry& ParticipantEntry : Conversation->Participants.List)
	{
		if (UConversationParticipantComponent* ParticipantComponent = GetComponentByClass<UConversationParticipantComponent>(ParticipantEntry.Actor))
		{
			ConversationParticipantComponent::ServerNotifyConversationStarted(ParticipantComponent, Conversation, ParticipantEntry.ParticipantID);
		}
	}

	Conversation->bConversationStarted = true;
	//broadcast

	TryStartingConversation(Conversation);
}

void ConversationInstance::TryStartingConversation(UConversationInstance* Conversation)
{
	if (!Conversation->bConversationStarted)
		return;

	OnCurrentConversationNodeModified(Conversation);
}

void ConversationInstance::ServerAdvanceConversation(UConversationInstance* Conversation, const FAdvanceConversationRequest& InChoicePicked)
{
	if (Conversation->bConversationStarted && UKismetGuidLibrary::IsValid_Guid(Conversation->CurrentBranchPoint.ClientChoice.ChoiceReference.NodeReference.NodeGUID))
	{
		TArray<FConversationBranchPoint> CandidateDestinations;

		if (UKismetGuidLibrary::IsValid_Guid(InChoicePicked.Choice.NodeReference.NodeGUID))
		{
			if (FConversationBranchPoint* BranchPoint = FindBranchPointFromClientChoice(Conversation, InChoicePicked.Choice))
			{
				CandidateDestinations.Add(*BranchPoint);
			}
			else
			{
				ServerAbortConversation(Conversation);
				return;
			}
		}
		else
		{
			if (Conversation->CurrentBranchPoints.Num() == 0 && Conversation->ScopeStack.Num() > 0)
			{
				ModifyCurrentConversationNode(Conversation, Conversation->ScopeStack[Conversation->ScopeStack.Num() - 1]);
				return;
			}

			for (const FConversationBranchPoint& BranchPoint : Conversation->CurrentBranchPoints)
			{
				if (BranchPoint.ClientChoice.ChoiceType != EConversationChoiceType::UserChoiceUnavailable)
				{
					CandidateDestinations.Add(BranchPoint);
				}
			}
		}

		TArray<FConversationBranchPoint> ValidDestinations;
		{
			FConversationContext Context = FConversationContext::CreateServerContext(Conversation, NULL);

			for (FConversationBranchPoint& BranchPoint : CandidateDestinations)
			{
				if (UConversationTaskNode* TaskNode = Cast<UConversationTaskNode>(BranchPoint.ClientChoice.ChoiceReference.NodeReference.TryToResolve(Context)))
				{
					EConversationRequirementResult Result = TaskNode->bIgnoreRequirementsWhileAdvancingConversations ? EConversationRequirementResult::Passed : TaskNode->CheckRequirements(Context);

					if (Result == EConversationRequirementResult::Passed)
					{
						ValidDestinations.Add(BranchPoint);
					}
				}
				else
				{
					ValidDestinations.Add(BranchPoint);
				}
			}
		}

		if (ValidDestinations.Num() == 0)
		{
			ServerAbortConversation(Conversation);
			return;
		}
		else
		{
			const FConversationChoiceReference& PerviousNode = Conversation->CurrentBranchPoint.ClientChoice.ChoiceReference;
			const int32 StartingIndex = UKismetMathLibrary::RandomIntegerInRange(0, ValidDestinations.Num() - 1);
			const FConversationBranchPoint& TargetChoice = ValidDestinations[StartingIndex];

			ModifyCurrentConversationNode(Conversation, TargetChoice);
		}
	}
}

void ConversationInstance::ServerAbortConversation(UConversationInstance* Conversation)
{
	if (Conversation->bConversationStarted)
	{
		TArray<FConversationParticipantEntry> ListCopy = Conversation->Participants.List;

		for (const FConversationParticipantEntry& ParticipantEntry : ListCopy)
		{
			ServerRemoveParticipant(Conversation, ParticipantEntry.ParticipantID);
		}
	}

	ResetConversationProgress(Conversation);

	Conversation->bConversationStarted = false;
}

void ConversationInstance::PauseConversationAndSendClientChoices(UConversationInstance* Conversation, const FConversationContext& Context, const FClientConversationMessage& ClientMessage)
{
	FClientConversationMessagePayload LastMessage = FClientConversationMessagePayload();
	LastMessage.Message = ClientMessage;
	LastMessage.Options = Conversation->CurrentUserChoices;
	LastMessage.CurrentNode = Context.ActiveConversation->CurrentBranchPoint.ClientChoice.ChoiceReference.NodeReference;
	LastMessage.Participants = Conversation->Participants;

	Conversation->ClientBranchPoints.Add(UConversationInstance::FCheckpoint(Conversation->CurrentBranchPoint, Conversation->ScopeStack));

	for (const FConversationParticipantEntry& KVP : LastMessage.Participants.List)
	{
		if (UConversationParticipantComponent* ParticipantComponent = GetComponentByClass<UConversationParticipantComponent>(KVP.Actor))
		{
			//ParticipantComponent->Lastmes

			ParticipantComponent->ClientUpdateConversation(LastMessage);
		}
	}
}

void ConversationInstance::ReturnToLastClientChoice(UConversationInstance* Conversation, const FConversationContext& Context)
{
	if (Conversation->ClientBranchPoints.Num() > 1)
	{
		Conversation->ClientBranchPoints.Remove(Conversation->ClientBranchPoints.Num() - 1);

		const UConversationInstance::FCheckpoint& Checkpont = Conversation->ClientBranchPoints[Conversation->ClientBranchPoints.Num() - 1];
		Conversation->ScopeStack = Checkpont.ScopeStack;
		ModifyCurrentConversationNode(Conversation, Checkpont.ClientBranchPoint);
	}
}

void ConversationInstance::ReturnToCurrentClientChoice(UConversationInstance* Conversation, const FConversationContext& Context)
{
	if (Conversation->ClientBranchPoints.Num() > 0)
	{
		const UConversationInstance::FCheckpoint Checkpoint = Conversation->ClientBranchPoints[Conversation->ClientBranchPoints.Num() - 1];

		Conversation->ClientBranchPoints.Remove(Conversation->ClientBranchPoints.Num() - 1);

		Conversation->ScopeStack = Checkpoint.ScopeStack;
		ModifyCurrentConversationNode(Conversation, Checkpoint.ClientBranchPoint);
	}
}

void ConversationInstance::ReturnToStart(UConversationInstance* Conversation, const FConversationContext& Context)
{
	FGameplayTag EntryStartPointGameplayTagCache = Conversation->StartingEntryGameplayTag;
	FConversationBranchPoint StartingBranchPointCache = Conversation->StartingBranchPoint;

	ResetConversationProgress(Conversation);

	Conversation->StartingEntryGameplayTag = EntryStartPointGameplayTagCache;
	Conversation->StartingBranchPoint = StartingBranchPointCache;

	ModifyCurrentConversationNode(Conversation, Conversation->StartingBranchPoint);
}

void ConversationInstance::ResetConversationProgress(UConversationInstance* Conversation)
{
	Conversation->StartingEntryGameplayTag = FGameplayTag();
	Conversation->StartingBranchPoint = FConversationBranchPoint();
	Conversation->CurrentBranchPoint = FConversationBranchPoint();
	Conversation->CurrentBranchPoints.Reset();
	Conversation->ClientBranchPoints.Reset();
	Conversation->CurrentUserChoices.Reset();
}

void ConversationInstance::UpdateNextChoices(UConversationInstance* Conversation, FConversationContext& Context)
{
	TArray<FConversationBranchPoint> AllChoices;

	if (UConversationTaskNode* TaskNode = Cast<UConversationTaskNode>(Conversation->CurrentBranchPoint.ClientChoice.ChoiceReference.NodeReference.TryToResolve(Context)))
	{
		FConversationContext ChoiceContext = Context.CreateChildContext(TaskNode);

		const TArray<FGuid> CandidateDestinations = ConversationRegistry::GetOutputLinkGUIDs(ChoiceContext.ConversationRegistry, Conversation->CurrentBranchPoint.ClientChoice.ChoiceReference.NodeReference.NodeGUID);

		FConversationBranchPointBuilder BranchBuilder;
		ConversationTaskNode::GenerateChoicesForDestinations(BranchBuilder, ChoiceContext, CandidateDestinations);
		AllChoices = BranchBuilder.GetBranches();
	}

	SetNextChoices(Conversation, AllChoices);
}

void ConversationInstance::SetNextChoices(UConversationInstance* Conversation, const TArray<FConversationBranchPoint>& InAllChoices)
{
	Conversation->CurrentUserChoices.Reset();
	Conversation->CurrentBranchPoints = InAllChoices;

	for (const FConversationBranchPoint& UserBranchPoint : Conversation->CurrentBranchPoints)
	{
		if (UserBranchPoint.ClientChoice.ChoiceType != EConversationChoiceType::ServerOnly)
		{
			Conversation->CurrentUserChoices.Add(UserBranchPoint.ClientChoice);
		}
	}

	if (Conversation->CurrentBranchPoints.Num() > 0 || Conversation->ScopeStack.Num() > 0)
	{
		if (Conversation->CurrentUserChoices.Num() == 0)
		{
			FClientConversationOptionEntry DefaultChoice{};
			DefaultChoice.ChoiceReference = FConversationChoiceReference();
			DefaultChoice.ChoiceText = UKismetTextLibrary::Conv_StringToText(L"Continue");
			DefaultChoice.ChoiceType = EConversationChoiceType::UserChoiceAvailable;
			Conversation->CurrentUserChoices.Add(DefaultChoice);
		}
	}
}

FConversationBranchPoint* ConversationInstance::FindBranchPointFromClientChoice(UConversationInstance* Conversation, const FConversationChoiceReference& InChoice)
{
	for (FConversationBranchPoint& BranchPoint : Conversation->CurrentBranchPoints)
	{
		if (BranchPoint.ClientChoice.ChoiceType == EConversationChoiceType::ServerOnly)
			continue;

		if (BranchPoint.ClientChoice.ChoiceReference.NodeReference.NodeGUID != InChoice.NodeReference.NodeGUID)
			continue;

		return &BranchPoint;
	}

	return NULL;
}

TArray<FGuid> ConversationInstance::DetermineBranches(UConversationInstance* Conversation, const TArray<FGuid>& SourceList, EConversationRequirementResult MaximumRequirementResult)
{
	FConversationContext Context = FConversationContext::CreateServerContext(Conversation, NULL);

	TArray<FGuid> EnabledPaths;

	for (const FGuid& TestGUID : SourceList)
	{
		UConversationNode* TestNode = Context.ConversationRegistry->GetRuntimeNodeFromGUID(TestGUID);

		if (UConversationTaskNode* TaskNode = Cast<UConversationTaskNode>(TestNode))
		{
			const EConversationRequirementResult RequirementResult = TaskNode->CheckRequirements(Context);

			if ((int64)RequirementResult <= (int64)MaximumRequirementResult)
			{
				EnabledPaths.Add(TestGUID);
			}
		}
	}

	return EnabledPaths;
}

void ConversationInstance::ModifyCurrentConversationNode(UConversationInstance* Conversation, const FConversationChoiceReference& NewChoice)
{
	FConversationBranchPoint BranchPoint{};
	BranchPoint.ClientChoice.ChoiceReference = NewChoice;

	ModifyCurrentConversationNode(Conversation, BranchPoint);
}

void ConversationInstance::ModifyCurrentConversationNode(UConversationInstance* Conversation, const FConversationBranchPoint& NewBranchPoint)
{
	Conversation->CurrentBranchPoint = NewBranchPoint;

	for (const FConversationNodeHandle& NodeHandle : NewBranchPoint.ReturnScopeStack)
	{
		FConversationChoiceReference ChoiceReference{};
		ChoiceReference.NodeReference = NodeHandle;

		Conversation->ScopeStack.Add(ChoiceReference);
	}

	OnCurrentConversationNodeModified(Conversation);
}

void ConversationInstance::OnCurrentConversationNodeModified(UConversationInstance* Conversation)
{
	if (!UKismetGuidLibrary::IsValid_Guid(Conversation->CurrentBranchPoint.ClientChoice.ChoiceReference.NodeReference.NodeGUID))
		return;

	FConversationContext AnonContext = FConversationContext::CreateServerContext(Conversation, NULL);
	UConversationNode* CurrentNode = Conversation->CurrentBranchPoint.ClientChoice.ChoiceReference.NodeReference.TryToResolve(AnonContext);

	if (UConversationTaskNode* TaskNode = Cast<UConversationTaskNode>(CurrentNode))
	{
		FConversationContext Context = AnonContext.CreateChildContext(TaskNode);

		const FConversationTaskResult TaskResult = ConversationTaskNode::ExecuteTaskNodeWithSideEffects(TaskNode, Context);

		if (Conversation->ScopeStack.Num() > 0 && Conversation->ScopeStack[Conversation->ScopeStack.Num() - 1].NodeReference.NodeGUID == TaskNode->Compiled_NodeGUID)
		{
			Conversation->ScopeStack.Remove(Conversation->ScopeStack.Num() - 1);
		}

		UpdateNextChoices(Conversation, Context);

		if (TaskResult.Type == EConversationTaskResultType::AbortConversation)
		{
			ServerAbortConversation(Conversation);
		}
		else if (TaskResult.Type == EConversationTaskResultType::AdvanceConversation)
		{
			ServerAdvanceConversation(Conversation, FAdvanceConversationRequest());
		}
		else if (TaskResult.Type == EConversationTaskResultType::AdvanceConversationWithChoice)
		{
			ModifyCurrentConversationNode(Conversation, TaskResult.AdvanceToChoice.Choice);
		}
		else if (TaskResult.Type == EConversationTaskResultType::PauseConversationAndSendClientChoices)
		{
			PauseConversationAndSendClientChoices(Conversation, Context, TaskResult.Message);
		}
		else if (TaskResult.Type == EConversationTaskResultType::ReturnToLastClientChoice)
		{
			ReturnToLastClientChoice(Conversation, Context);
		}
		else if (TaskResult.Type == EConversationTaskResultType::ReturnToCurrentClientChoice)
		{
			ReturnToCurrentClientChoice(Conversation, Context);
		}
		else if (TaskResult.Type == EConversationTaskResultType::ReturnToConversationStart)
		{
			ReturnToStart(Conversation, Context);
		}
	}
	else
	{
		ServerAbortConversation(Conversation);
	}
}

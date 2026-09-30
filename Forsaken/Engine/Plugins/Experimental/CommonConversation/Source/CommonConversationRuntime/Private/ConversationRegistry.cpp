#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationRegistry.h"

TArray<FGuid> ConversationRegistry::GetEntryPointGUIDs(UConversationRegistry* ConversationRegistry, FGameplayTag EntryPoint)
{
	const_cast<UConversationRegistry*>(ConversationRegistry)->BuildDependenciesGraph();

	for (const TPair<FGameplayTag, TArray<FGuid>>& Pair : ConversationRegistry->EntryTagToEntryList)
	{
		if (Pair.Key().TagName.ComparisonIndex == EntryPoint.TagName.ComparisonIndex)
			return Pair.Value();
	}

	return TArray<FGuid>();
}

TArray<FGuid> ConversationRegistry::GetOutputLinkGUIDs(UConversationRegistry* ConversationRegistry, FGameplayTag EntryPoint)
{
	TArray<FGuid> SourceGUIDs = GetEntryPointGUIDs(ConversationRegistry, EntryPoint);
	return GetOutputLinkGUIDs(ConversationRegistry, SourceGUIDs);
}

TArray<FGuid> ConversationRegistry::GetOutputLinkGUIDs(UConversationRegistry* ConversationRegistry, const FGuid& SourceGUID)
{
	TArray<FGuid> SourceArray;
	SourceArray.Add(SourceGUID);

	return GetOutputLinkGUIDs(ConversationRegistry, SourceArray);
}

TArray<FGuid> ConversationRegistry::GetOutputLinkGUIDs(UConversationRegistry* ConversationRegistry, const TArray<FGuid>& SourceGUIDs)
{
	const_cast<UConversationRegistry*>(ConversationRegistry)->BuildDependenciesGraph();

	TArray<FGuid> Result;

	for (const FGuid& SourceGUID : SourceGUIDs)
	{
		if (const UConversationDatabase* SourceConversation = ConversationRegistry->GetConversationFromNodeGUID(SourceGUID))
		{
			UConversationNode* SourceNode = NULL;

			for (const TPair<FGuid, UConversationNode*>& Pair : SourceConversation->ReachableNodeMap)
			{
				if (Pair.Key() == SourceGUID)
				{
					SourceNode = Pair.Value();
					break;
				}
			}

			if (SourceNode == NULL)
				continue;

			if (UConversationNodeWithLinks* SourceNodeWithLinks = Cast<UConversationNodeWithLinks>(SourceNode))
			{
				for (FGuid& OutputConnection : SourceNodeWithLinks->OutputConnections)
				{
					Result.Add(OutputConnection);
				}
			}
		}
	}

	return Result;
}

#pragma once

#include "CoreGlobals.h"

struct FConversationBranchPointBuilder
{
	void AddChoice(const FConversationContext& InContext, FClientConversationOptionEntry&& InChoice);

	int32 Num() const { return BranchPoints.Num(); }
	const TArray<FConversationBranchPoint>& GetBranches() const { return BranchPoints; }

private:
	TArray<FConversationBranchPoint> BranchPoints;
};

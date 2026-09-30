#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationLibrary.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationContext.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationInstance.h"

UConversationInstance* ConversationLibrary::StartConversation(FGameplayTag ConversationEntryTag, AActor* Instigator, FGameplayTag InstigatorTag, AActor* Target, FGameplayTag TargetTag)
{
	if (Instigator == NULL || Target == NULL)
		return NULL;

	if (UWorld* World = UWorld::GetWorld())
	{
		UClass* InstanceClass = UConversationSettings::GetDefaultObj()->ConversationInstanceClass.Get();

		if (InstanceClass == NULL)
			InstanceClass = UConversationInstance::StaticClass();

		UConversationInstance* ConversationInstance = NewObject<UConversationInstance>(World, InstanceClass);

		if (ConversationInstance)
		{
			FConversationContext Context = FConversationContext::CreateServerContext(ConversationInstance, NULL);

			ConversationContextHelpers::MakeConversationParticipant(Context, Target, TargetTag);
			ConversationContextHelpers::MakeConversationParticipant(Context, Instigator, InstigatorTag);

			ConversationInstance::ServerStartConversation(ConversationInstance, ConversationEntryTag);
		}

		return ConversationInstance;
	}

	return NULL;
}

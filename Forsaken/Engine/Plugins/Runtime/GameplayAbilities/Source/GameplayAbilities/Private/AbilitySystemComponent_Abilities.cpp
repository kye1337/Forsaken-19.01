#include "Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemComponent.h"

void AbilitySystemComponent::InternalServerTryActivateAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey& PredictionKey, const FGameplayEventData* TriggerEventData)
{
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);

	if (Spec == NULL)
	{
		AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
		return;
	}

	const UGameplayAbility* AbilityToActivate = Spec->Ability;

	if (AbilityToActivate == NULL)
	{
		AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
		return;
	}

	UGameplayAbility* InstancedAbility = NULL;
	Spec->InputPressed = true;

	if (AbilitySystemComponent->InternalTryActivateAbility(Handle, PredictionKey, &InstancedAbility, NULL, TriggerEventData))
	{
		// TryActivateAbility handles notifying the client of success
	}
	else
	{
		AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
		Spec->InputPressed = false;

		AbilitySystemComponent->MarkAbilitySpecDirty(*Spec);
	}
}

void AbilitySystemComponent::Setup()
{
	UKismetDetourLibrary::InitializeDetour<UFortAbilitySystemComponentAthena, VFT>(0x108, InternalServerTryActivateAbility);
}

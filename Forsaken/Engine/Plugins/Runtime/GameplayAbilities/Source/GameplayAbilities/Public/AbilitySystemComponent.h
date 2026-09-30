#pragma once

#include "CoreGlobals.h"

class AbilitySystemComponent
{
private:
	static void InternalServerTryActivateAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle AbilityToActivate, bool InputPressed, const FPredictionKey& PredictionKey, const FGameplayEventData* TriggerEventData);
public:
	static void Setup();
};

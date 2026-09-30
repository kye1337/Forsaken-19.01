#include "Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/Abilities/GameplayAbility.h"

void GameplayAbility::K2_ExecuteGameplayCue(UObject* Context, FFrame& Stack, void* Ret)
{
	UPROPERTY(FGameplayTag, GameplayCueTag);
	UPROPERTY(FGameplayEffectContextHandle, ContextHandle);

	Original::K2_ExecuteGameplayCue(Context, Stack, Ret);

	UGameplayAbility* GameplayAbility = Cast<UGameplayAbility>(Context);

	if (GameplayAbility == NULL)
		return;

	UAbilitySystemComponent* AbilitySystemComponent = GameplayAbility->GetAbilitySystemComponentFromActorInfo();

	if (AbilitySystemComponent == NULL)
		return;

	AbilitySystemComponent->NetMulticast_InvokeGameplayCueAdded(GameplayCueTag, FPredictionKey(), ContextHandle);
	AbilitySystemComponent->NetMulticast_InvokeGameplayCueExecuted(GameplayCueTag, FPredictionKey(), ContextHandle);
}

void GameplayAbility::Setup()
{
	UFunction* execK2_ExecuteGameplayCue = StaticFindObject<UFunction>(TEXT("/Script/GameplayAbilities.GameplayAbility.K2_ExecuteGameplayCue"));

	UKismetDetourLibrary::InitializeDetour<UGameplayAbility, Exec>(execK2_ExecuteGameplayCue, K2_ExecuteGameplayCue, &Original::K2_ExecuteGameplayCue);
}

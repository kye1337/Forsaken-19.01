#include "Pawns/FortPlayerPawn.h"
#include "Items/FortInventory.h"

// you're my biggest skid

void FortPlayerPawn::ServerReviveFromDBNO(AFortPlayerPawn* Pawn, AController* EventInstigator)
{
	static UClass* GAB_AthenaDBNOClass = StaticLoadObject<UClass>(
		TEXT("/Game/Abilities/NPC/Generic/GAB_AthenaDBNO.GAB_AthenaDBNO_C"));

	AController* PlayerController = Pawn->Controller;

	if (PlayerController == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	UFortAbilitySystemComponent* AbilitySystemComponent = PlayerState->AbilitySystemComponent;

	if (AbilitySystemComponent == NULL)
		return;

	FGameplayEventData GameplayEventData = FGameplayEventData();

	GameplayEventData.Instigator = EventInstigator;
	GameplayEventData.Target = Pawn;
	GameplayEventData.EventTag = Pawn->EventReviveTag;
	GameplayEventData.TargetTags = Pawn->GameplayTags;
	GameplayEventData.ContextHandle = AbilitySystemComponent->MakeEffectContext();
	GameplayEventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Pawn);

	if (AFortPlayerPawnAthena* InstigatorPawn = Cast<AFortPlayerPawnAthena>(EventInstigator->Pawn))
	{
		for (FGameplayTag& GameplayTag : InstigatorPawn->GameplayTags.GameplayTags)
		{
			GameplayEventData.InstigatorTags.GameplayTags.Add(GameplayTag);
		}
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Pawn, Pawn->EventReviveTag, GameplayEventData);

	Pawn->bPlayedDying = false;
	Pawn->bIsDBNO = false;

	Pawn->OnRep_IsDBNO();
	Pawn->SetHealth(30);

	PlayerState->DeathInfo = FDeathInfo();
	PlayerState->OnRep_DeathInfo();

	if (AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerController))	
	{
		PlayerControllerAthena->ClientOnPawnRevived(EventInstigator);
	}

	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->ActivatableAbilities.Items)
	{
		UGameplayAbility* Ability = AbilitySpec.Ability;

		if (Ability == NULL || !Ability->IsA(GAB_AthenaDBNOClass))
			continue;

		AbilitySystemComponent->ClientCancelAbility(AbilitySpec.Handle, AbilitySpec.ActivationInfo);
		AbilitySystemComponent->ClientEndAbility(AbilitySpec.Handle, AbilitySpec.ActivationInfo);
		AbilitySystemComponent->ServerEndAbility(AbilitySpec.Handle, AbilitySpec.ActivationInfo, AbilitySpec.ActivationInfo.PredictionKeyWhenActivated);
		break;
	}
}

void FortPlayerPawn::TryToAutoPickup(AFortPlayerPawn* Pawn, AFortPickup* Pickup)
{
	Original::TryToAutoPickup(Pawn, Pickup);

	AFortPlayerController* PlayerController = Cast<AFortPlayerController>(Pawn->Controller);

	if (PlayerController == NULL)
		return;

	if (Pickup->PawnWhoDroppedPickup == Pawn)
	{
		if (!Pickup->CheckForRePickup(Pawn))
			return;
	}

	static bool (*allowed)(AFortPickup*, AFortPlayerController*) =
		decltype(allowed)(InSDKUtils::GetImageBase() + 0x65308A4);

	if (allowed(Pickup, PlayerController))
	{
		Pawn->ServerHandlePickup(Pickup, Pickup->DefaultFlyTime, Pickup->PickupLocationData.StartDirection, Pickup->PickupLocationData.bPlayPickupSound);
	}
}

void FortPlayerPawn::ServerSendZiplineState(AFortPlayerPawn* Pawn, const FZiplinePawnState& InZiplineState)
{
	Pawn->ZiplineState = InZiplineState;
	Pawn->OnRep_ZiplineState();

	if (InZiplineState.bJumped)
	{
		float ZiplineJumpDampening = 0;
		float ZiplineJumpStrength = 0;

		UFortKismetLibrary::EvaluateCurveTableRow(Pawn->ZiplineJumpDampening, 0.f, &ZiplineJumpDampening, FString());
		UFortKismetLibrary::EvaluateCurveTableRow(Pawn->ZiplineJumpStrength, 0.f, &ZiplineJumpStrength, FString());

		if (ZiplineJumpDampening <= 0 || ZiplineJumpStrength <= 0)
			return;

		const FVector Velocity = Pawn->CharacterMovement->Velocity;

		FVector LaunchVelocity = FVector(-750, -750, ZiplineJumpStrength);

		float DampenedVelocityX = ZiplineJumpDampening * Velocity.X;
		float DampenedVelocityY = ZiplineJumpDampening * Velocity.Y;

		if (DampenedVelocityX >= -750.f)
		{
			LaunchVelocity.X = UKismetMathLibrary::Min(DampenedVelocityX, 750);
		}

		if (DampenedVelocityY >= -750.f)
		{
			LaunchVelocity.Y = UKismetMathLibrary::Min(DampenedVelocityY, 750);
		}

		Pawn->LaunchCharacterJump(LaunchVelocity, false, false, true, true);
	}
}

void FortPlayerPawn::Setup()
{
	UKismetDetourLibrary::InitializeDetour<AFortPlayerPawnAthena, VFT>(0x204, ServerReviveFromDBNO);
	UKismetDetourLibrary::InitializeDetour<AFortPlayerPawnAthena, VFT>(0x22e, ServerSendZiplineState);

	UKismetDetourLibrary::InitializeDetour<AFortPlayerPawn, MinHook>(0x1137A2C, TryToAutoPickup, &Original::TryToAutoPickup);
}

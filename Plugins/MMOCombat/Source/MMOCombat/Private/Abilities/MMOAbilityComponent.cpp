#include "Abilities/MMOAbilityComponent.h"
#include "Abilities/MMOAbilityBase.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Characters/MMOCharacterBase.h"

UMMOAbilityComponent::UMMOAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	SetIsReplicatedByDefault(true);
}

void UMMOAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMMOAbilityComponent, KnownAbilities);
	DOREPLIFETIME(UMMOAbilityComponent, HotbarSlots);
	DOREPLIFETIME(UMMOAbilityComponent, bIsCasting);
	DOREPLIFETIME(UMMOAbilityComponent, CurrentCastingAbility);
	DOREPLIFETIME(UMMOAbilityComponent, CurrentCastTarget);
	DOREPLIFETIME(UMMOAbilityComponent, CurrentCastLocation);
}

void UMMOAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMMOAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Advance active cast timer
	if (bIsCasting && CurrentCastingAbility)
	{
		AdvanceCast(DeltaTime);
	}

	// Tick cooldowns
	for (int32 i = ActiveCooldowns.Num() - 1; i >= 0; --i)
	{
		ActiveCooldowns[i].Remaining -= DeltaTime;
		if (ActiveCooldowns[i].Remaining <= 0.0f)
		{
			UMMOAbilityBase* ExpiredAbility = ActiveCooldowns[i].Ability.LoadSynchronous();
			ActiveCooldowns.RemoveAt(i);
			if (ExpiredAbility)
			{
				ExpiredAbility->bIsOnCooldown = false;
				ExpiredAbility->CurrentCooldownRemaining = 0.0f;
				OnCooldownExpired.Broadcast(ExpiredAbility);
			}
		}
	}
}

void UMMOAbilityComponent::OnRep_IsCasting()
{
	if (!bIsCasting)
	{
		CurrentCastingAbility = nullptr;
		CurrentCastTarget = nullptr;
	}
}

bool UMMOAbilityComponent::LearnAbility(TSoftObjectPtr<UMMOAbilityBase> AbilityPtr, int32 Rank)
{
	if (GetNetMode() == NM_Client)
	{
		return false;
	}

	UMMOAbilityBase* Ability = AbilityPtr.LoadSynchronous();
	if (!Ability)
	{
		UE_LOG(LogTemp, Warning, TEXT("LearnAbility: Ability asset is null."));
		return false;
	}

	if (HasAbility(Ability->AbilityID))
	{
		UE_LOG(LogTemp, Warning, TEXT("LearnAbility: Ability %s already known."), *Ability->AbilityID.ToString());
		return false;
	}

	UMMOAbilityBase* NewAbility = DuplicateObject(Ability, this);
	NewAbility->OwningComponent = this;
	NewAbility->CurrentRank = FMath::Clamp(Rank, 1, NewAbility->MaxRank);
	KnownAbilities.Add(NewAbility);
	AbilityRanks.Add(NewAbility->AbilityID, NewAbility->CurrentRank);

	OnAbilityLearned.Broadcast(NewAbility, NewAbility->CurrentRank);
	return true;
}

bool UMMOAbilityComponent::UnlearnAbility(FName AbilityID)
{
	if (GetNetMode() == NM_Client)
	{
		return false;
	}

	for (int32 i = 0; i < KnownAbilities.Num(); ++i)
	{
		if (KnownAbilities[i]->AbilityID == AbilityID)
		{
			KnownAbilities.RemoveAt(i);
			AbilityRanks.Remove(AbilityID);

			for (int32 j = HotbarSlots.Num() - 1; j >= 0; --j)
			{
				if (HotbarSlots[j].Ability.Get() == KnownAbilities[i])
				{
					HotbarSlots[j].Ability = nullptr;
				}
			}
			return true;
		}
	}
	return false;
}

bool UMMOAbilityComponent::RankUpAbility(FName AbilityID)
{
	if (GetNetMode() == NM_Client)
	{
		return false;
	}

	int32* CurrentRankPtr = AbilityRanks.Find(AbilityID);
	if (!CurrentRankPtr)
	{
		return false;
	}

	UMMOAbilityBase* Ability = FindAbilityByID(AbilityID);
	if (!Ability || *CurrentRankPtr >= Ability->MaxRank)
	{
		return false;
	}

	(*CurrentRankPtr)++;
	Ability->CurrentRank = *CurrentRankPtr;
	return true;
}

UMMOAbilityBase* UMMOAbilityComponent::FindAbilityByID(FName AbilityID) const
{
	for (UMMOAbilityBase* Ability : KnownAbilities)
	{
		if (Ability && Ability->AbilityID == AbilityID)
		{
			return Ability;
		}
	}
	return nullptr;
}

bool UMMOAbilityComponent::HasAbility(FName AbilityID) const
{
	return FindAbilityByID(AbilityID) != nullptr;
}

bool UMMOAbilityComponent::AssignAbilityToSlot(int32 SlotIndex, TSoftObjectPtr<UMMOAbilityBase> Ability)
{
	if (SlotIndex < 0 || SlotIndex >= HotbarSlots.Num())
	{
		return false;
	}

	HotbarSlots[SlotIndex].Ability = Ability;
	HotbarSlots[SlotIndex].SlotIndex = SlotIndex;
	return true;
}

bool UMMOAbilityComponent::RemoveAbilityFromSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= HotbarSlots.Num())
	{
		return false;
	}

	HotbarSlots[SlotIndex].Ability = nullptr;
	return true;
}

UMMOAbilityBase* UMMOAbilityComponent::GetAbilityInSlot(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= HotbarSlots.Num())
	{
		return nullptr;
	}
	return HotbarSlots[SlotIndex].Ability.LoadSynchronous();
}

bool UMMOAbilityComponent::StartCast(int32 SlotIndex, const FVector& TargetLocation, AMMOCharacterBase* Target)
{
	if (SlotIndex < 0 || SlotIndex >= HotbarSlots.Num())
	{
		return false;
	}

	UMMOAbilityBase* Ability = HotbarSlots[SlotIndex].Ability.LoadSynchronous();
	return StartCastByAbility(Ability, TargetLocation, Target);
}

bool UMMOAbilityComponent::StartCastByAbility(UMMOAbilityBase* Ability, const FVector& TargetLocation, AMMOCharacterBase* Target)
{
	if (!Ability || !GetOwnerCharacter())
	{
		return false;
	}

	if (bIsCasting)
	{
		return false;
	}

	if (IsOnCooldown(Ability))
	{
		return false;
	}

	if (!Ability->CanCast(GetOwnerCharacter(), Target))
	{
		return false;
	}

	if (GetOwnerRole() < ROLE_Authority)
	{
		PredictiveStartCast(Ability, TargetLocation, Target);
	}

	Server_StartCast(Ability, TargetLocation, Target);
	return true;
}

void UMMOAbilityComponent::CancelCast(bool bPlayCancelledAnim)
{
	if (!bIsCasting || !CurrentCastingAbility)
	{
		return;
	}

	if (GetOwnerRole() < ROLE_Authority)
	{
		CurrentCastingAbility->OnCastCancelled(GetOwnerCharacter());
	}

	Server_CancelCast();
}

bool UMMOAbilityComponent::Server_StartCast_Validate(UMMOAbilityBase* Ability, const FVector& TargetLocation, AMMOCharacterBase* Target)
{
	return Ability != nullptr;
}

void UMMOAbilityComponent::Server_StartCast_Implementation(UMMOAbilityBase* Ability, const FVector& TargetLocation, AMMOCharacterBase* Target)
{
	if (!Ability || !GetOwnerCharacter())
	{
		return;
	}

	if (bIsCasting || IsOnCooldown(Ability))
	{
		return;
	}

	Internal_StartCast(Ability, TargetLocation, Target);
}

bool UMMOAbilityComponent::Server_CancelCast_Validate()
{
	return true;
}

void UMMOAbilityComponent::Server_CancelCast_Implementation()
{
	if (CurrentCastingAbility)
	{
		CurrentCastingAbility->OnCastCancelled(GetOwnerCharacter());
		Internal_CompleteCast();
	}
}

void UMMOAbilityComponent::Client_CastResult_Hit_Implementation(UMMOAbilityBase* Ability, AMMOCharacterBase* Target, float Damage, bool bCritical)
{
	OnCastResult.Broadcast(Ability, Target, bCritical);
}

void UMMOAbilityComponent::Client_CastResult_Miss_Implementation(UMMOAbilityBase* Ability, AMMOCharacterBase* Target)
{
	OnCastResult.Broadcast(Ability, Target, false);
}

void UMMOAbilityComponent::Client_CastResult_Crit_Implementation(UMMOAbilityBase* Ability, AMMOCharacterBase* Target, float Damage)
{
	OnCastResult.Broadcast(Ability, Target, true);
}

void UMMOAbilityComponent::StartCooldown(UMMOAbilityBase* Ability)
{
	if (!Ability)
	{
		return;
	}

	const float CooldownDuration = Ability->GetCooldownForRank(Ability->CurrentRank);
	if (CooldownDuration <= 0.0f)
	{
		return;
	}

	FMMOAbilityCooldown NewCooldown;
	NewCooldown.Ability = Ability;
	NewCooldown.Duration = CooldownDuration;
	NewCooldown.Remaining = CooldownDuration;
	NewCooldown.StartTime = GetWorld()->GetTimeSeconds();
	ActiveCooldowns.Add(NewCooldown);

	Ability->bIsOnCooldown = true;
	Ability->CurrentCooldownRemaining = CooldownDuration;

	OnCooldownStarted.Broadcast(Ability, CooldownDuration);
}

bool UMMOAbilityComponent::IsOnCooldown(UMMOAbilityBase* Ability) const
{
	for (const FMMOAbilityCooldown& Cooldown : ActiveCooldowns)
	{
		if (Cooldown.Ability.LoadSynchronous() == Ability && Cooldown.Remaining > 0.0f)
		{
			return true;
		}
	}
	return false;
}

float UMMOAbilityComponent::GetCooldownRemaining(UMMOAbilityBase* Ability) const
{
	for (const FMMOAbilityCooldown& Cooldown : ActiveCooldowns)
	{
		if (Cooldown.Ability.LoadSynchronous() == Ability)
		{
			return Cooldown.Remaining;
		}
	}
	return 0.0f;
}

void UMMOAbilityComponent::PredictiveStartCast(UMMOAbilityBase* Ability, const FVector& TargetLocation, AMMOCharacterBase* Target)
{
	if (!Ability)
	{
		return;
	}

	PredictedCasts.Add(Ability);
	Internal_StartCast(Ability, TargetLocation, Target);
}

void UMMOAbilityComponent::ConfirmPredictedCast(UMMOAbilityBase* Ability)
{
	PredictedCasts.Remove(Ability);
}

void UMMOAbilityComponent::RejectPredictedCast(UMMOAbilityBase* Ability)
{
	if (Ability == CurrentCastingAbility)
	{
		Ability->OnCastCancelled(GetOwnerCharacter());
		Internal_CompleteCast();
	}
	PredictedCasts.Remove(Ability);
}

AMMOCharacterBase* UMMOAbilityComponent::GetOwnerCharacter() const
{
	return Cast<AMMOCharacterBase>(GetOwner());
}

void UMMOAbilityComponent::Internal_StartCast(UMMOAbilityBase* Ability, const FVector& TargetLocation, AMMOCharacterBase* Target)
{
	bIsCasting = true;
	CurrentCastingAbility = Ability;
	CurrentCastTarget = Target;
	CurrentCastLocation = TargetLocation;
	CastElapsedTime = 0.0f;
	CurrentCastTime = Ability->GetCastTimeForRank(Ability->CurrentRank);

	Ability->OwningComponent = this;
	Ability->OnCastStart(GetOwnerCharacter(), TargetLocation, Target);

	if (CurrentCastTime <= 0.0f)
	{
		// Instant cast
		Ability->OnCastCompleted(GetOwnerCharacter(), TargetLocation, Target);
		Internal_CompleteCast();
		StartCooldown(Ability);
	}
}

void UMMOAbilityComponent::Internal_CompleteCast()
{
	bIsCasting = false;
	CurrentCastingAbility = nullptr;
	CurrentCastTarget = nullptr;
	CastElapsedTime = 0.0f;
	CurrentCastTime = 0.0f;
}

void UMMOAbilityComponent::AdvanceCast(float DeltaTime)
{
	if (!CurrentCastingAbility)
	{
		return;
	}

	CastElapsedTime += DeltaTime;
	CurrentCastingAbility->OnCastTick(GetOwnerCharacter(), DeltaTime);

	if (CastElapsedTime >= CurrentCastTime)
	{
		CurrentCastingAbility->OnCastCompleted(GetOwnerCharacter(), CurrentCastLocation, CurrentCastTarget);
		Internal_CompleteCast();
		StartCooldown(CurrentCastingAbility);
	}
}

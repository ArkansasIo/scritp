#include "Components/MMOCombatStateComponent.h"
#include "Characters/MMOCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMMOCombatStateComponent::UMMOCombatStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(true);

	CombatState = EMMOCombatState::OutOfCombat;
	CombatTimeoutDuration = 15.0f;
	CurrentCombatTimer = 0.0f;

	LastDamageTime = 0.0f;
	LastDamageAmount = 0.0f;
	LastAttacker = nullptr;
	LockedOnTarget = nullptr;
}

void UMMOCombatStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMMOCombatStateComponent, CombatState, COND_Always);
	DOREPLIFETIME_CONDITION(UMMOCombatStateComponent, ThreatList, COND_OwnerOnly);
}

void UMMOCombatStateComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMMOCombatStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!HasAuthority())
	{
		return;
	}

	UpdateCombatTimer(DeltaTime);
}

void UMMOCombatStateComponent::UpdateCombatTimer(float DeltaTime)
{
	if (CombatState == EMMOCombatState::InCombat || CombatState == EMMOCombatState::InPvP)
	{
		CurrentCombatTimer += DeltaTime;

		if (CurrentCombatTimer >= CombatTimeoutDuration)
		{
			SetCombatState(EMMOCombatState::OutOfCombat);
		}
	}
}

void UMMOCombatStateComponent::OnRep_CombatState(EMMOCombatState OldState)
{
	OnCombatStateChanged.Broadcast(CombatState);
}

void UMMOCombatStateComponent::SetCombatState(EMMOCombatState NewState)
{
	if (!HasAuthority())
	{
		return;
	}

	EMMOCombatState OldState = CombatState;
	CombatState = NewState;

	CurrentCombatTimer = 0.0f;

	OnRep_CombatState(OldState);
}

void UMMOCombatStateComponent::EnterCombat()
{
	if (!HasAuthority())
	{
		return;
	}

	SetCombatState(EMMOCombatState::InCombat);
}

void UMMOCombatStateComponent::ExitCombat()
{
	if (!HasAuthority())
	{
		return;
	}

	SetCombatState(EMMOCombatState::OutOfCombat);
	ClearThreatList();
	UnlockTarget();
}

void UMMOCombatStateComponent::ResetCombatTimer()
{
	CurrentCombatTimer = 0.0f;
}

void UMMOCombatStateComponent::LockOnTarget(AActor* Target)
{
	if (LockedOnTarget != Target)
	{
		AActor* OldTarget = LockedOnTarget;
		LockedOnTarget = Target;

		if (OldTarget)
		{
			OnTargetUnlocked.Broadcast(OldTarget);
		}

		if (LockedOnTarget)
		{
			OnTargetLocked.Broadcast(LockedOnTarget);
		}
	}
}

void UMMOCombatStateComponent::UnlockTarget()
{
	if (LockedOnTarget)
	{
		AActor* OldTarget = LockedOnTarget;
		LockedOnTarget = nullptr;
		OnTargetUnlocked.Broadcast(OldTarget);
	}
}

bool UMMOCombatStateComponent::IsTargetLocked() const
{
	return LockedOnTarget != nullptr;
}

void UMMOCombatStateComponent::AddThreat(AActor* Actor, float ThreatAmount)
{
	if (!HasAuthority() || !Actor)
	{
		return;
	}

	for (FThreatEntry& Entry : ThreatList)
	{
		if (Entry.ThreatActor == Actor)
		{
			Entry.ThreatValue += ThreatAmount;
			Entry.LastDamageTime = GetWorld()->GetTimeSeconds();
			return;
		}
	}

	FThreatEntry NewEntry;
	NewEntry.ThreatActor = Actor;
	NewEntry.ThreatValue = ThreatAmount;
	NewEntry.LastDamageTime = GetWorld()->GetTimeSeconds();
	ThreatList.Add(NewEntry);
}

void UMMOCombatStateComponent::RemoveThreat(AActor* Actor)
{
	if (!HasAuthority())
	{
		return;
	}

	ThreatList.RemoveAll([Actor](const FThreatEntry& Entry)
	{
		return Entry.ThreatActor == Actor;
	});
}

void UMMOCombatStateComponent::ClearThreatList()
{
	if (!HasAuthority())
	{
		return;
	}

	ThreatList.Empty();
}

AActor* UMMOCombatStateComponent::GetHighestThreatTarget() const
{
	if (ThreatList.Num() == 0)
	{
		return nullptr;
	}

	AActor* HighestActor = nullptr;
	float HighestValue = -1.0f;

	for (const FThreatEntry& Entry : ThreatList)
	{
		if (Entry.ThreatValue > HighestValue)
		{
			HighestValue = Entry.ThreatValue;
			HighestActor = Entry.ThreatActor;
		}
	}

	return HighestActor;
}

float UMMOCombatStateComponent::GetThreatValue(AActor* Actor) const
{
	for (const FThreatEntry& Entry : ThreatList)
	{
		if (Entry.ThreatActor == Actor)
		{
			return Entry.ThreatValue;
		}
	}

	return 0.0f;
}

void UMMOCombatStateComponent::RecordDamage(AActor* Attacker, float DamageAmount)
{
	if (!HasAuthority())
	{
		return;
	}

	LastAttacker = Attacker;
	LastDamageTime = GetWorld()->GetTimeSeconds();
	LastDamageAmount = DamageAmount;

	AddThreat(Attacker, DamageAmount);
	ResetCombatTimer();
}

#include "Combat/MMOCombatManagerComponent.h"
#include "Abilities/MMODamageSystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "MMOCharacterBase.h"

UMMOCombatManagerComponent::UMMOCombatManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	SetIsReplicatedByDefault(true);
}

void UMMOCombatManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMMOCombatManagerComponent, bIsInCombat);
	DOREPLIFETIME(UMMOCombatManagerComponent, PvPState);
	DOREPLIFETIME(UMMOCombatManagerComponent, CombatTimer);
	DOREPLIFETIME(UMMOCombatManagerComponent, ActiveDuel);
	DOREPLIFETIME(UMMOCombatManagerComponent, BindPointLocation);
	DOREPLIFETIME(UMMOCombatManagerComponent, bIsDead);
}

void UMMOCombatManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMMOCombatManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!HasAuthority())
	{
		return;
	}

	// Combat timer
	if (bIsInCombat)
	{
		CombatTimer -= DeltaTime;
		if (CombatTimer <= 0.0f)
		{
			ExitCombat(nullptr);
		}
	}

	// Duel request timeout
	if (ActiveDuel.DuelState == EMMODuelState::Requested)
	{
		if (GetWorld()->GetTimeSeconds() - ActiveDuel.RequestTime >= DuelRequestTimeout)
		{
			ActiveDuel.DuelState = EMMODuelState::None;
			ActiveDuel.Challenger = nullptr;
			ActiveDuel.Opponent = nullptr;
		}
	}
}

void UMMOCombatManagerComponent::OnRep_CombatState()
{
	// Client-side combat state change
}

void UMMOCombatManagerComponent::OnRep_PvPState()
{
	// Client-side PvP state change
}

void UMMOCombatManagerComponent::AddThreat(AActor* Source, float Amount)
{
	if (!Source || Amount <= 0.0f)
	{
		return;
	}

	for (FMMOThreatEntry& Entry : ThreatList)
	{
		if (Entry.ThreatSource == Source)
		{
			Entry.ThreatValue += Amount;
			Entry.LastThreatUpdateTime = GetWorld()->GetTimeSeconds();
			Entry.bIsInCombat = true;
			return;
		}
	}

	FMMOThreatEntry NewEntry;
	NewEntry.ThreatSource = Source;
	NewEntry.ThreatValue = Amount;
	NewEntry.LastThreatUpdateTime = GetWorld()->GetTimeSeconds();
	NewEntry.bIsInCombat = true;
	ThreatList.Add(NewEntry);

	EnterCombat(Source);
}

void UMMOCombatManagerComponent::RemoveThreat(AActor* Source)
{
	ThreatList.RemoveAll([Source](const FMMOThreatEntry& Entry)
	{
		return Entry.ThreatSource == Source;
	});
}

void UMMOCombatManagerComponent::ClearAllThreat()
{
	ThreatList.Empty();
}

AActor* UMMOCombatManagerComponent::GetHighestThreatTarget() const
{
	AActor* HighestTarget = nullptr;
	float HighestThreat = 0.0f;

	for (const FMMOThreatEntry& Entry : ThreatList)
	{
		if (Entry.ThreatValue > HighestThreat)
		{
			HighestThreat = Entry.ThreatValue;
			HighestTarget = Entry.ThreatSource;
		}
	}

	return HighestTarget;
}

float UMMOCombatManagerComponent::GetThreatForActor(AActor* Source) const
{
	for (const FMMOThreatEntry& Entry : ThreatList)
	{
		if (Entry.ThreatSource == Source)
		{
			return Entry.ThreatValue;
		}
	}
	return 0.0f;
}

void UMMOCombatManagerComponent::EnablePvP()
{
	if (!HasAuthority())
	{
		Server_FlagForPvP();
		return;
	}

	PvPState = EMMOPvPState::PvPEnabled;
}

void UMMOCombatManagerComponent::DisablePvP()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsInCombat)
	{
		return;
	}

	PvPState = EMMOPvPState::Peaceful;
}

bool UMMOCombatManagerComponent::CanAttack(AActor* Target) const
{
	if (!Target || Target == GetOwner())
	{
		return false;
	}

	AMMOCharacterBase* TargetChar = Cast<AMMOCharacterBase>(Target);
	if (!TargetChar)
	{
		return false;
	}

	UMMOCombatManagerComponent* TargetCombat = TargetChar->FindComponentByClass<UMMOCombatManagerComponent>();
	if (!TargetCombat)
	{
		return true;
	}

	// In a duel, only attack duel opponent
	if (ActiveDuel.DuelState == EMMODuelState::Active)
	{
		return Target == ActiveDuel.Opponent;
	}

	// PvP check
	if (PvPState == EMMOPvPState::Peaceful && TargetCombat->PvPState == EMMOPvPState::Peaceful)
	{
		return false;
	}

	return true;
}

bool UMMOCombatManagerComponent::Server_FlagForPvP_Validate()
{
	return true;
}

void UMMOCombatManagerComponent::Server_FlagForPvP_Implementation()
{
	EnablePvP();
}

void UMMOCombatManagerComponent::RequestDuel(AMMOCharacterBase* Opponent)
{
	if (!Opponent || !HasAuthority())
	{
		return;
	}

	Server_RequestDuel(Opponent);
}

void UMMOCombatManagerComponent::AcceptDuel()
{
	if (!HasAuthority() || ActiveDuel.DuelState != EMMODuelState::Requested)
	{
		return;
	}

	Server_AcceptDuel(ActiveDuel.Challenger);
}

void UMMOCombatManagerComponent::DeclineDuel()
{
	if (!HasAuthority() || ActiveDuel.DuelState != EMMODuelState::Requested)
	{
		return;
	}

	Server_DeclineDuel(ActiveDuel.Challenger);
}

void UMMOCombatManagerComponent::ForfeitDuel()
{
	if (!HasAuthority() || ActiveDuel.DuelState != EMMODuelState::Active)
	{
		return;
	}

	const AMMOCharacterBase* Winner = (GetOwnerCharacter() == ActiveDuel.Challenger) ? ActiveDuel.Opponent : ActiveDuel.Challenger;
	Server_EndDuel(const_cast<AMMOCharacterBase*>(Winner), GetOwnerCharacter(), true);
}

bool UMMOCombatManagerComponent::Server_RequestDuel_Validate(AMMOCharacterBase* Opponent)
{
	return Opponent != nullptr;
}

void UMMOCombatManagerComponent::Server_RequestDuel_Implementation(AMMOCharacterBase* Opponent)
{
	if (!Opponent || ActiveDuel.DuelState != EMMODuelState::None)
	{
		return;
	}

	UMMOCombatManagerComponent* OpponentCombat = Opponent->FindComponentByClass<UMMOCombatManagerComponent>();
	if (!OpponentCombat || OpponentCombat->ActiveDuel.DuelState != EMMODuelState::None)
	{
		return;
	}

	ActiveDuel.Challenger = GetOwnerCharacter();
	ActiveDuel.Opponent = Opponent;
	ActiveDuel.DuelState = EMMODuelState::Requested;
	ActiveDuel.RequestTime = GetWorld()->GetTimeSeconds();
	ActiveDuel.StartLocation = GetOwnerCharacter()->GetActorLocation();

	OpponentCombat->ActiveDuel = ActiveDuel;

	OnDuelRequested.Broadcast(ActiveDuel.Challenger, ActiveDuel.Opponent);
	OpponentCombat->OnDuelRequested.Broadcast(ActiveDuel.Challenger, ActiveDuel.Opponent);
}

bool UMMOCombatManagerComponent::Server_AcceptDuel_Validate(AMMOCharacterBase* Challenger)
{
	return Challenger != nullptr;
}

void UMMOCombatManagerComponent::Server_AcceptDuel_Implementation(AMMOCharacterBase* Challenger)
{
	if (ActiveDuel.DuelState != EMMODuelState::Requested || ActiveDuel.Challenger != Challenger)
	{
		return;
	}

	UMMOCombatManagerComponent* ChallengerCombat = Challenger->FindComponentByClass<UMMOCombatManagerComponent>();
	if (!ChallengerCombat)
	{
		return;
	}

	ActiveDuel.DuelState = EMMODuelState::Active;
	ChallengerCombat->ActiveDuel = ActiveDuel;

	OnDuelStarted.Broadcast(ActiveDuel.Challenger, ActiveDuel.Opponent);
	ChallengerCombat->OnDuelStarted.Broadcast(ActiveDuel.Challenger, ActiveDuel.Opponent);
}

bool UMMOCombatManagerComponent::Server_DeclineDuel_Validate(AMMOCharacterBase* Challenger)
{
	return Challenger != nullptr;
}

void UMMOCombatManagerComponent::Server_DeclineDuel_Implementation(AMMOCharacterBase* Challenger)
{
	if (ActiveDuel.DuelState != EMMODuelState::Requested || ActiveDuel.Challenger != Challenger)
	{
		return;
	}

	UMMOCombatManagerComponent* ChallengerCombat = Challenger->FindComponentByClass<UMMOCombatManagerComponent>();
	if (!ChallengerCombat)
	{
		return;
	}

	ActiveDuel.DuelState = EMMODuelState::Declined;
	ChallengerCombat->ActiveDuel = ActiveDuel;

	OnDuelEnded.Broadcast(nullptr, nullptr, false);
	ChallengerCombat->OnDuelEnded.Broadcast(nullptr, nullptr, false);

	ActiveDuel = FMMODuelInfo();
	ChallengerCombat->ActiveDuel = FMMODuelInfo();
}

bool UMMOCombatManagerComponent::Server_EndDuel_Validate(AMMOCharacterBase* Winner, AMMOCharacterBase* Loser, bool bForfeit)
{
	return Winner != nullptr && Loser != nullptr;
}

void UMMOCombatManagerComponent::Server_EndDuel_Implementation(AMMOCharacterBase* Winner, AMMOCharacterBase* Loser, bool bForfeit)
{
	if (ActiveDuel.DuelState != EMMODuelState::Active)
	{
		return;
	}

	ActiveDuel.Winner = Winner;
	ActiveDuel.DuelState = EMMODuelState::Completed;

	UMMOCombatManagerComponent* WinnerCombat = Winner->FindComponentByClass<UMMOCombatManagerComponent>();
	UMMOCombatManagerComponent* LoserCombat = Loser->FindComponentByClass<UMMOCombatManagerComponent>();

	if (WinnerCombat)
	{
		WinnerCombat->ActiveDuel = ActiveDuel;
		WinnerCombat->OnDuelEnded.Broadcast(Winner, Loser, bForfeit);
		WinnerCombat->ExitCombat(Loser);
	}

	if (LoserCombat)
	{
		LoserCombat->ActiveDuel = ActiveDuel;
		LoserCombat->OnDuelEnded.Broadcast(Winner, Loser, bForfeit);
		LoserCombat->ExitCombat(Winner);
	}
}

void UMMOCombatManagerComponent::OnDeath(AActor* Killer)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsInCombat = false;
	CombatTimer = 0.0f;

	if (ActiveDuel.DuelState == EMMODuelState::Active)
	{
		const AMMOCharacterBase* Winner = (GetOwnerCharacter() == ActiveDuel.Challenger) ? ActiveDuel.Opponent : ActiveDuel.Challenger;
		Server_EndDuel(const_cast<AMMOCharacterBase*>(Winner), GetOwnerCharacter(), false);
	}

	OnActorDeath.Broadcast(GetOwner(), Killer);

	if (AMMOCharacterBase* DeadChar = Cast<AMMOCharacterBase>(GetOwner()))
	{
		DeadChar->SetLifeSpan(60.0f);
	}

	AddCombatLogEntry(FText::Format(
		NSLOCTEXT("MMOCombat", "DeathLog", "{0} was killed by {1}"),
		FText::FromString(GetOwner() ? GetOwner()->GetName() : TEXT("Unknown")),
		FText::FromString(Killer ? Killer->GetName() : TEXT("Unknown"))
	), Killer, GetOwner(), 0.0f);
}

void UMMOCombatManagerComponent::SetBindPoint(const FVector& Location)
{
	BindPointLocation = Location;
}

void UMMOCombatManagerComponent::ReleaseSpirit()
{
	AMMOCharacterBase* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar || !bIsDead)
	{
		return;
	}

	if (AMMOCharacterBase* Char = OwnerChar)
	{
		Char->SetActorLocation(BindPointLocation);
		Char->SetLifeSpan(0.0f);
	}
}

void UMMOCombatManagerComponent::RespawnAtBindPoint()
{
	Server_Respawn(EMMORespawnMethod::BindPoint);
}

void UMMOCombatManagerComponent::RespawnAtGraveyard()
{
	Server_Respawn(EMMORespawnMethod::Graveyard);
}

bool UMMOCombatManagerComponent::Server_Respawn_Validate(EMMORespawnMethod Method)
{
	return true;
}

void UMMOCombatManagerComponent::Server_Respawn_Implementation(EMMORespawnMethod Method)
{
	if (!bIsDead)
	{
		return;
	}

	AMMOCharacterBase* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar)
	{
		return;
	}

	FVector RespawnLocation = BindPointLocation;

	if (Method == EMMORespawnMethod::Graveyard)
	{
		// In a full implementation, query the game state for nearest graveyard
		RespawnLocation = BindPointLocation;
	}

	OwnerChar->SetHealth(OwnerChar->GetMaxHealth());
	OwnerChar->SetMana(OwnerChar->GetMaxMana());
	OwnerChar->SetActorLocation(RespawnLocation);
	OwnerChar->SetLifeSpan(0.0f);

	bIsDead = false;
	ClearAllThreat();
	ExitCombat(nullptr);

	PvPState = EMMOPvPState::Peaceful;
}

void UMMOCombatManagerComponent::AddCombatLogEntry(
	const FText& Description, AActor* Source, AActor* Target,
	float Value, EMMOHitResult HitResult, const FGameplayTagContainer& EventTags)
{
	if (CombatLog.Num() >= MaxCombatLogEntries)
	{
		CombatLog.RemoveAt(0, CombatLog.Num() - MaxCombatLogEntries + 1);
	}

	FMMOCombatLogEntry Entry;
	Entry.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Entry.EventDescription = Description;
	Entry.SourceActor = Source;
	Entry.TargetActor = Target;
	Entry.Value = Value;
	Entry.HitResult = HitResult;
	Entry.EventTags = EventTags;
	CombatLog.Add(Entry);
}

void UMMOCombatManagerComponent::ClearCombatLog()
{
	CombatLog.Empty();
}

void UMMOCombatManagerComponent::EnterCombat(AActor* Opponent)
{
	if (bIsInCombat)
	{
		CombatTimer = OutOfCombatDelay;
		return;
	}

	bIsInCombat = true;
	CombatTimer = OutOfCombatDelay;

	PvPState = EMMOPvPState::InCombat;

	OnCombatEnter.Broadcast(GetOwner(), Opponent);
	AddCombatLogEntry(FText::Format(
		NSLOCTEXT("MMOCombat", "CombatEnterLog", "Entered combat with {0}"),
		FText::FromString(Opponent ? Opponent->GetName() : TEXT("Unknown"))
	), GetOwner(), Opponent, 0.0f);
}

void UMMOCombatManagerComponent::ExitCombat(AActor* Opponent)
{
	if (!bIsInCombat)
	{
		return;
	}

	bIsInCombat = false;
	CombatTimer = 0.0f;

	if (PvPState == EMMOPvPState::InCombat)
	{
		PvPState = EMMOPvPState::RecentlyEngaged;
	}

	OnCombatExit.Broadcast(GetOwner(), Opponent);
	AddCombatLogEntry(NSLOCTEXT("MMOCombat", "CombatExitLog", "Exited combat"), GetOwner(), Opponent, 0.0f);
}

bool UMMOCombatManagerComponent::CanAttackTarget(AActor* Target) const
{
	return CanAttack(Target);
}

AMMOCharacterBase* UMMOCombatManagerComponent::GetOwnerCharacter() const
{
	return Cast<AMMOCharacterBase>(GetOwner());
}

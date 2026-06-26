#include "Abilities/MMOTargetingSystem.h"
#include "Abilities/MMOAbilityBase.h"
#include "MMOCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CollisionQueryParams.h"
#include "Engine/HitResult.h"

UMMOTargetingSystemComponent::UMMOTargetingSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	SetIsReplicatedByDefault(true);
}

void UMMOTargetingSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMMOTargetingSystemComponent, HardTarget);
}

void UMMOTargetingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Validate hard target periodically
	if (HardTarget)
	{
		FMMOTargetingFilter DefaultFilter;
		DefaultFilter.bAliveOnly = true;
		DefaultFilter.bRequireLineOfSight = false;
		DefaultFilter.MaxRange = 0.0f;
		if (!IsValidTarget(HardTarget, DefaultFilter))
		{
			ClearHardTarget();
		}
	}
}

void UMMOTargetingSystemComponent::OnRep_HardTarget()
{
	OnHardTargetChanged.Broadcast(HardTarget);
}

void UMMOTargetingSystemComponent::SetHardTarget(AMMOCharacterBase* NewTarget)
{
	if (HardTarget == NewTarget)
	{
		return;
	}

	HardTarget = NewTarget;
	OnRep_HardTarget();
}

void UMMOTargetingSystemComponent::ClearHardTarget()
{
	if (!HardTarget)
	{
		return;
	}

	HardTarget = nullptr;
	OnRep_HardTarget();
}

void UMMOTargetingSystemComponent::SetSoftTarget(AMMOCharacterBase* NewTarget)
{
	if (SoftTarget == NewTarget)
	{
		return;
	}

	SoftTarget = NewTarget;
	OnSoftTargetChanged.Broadcast(SoftTarget);
}

void UMMOTargetingSystemComponent::ClearSoftTarget()
{
	if (!SoftTarget)
	{
		return;
	}

	SoftTarget = nullptr;
	OnSoftTargetChanged.Broadcast(nullptr);
}

void UMMOTargetingSystemComponent::SetGroundTarget(const FVector& Location)
{
	GroundTargetLocation = Location;
	OnGroundTargetChanged.Broadcast(Location);
}

void UMMOTargetingSystemComponent::ClearGroundTarget()
{
	GroundTargetLocation = FVector::ZeroVector;
	OnGroundTargetChanged.Broadcast(FVector::ZeroVector);
}

void UMMOTargetingSystemComponent::TabTargetForward(const FMMOTargetingFilter& Filter)
{
	RefreshEnemyList();

	if (CachedEnemyList.Num() == 0)
	{
		return;
	}

	CurrentTabIndex = (CurrentTabIndex + 1) % CachedEnemyList.Num();

	// Validate the new target
	int32 Attempts = 0;
	while (Attempts < CachedEnemyList.Num())
	{
		AMMOCharacterBase* Candidate = CachedEnemyList[CurrentTabIndex];
		if (Candidate && IsValidTarget(Candidate, Filter))
		{
			SetHardTarget(Candidate);
			return;
		}
		CurrentTabIndex = (CurrentTabIndex + 1) % CachedEnemyList.Num();
		Attempts++;
	}

	ClearHardTarget();
}

void UMMOTargetingSystemComponent::TabTargetBackward(const FMMOTargetingFilter& Filter)
{
	RefreshEnemyList();

	if (CachedEnemyList.Num() == 0)
	{
		return;
	}

	CurrentTabIndex = (CurrentTabIndex - 1 + CachedEnemyList.Num()) % CachedEnemyList.Num();

	int32 Attempts = 0;
	while (Attempts < CachedEnemyList.Num())
	{
		AMMOCharacterBase* Candidate = CachedEnemyList[CurrentTabIndex];
		if (Candidate && IsValidTarget(Candidate, Filter))
		{
			SetHardTarget(Candidate);
			return;
		}
		CurrentTabIndex = (CurrentTabIndex - 1 + CachedEnemyList.Num()) % CachedEnemyList.Num();
		Attempts++;
	}

	ClearHardTarget();
}

AMMOCharacterBase* UMMOTargetingSystemComponent::FindBestTarget(EMMOTargetPriority Priority, const FMMOTargetingFilter& Filter) const
{
	RefreshEnemyList();

	if (CachedEnemyList.Num() == 0)
	{
		return nullptr;
	}

	AMMOCharacterBase* BestTarget = nullptr;
	float BestValue = 0.0f;

	const AActor* Owner = GetOwner();
	const FVector OwnerLocation = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;

	for (AMMOCharacterBase* Candidate : CachedEnemyList)
	{
		if (!Candidate || !IsValidTarget(Candidate, Filter))
		{
			continue;
		}

		float Value = 0.0f;
		switch (Priority)
		{
		case EMMOTargetPriority::Nearest:
			Value = -FVector::DistSquared(OwnerLocation, Candidate->GetActorLocation());
			break;
		case EMMOTargetPriority::LowestHealth:
			Value = -Candidate->GetHealth();
			break;
		case EMMOTargetPriority::HighestThreat:
			Value = Candidate->GetThreatAgainst(Owner);
			break;
		case EMMOTargetPriority::ConeInFront:
		{
			const FVector DirToTarget = (Candidate->GetActorLocation() - OwnerLocation).GetSafeNormal();
			const FVector Forward = Owner ? Owner->GetActorForwardVector() : FVector::ForwardVector;
			const float Dot = FVector::DotProduct(Forward, DirToTarget);
			Value = Dot;
			break;
		}
		case EMMOTargetPriority::CurrentTarget:
			if (Candidate == HardTarget)
			{
				return Candidate;
			}
			continue;
		default:
			Value = -FVector::DistSquared(OwnerLocation, Candidate->GetActorLocation());
			break;
		}

		if (!BestTarget || Value > BestValue)
		{
			BestValue = Value;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

TArray<AMMOCharacterBase*> UMMOTargetingSystemComponent::FindTargetsInCone(float HalfAngle, float Range, const FMMOTargetingFilter& Filter) const
{
	TArray<AMMOCharacterBase*> Results;
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return Results;
	}

	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector Forward = Owner->GetActorForwardVector();

	for (AMMOCharacterBase* Candidate : CachedEnemyList)
	{
		if (!Candidate || !IsValidTarget(Candidate, Filter))
		{
			continue;
		}

		const FVector ToTarget = (Candidate->GetActorLocation() - OwnerLocation).GetSafeNormal();
		const float Dot = FVector::DotProduct(Forward, ToTarget);
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

		if (Angle <= HalfAngle && IsInRange(Candidate, Range))
		{
			Results.Add(Candidate);
		}
	}

	return Results;
}

TArray<AMMOCharacterBase*> UMMOTargetingSystemComponent::FindTargetsInRadius(const FVector& Origin, float Radius, const FMMOTargetingFilter& Filter) const
{
	TArray<AMMOCharacterBase*> Results;

	for (AMMOCharacterBase* Candidate : CachedEnemyList)
	{
		if (!Candidate || !IsValidTarget(Candidate, Filter))
		{
			continue;
		}

		if (FVector::DistSquared(Origin, Candidate->GetActorLocation()) <= Radius * Radius)
		{
			Results.Add(Candidate);
		}
	}

	return Results;
}

bool UMMOTargetingSystemComponent::ClickToTarget(APlayerController* PC, EMMOTargetTypeFilter TargetFilter)
{
	if (!PC)
	{
		return false;
	}

	FVector WorldOrigin;
	FVector WorldDirection;
	if (!PC->DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
	{
		return false;
	}

	const float TraceDistance = 10000.0f;
	const FVector TraceEnd = WorldOrigin + WorldDirection * TraceDistance;

	FHitResult Hit;
	FMMOTargetingFilter Filter;
	Filter.TypeFilter = TargetFilter;
	Filter.bRequireLineOfSight = false;
	Filter.bAliveOnly = true;

	if (TraceForTarget(WorldOrigin, TraceEnd, Hit, TargetFilter))
	{
		if (AMMOCharacterBase* CharTarget = Cast<AMMOCharacterBase>(Hit.GetActor()))
		{
			SetHardTarget(CharTarget);
			return true;
		}
	}

	return false;
}

bool UMMOTargetingSystemComponent::TraceForTarget(const FVector& Start, const FVector& End, FHitResult& OutHit, EMMOTargetTypeFilter TargetFilter) const
{
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = false;
	QueryParams.AddIgnoredActor(GetOwner());

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	UWorld* World = Owner->GetWorld();
	if (!World)
	{
		return false;
	}

	if (!World->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, QueryParams))
	{
		return false;
	}

	AMMOCharacterBase* HitCharacter = Cast<AMMOCharacterBase>(OutHit.GetActor());
	if (!HitCharacter)
	{
		return false;
	}

	FMMOTargetingFilter Filter;
	Filter.TypeFilter = TargetFilter;
	Filter.bAliveOnly = true;
	Filter.bRequireLineOfSight = false;

	return IsValidTarget(HitCharacter, Filter);
}

bool UMMOTargetingSystemComponent::IsValidTarget(AMMOCharacterBase* Target, const FMMOTargetingFilter& Filter) const
{
	if (!Target)
	{
		return false;
	}

	if (Target == GetOwner())
	{
		return Filter.bIncludeSelf;
	}

	if (Filter.bAliveOnly && !Target->IsAlive())
	{
		return false;
	}

	if (Filter.MaxRange > 0.0f && !IsInRange(Target, Filter.MaxRange))
	{
		return false;
	}

	if (Filter.bRequireLineOfSight && !HasLineOfSight(Target))
	{
		return false;
	}

	if (Filter.RequiredTags.IsValid() && !Target->GetGameplayTags().HasAll(Filter.RequiredTags))
	{
		return false;
	}

	if (Filter.BlockedTags.IsValid() && Target->GetGameplayTags().HasAny(Filter.BlockedTags))
	{
		return false;
	}

	switch (Filter.TypeFilter)
	{
	case EMMOTargetTypeFilter::Enemy:
		if (!IsEnemy(Target))
		{
			return false;
		}
		break;
	case EMMOTargetTypeFilter::Friendly:
		if (!IsFriendly(Target))
		{
			return false;
		}
		break;
	case EMMOTargetTypeFilter::Self:
		if (Target != GetOwner())
		{
			return false;
		}
		break;
	case EMMOTargetTypeFilter::Any:
		break;
	default:
		break;
	}

	return true;
}

bool UMMOTargetingSystemComponent::HasLineOfSight(AMMOCharacterBase* Target) const
{
	if (!Target || !GetOwner())
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(Target);

	const AActor* Owner = GetOwner();
	UWorld* World = Owner->GetWorld();
	if (!World)
	{
		return false;
	}

	return !World->LineTraceSingleByChannel(Hit, Owner->GetActorLocation(), Target->GetActorLocation(), ECC_Visibility, QueryParams);
}

bool UMMOTargetingSystemComponent::IsInRange(AMMOCharacterBase* Target, float Range) const
{
	if (!Target || !GetOwner())
	{
		return false;
	}

	return FVector::DistSquared(GetOwner()->GetActorLocation(), Target->GetActorLocation()) <= Range * Range;
}

bool UMMOTargetingSystemComponent::IsValidTargetForAbility(AMMOCharacterBase* Target, class UMMOAbilityBase* Ability) const
{
	if (!Ability || !Target)
	{
		return false;
	}

	FMMOTargetingFilter Filter;
	switch (Ability->TargetType)
	{
	case EMMOTargetType::Self:
		Filter.TypeFilter = EMMOTargetTypeFilter::Self;
		break;
	case EMMOTargetType::SingleEnemy:
		Filter.TypeFilter = EMMOTargetTypeFilter::Enemy;
		break;
	case EMMOTargetType::SingleAlly:
		Filter.TypeFilter = EMMOTargetTypeFilter::Friendly;
		break;
	default:
		Filter.TypeFilter = EMMOTargetTypeFilter::Any;
		break;
	}

	Filter.MaxRange = Ability->Range;
	Filter.bRequireLineOfSight = true;
	Filter.bAliveOnly = true;

	return IsValidTarget(Target, Filter);
}

AMMOCharacterBase* UMMOTargetingSystemComponent::GetEffectiveTarget() const
{
	if (HardTarget)
	{
		return HardTarget;
	}
	return SoftTarget;
}

void UMMOTargetingSystemComponent::RefreshEnemyList()
{
	CachedEnemyList.Empty();

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(Owner->GetWorld(), AMMOCharacterBase::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (AMMOCharacterBase* Char = Cast<AMMOCharacterBase>(Actor))
		{
			if (Char != Owner)
			{
				CachedEnemyList.Add(Char);
			}
		}
	}
}

bool UMMOTargetingSystemComponent::IsEnemy(AMMOCharacterBase* Target) const
{
	if (!Target)
	{
		return false;
	}

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// If the target has a combat manager, check PvP/team status
	UMMOCombatManagerComponent* OwnerCombat = Owner->FindComponentByClass<UMMOCombatManagerComponent>();
	UMMOCombatManagerComponent* TargetCombat = Target->FindComponentByClass<UMMOCombatManagerComponent>();

	if (OwnerCombat && TargetCombat)
	{
		return !OwnerCombat->CanAttack(Target);
	}

	// Fallback: check if different team/group via tags
	return !Target->GetGameplayTags().HasAny(Owner->GetGameplayTags());
}

bool UMMOTargetingSystemComponent::IsFriendly(AMMOCharacterBase* Target) const
{
	if (!Target)
	{
		return false;
	}

	if (Target == GetOwner())
	{
		return true;
	}

	return !IsEnemy(Target);
}

#include "Abilities/MMOBuffSystem.h"
#include "Abilities/MMOBuff.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "MMOCharacterBase.h"

UMMOBuffSystemComponent::UMMOBuffSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	SetIsReplicatedByDefault(true);
}

void UMMOBuffSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMMOBuffSystemComponent, ActiveBuffEntries);
}

void UMMOBuffSystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMMOBuffSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!HasAuthority())
	{
		return;
	}

	for (int32 i = ActiveBuffEntries.Num() - 1; i >= 0; --i)
	{
		FMMOActiveBuffEntry& Entry = ActiveBuffEntries[i];
		if (!Entry.BuffInstance)
		{
			ActiveBuffEntries.RemoveAt(i);
			continue;
		}

		// Tick the buff
		Entry.BuffInstance->OnTick(Entry.BuffInstance->TargetActor.Get(), DeltaTime);

		// Handle expiration
		if (!Entry.bInfinite && Entry.BuffInstance->IsExpired())
		{
			ExpireBuff(Entry);
			ActiveBuffEntries.RemoveAt(i);
			continue;
		}

		// Process periodic effects
		if (Entry.BuffInstance->PeriodicEffect.TickInterval > 0.0f)
		{
			ProcessPeriodicEffect(Entry, DeltaTime);
		}
	}
}

void UMMOBuffSystemComponent::OnRep_ActiveBuffs()
{
	// Client-side update for UI
}

UMMOBuff* UMMOBuffSystemComponent::ApplyBuff(TSubclassOf<UMMOBuff> BuffClass, AActor* Source, AActor* Target)
{
	if (!BuffClass || !Target || !HasAuthority())
	{
		return nullptr;
	}

	if (IsImmuneToBuff(BuffClass.GetDefaultObject()))
	{
		return nullptr;
	}

	UMMOBuff* ExistingBuff = FindBuffByID(BuffClass.GetDefaultObject()->BuffID);
	if (ExistingBuff)
	{
		switch (ExistingBuff->StackType)
		{
		case EMMOBuffStackType::NoStack:
			// Can't re-apply; refresh duration instead
			RefreshBuff(ExistingBuff);
			return ExistingBuff;
		case EMMOBuffStackType::Single:
			RefreshBuff(ExistingBuff);
			return ExistingBuff;
		case EMMOBuffStackType::Multi:
			RefreshBuff(ExistingBuff);
			ExistingBuff->AddStack(1);
			return ExistingBuff;
		case EMMOBuffStackType::Limited:
			if (ExistingBuff->CurrentStacks >= ExistingBuff->MaxStacks)
			{
				RefreshBuff(ExistingBuff);
				return ExistingBuff;
			}
			ExistingBuff->AddStack(1);
			RefreshBuff(ExistingBuff);
			return ExistingBuff;
		default:
			break;
		}
	}

	UMMOBuff* NewBuff = NewObject<UMMOBuff>(this, BuffClass);
	if (!NewBuff)
	{
		return nullptr;
	}

	Internal_ApplyBuff(NewBuff, Source, Target);
	return NewBuff;
}

bool UMMOBuffSystemComponent::RemoveBuff(UMMOBuff* BuffInstance, bool bDispelled)
{
	if (!BuffInstance)
	{
		return false;
	}

	const int32 EntryIndex = FindBuffEntryIndexByInstance(BuffInstance);
	if (EntryIndex == INDEX_NONE)
	{
		return false;
	}

	InternalRemoveBuff(EntryIndex, bDispelled);
	return true;
}

bool UMMOBuffSystemComponent::RemoveBuffByID(FName BuffID, bool bDispelled)
{
	const int32 EntryIndex = FindBuffEntryIndex(BuffID);
	if (EntryIndex == INDEX_NONE)
	{
		return false;
	}

	InternalRemoveBuff(EntryIndex, bDispelled);
	return true;
}

void UMMOBuffSystemComponent::RemoveAllBuffs(bool bRemoveBeneficial, bool bRemoveHarmful, bool bDispelled)
{
	for (int32 i = ActiveBuffEntries.Num() - 1; i >= 0; --i)
	{
		if (!ActiveBuffEntries[i].BuffInstance)
		{
			continue;
		}

		const EMMOBuffType Type = ActiveBuffEntries[i].BuffInstance->BuffType;
		if ((Type == EMMOBuffType::Beneficial && bRemoveBeneficial) ||
			(Type == EMMOBuffType::Harmful && bRemoveHarmful) ||
			(Type == EMMOBuffType::Hidden))
		{
			InternalRemoveBuff(i, bDispelled);
		}
	}
}

void UMMOBuffSystemComponent::RemoveAllBuffsByTag(const FGameplayTagContainer& Tags, bool bDispelled)
{
	for (int32 i = ActiveBuffEntries.Num() - 1; i >= 0; --i)
	{
		if (ActiveBuffEntries[i].BuffInstance &&
			ActiveBuffEntries[i].BuffInstance->BuffTags.HasAny(Tags))
		{
			InternalRemoveBuff(i, bDispelled);
		}
	}
}

void UMMOBuffSystemComponent::RemoveBuffsByType(EMMOBuffType BuffType, bool bDispelled)
{
	for (int32 i = ActiveBuffEntries.Num() - 1; i >= 0; --i)
	{
		if (ActiveBuffEntries[i].BuffInstance &&
			ActiveBuffEntries[i].BuffInstance->BuffType == BuffType)
		{
			InternalRemoveBuff(i, bDispelled);
		}
	}
}

void UMMOBuffSystemComponent::RefreshBuff(UMMOBuff* BuffInstance)
{
	if (!BuffInstance)
	{
		return;
	}

	const int32 EntryIndex = FindBuffEntryIndexByInstance(BuffInstance);
	if (EntryIndex == INDEX_NONE)
	{
		return;
	}

	ActiveBuffEntries[EntryIndex].TimeRemaining = BuffInstance->Duration;
	ActiveBuffEntries[EntryIndex].Duration = BuffInstance->Duration;
	BuffInstance->RefreshDuration();
}

bool UMMOBuffSystemComponent::HasBuff(FName BuffID) const
{
	return FindBuffEntryIndex(BuffID) != INDEX_NONE;
}

UMMOBuff* UMMOBuffSystemComponent::FindBuffByID(FName BuffID) const
{
	const int32 EntryIndex = FindBuffEntryIndex(BuffID);
	if (EntryIndex == INDEX_NONE)
	{
		return nullptr;
	}
	return ActiveBuffEntries[EntryIndex].BuffInstance;
}

TArray<UMMOBuff*> UMMOBuffSystemComponent::FindBuffsByTag(const FGameplayTagContainer& Tags) const
{
	TArray<UMMOBuff*> Results;
	for (const FMMOActiveBuffEntry& Entry : ActiveBuffEntries)
	{
		if (Entry.BuffInstance && Entry.BuffInstance->BuffTags.HasAny(Tags))
		{
			Results.Add(Entry.BuffInstance);
		}
	}
	return Results;
}

int32 UMMOBuffSystemComponent::GetBuffStackCount(FName BuffID) const
{
	const int32 EntryIndex = FindBuffEntryIndex(BuffID);
	if (EntryIndex == INDEX_NONE)
	{
		return 0;
	}
	return ActiveBuffEntries[EntryIndex].StackCount;
}

EMMODispelResult UMMOBuffSystemComponent::DispelBuff(EMMODispelType DispelType, int32 NumBuffsToDispel)
{
	int32 Dispelled = 0;
	for (int32 i = ActiveBuffEntries.Num() - 1; i >= 0 && Dispelled < NumBuffsToDispel; --i)
	{
		if (!ActiveBuffEntries[i].BuffInstance)
		{
			continue;
		}

		if (ActiveBuffEntries[i].BuffInstance->DispelType == DispelType ||
			DispelType == EMMODispelType::None)
		{
			InternalRemoveBuff(i, true);
			Dispelled++;
		}
	}

	return Dispelled > 0 ? EMMODispelResult::Success : EMMODispelResult::FailedNotFound;
}

EMMODispelResult UMMOBuffSystemComponent::DispelBuffsByType(EMMOBuffType BuffType, int32 NumBuffsToDispel)
{
	int32 Dispelled = 0;
	for (int32 i = ActiveBuffEntries.Num() - 1; i >= 0 && Dispelled < NumBuffsToDispel; --i)
	{
		if (!ActiveBuffEntries[i].BuffInstance)
		{
			continue;
		}

		if (ActiveBuffEntries[i].BuffInstance->BuffType == BuffType)
		{
			InternalRemoveBuff(i, true);
			Dispelled++;
		}
	}

	return Dispelled > 0 ? EMMODispelResult::Success : EMMODispelResult::FailedNotFound;
}

bool UMMOBuffSystemComponent::IsImmuneToBuff(const UMMOBuff* Buff) const
{
	if (!Buff)
	{
		return true;
	}

	for (const FGameplayTag& ImmunityTag : ActiveImmunityCategories)
	{
		if (Buff->ImmunityCategories.HasTag(ImmunityTag))
		{
			return true;
		}
	}
	return false;
}

void UMMOBuffSystemComponent::AddImmunityCategory(const FGameplayTag& ImmunityTag, float Duration)
{
	if (!ActiveImmunityCategories.Contains(ImmunityTag))
	{
		ActiveImmunityCategories.Add(ImmunityTag);
	}
}

void UMMOBuffSystemComponent::RemoveImmunityCategory(const FGameplayTag& ImmunityTag)
{
	ActiveImmunityCategories.Remove(ImmunityTag);
}

void UMMOBuffSystemComponent::ProcessPeriodicEffect(const FMMOActiveBuffEntry& Entry, float DeltaTime)
{
	// Periodic processing is handled in buff Tick for flexibility
}

void UMMOBuffSystemComponent::ExpireBuff(FMMOActiveBuffEntry& Entry)
{
	if (Entry.BuffInstance)
	{
		Entry.BuffInstance->OnExpired(Entry.BuffInstance->TargetActor.Get());
		OnBuffExpired.Broadcast(Entry.BuffInstance, Entry.BuffInstance->TargetActor.Get());
	}
}

void UMMOBuffSystemComponent::InternalRemoveBuff(int32 Index, bool bDispelled)
{
	if (!ActiveBuffEntries.IsValidIndex(Index))
	{
		return;
	}

	FMMOActiveBuffEntry& Entry = ActiveBuffEntries[Index];
	if (Entry.BuffInstance)
	{
		Entry.BuffInstance->OnRemoved(Entry.BuffInstance->TargetActor.Get(), bDispelled);
		OnBuffRemoved.Broadcast(Entry.BuffInstance, Entry.BuffInstance->TargetActor.Get(), bDispelled);
	}

	ActiveBuffEntries.RemoveAt(Index);
}

void UMMOBuffSystemComponent::Internal_ApplyBuff(UMMOBuff* BuffInstance, AActor* Source, AActor* Target)
{
	if (!BuffInstance)
	{
		return;
	}

	BuffInstance->OwningSystem = this;
	BuffInstance->SourceActor = Source;
	BuffInstance->TargetActor = Target;

	FMMOActiveBuffEntry NewEntry;
	NewEntry.BuffInstance = BuffInstance;
	NewEntry.Duration = BuffInstance->Duration;
	NewEntry.TimeRemaining = BuffInstance->bInfiniteDuration ? -1.0f : BuffInstance->Duration;
	NewEntry.StackCount = 1;
	NewEntry.bInfinite = BuffInstance->bInfiniteDuration;
	NewEntry.SourceActor = Source;
	NewEntry.TickCount = 0;
	ActiveBuffEntries.Add(NewEntry);

	BuffInstance->OnApplied(Source, Target);
	OnBuffApplied.Broadcast(BuffInstance, Source, Target);
}

int32 UMMOBuffSystemComponent::FindBuffEntryIndex(FName BuffID) const
{
	for (int32 i = 0; i < ActiveBuffEntries.Num(); ++i)
	{
		if (ActiveBuffEntries[i].BuffInstance && ActiveBuffEntries[i].BuffInstance->BuffID == BuffID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UMMOBuffSystemComponent::FindBuffEntryIndexByInstance(const UMMOBuff* BuffInstance) const
{
	for (int32 i = 0; i < ActiveBuffEntries.Num(); ++i)
	{
		if (ActiveBuffEntries[i].BuffInstance == BuffInstance)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

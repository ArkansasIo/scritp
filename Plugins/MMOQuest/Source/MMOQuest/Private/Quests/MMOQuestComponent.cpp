#include "Quests/MMOQuestComponent.h"
#include "Quests/MMOQuestBase.h"
#include "Data/MMOQuestDatabase.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

UMMOQuestComponent::UMMOQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	SetIsReplicatedByDefault(true);
}

void UMMOQuestComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMMOQuestComponent, ActiveQuests, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UMMOQuestComponent, CompletedQuests, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UMMOQuestComponent, DailyQuestResets, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UMMOQuestComponent, WeeklyQuestResets, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UMMOQuestComponent, TrackedQuestID, COND_OwnerOnly);
}

void UMMOQuestComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMMOQuestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMMOQuestComponent::OnRep_ActiveQuests()
{
	OnQuestAccepted.Broadcast(nullptr, INDEX_NONE);
}

void UMMOQuestComponent::OnRep_CompletedQuests()
{
	OnQuestCompleted.Broadcast(nullptr, INDEX_NONE);
}

bool UMMOQuestComponent::CanAcceptQuest(FGameplayTag QuestID, FString& OutReason) const
{
	if (ActiveQuests.Num() >= MaxActiveQuests)
	{
		OutReason = TEXT("Quest journal is full.");
		return false;
	}
	if (HasActiveQuest(QuestID))
	{
		OutReason = TEXT("Quest already active.");
		return false;
	}
	const UMMOQuestBase* QuestData = GetQuestData(QuestID);
	if (!QuestData)
	{
		OutReason = TEXT("Quest not found in database.");
		return false;
	}
	for (const FMMOActiveQuestState& State : ActiveQuests)
	{
		if (State.QuestID == QuestID)
		{
			OutReason = TEXT("Quest already in journal.");
			return false;
		}
	}
	return true;
}

void UMMOQuestComponent::ServerAcceptQuest_Implementation(FGameplayTag QuestID)
{
	FString Reason;
	if (!CanAcceptQuest(QuestID, Reason))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot accept quest %s: %s"), *QuestID.ToString(), *Reason);
		return;
	}
	const UMMOQuestBase* QuestData = GetQuestData(QuestID);
	if (QuestData)
	{
		InternalAcceptQuest(QuestData);
	}
}

bool UMMOQuestComponent::ServerAcceptQuest_Validate(FGameplayTag QuestID)
{
	return QuestID.IsValid();
}

void UMMOQuestComponent::InternalAcceptQuest(const UMMOQuestBase* Quest)
{
	if (!Quest)
	{
		return;
	}

	FMMOActiveQuestState NewState;
	NewState.QuestID = Quest->QuestID;
	NewState.QuestSlot = ActiveQuests.Num();
	NewState.AcceptedTime = FPlatformTime::Seconds();

	for (const FMMOQuestObjective& Obj : Quest->Objectives)
	{
		FMMOQuestObjective ProgressCopy = Obj;
		ProgressCopy.CurrentCount = 0;
		NewState.ObjectiveProgress.Add(ProgressCopy);
	}

	ActiveQuests.Add(NewState);
	OnQuestAccepted.Broadcast(const_cast<UMMOQuestBase*>(Quest), NewState.QuestSlot);

	ClientQuestStateUpdated(Quest->QuestID, Quest->QuestType, true, false, false);

	if (Quest->bAutoAccept)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Quest %s auto-accepted."), *Quest->QuestID.ToString());
	}

	AddReferencedQuest(const_cast<UMMOQuestBase*>(Quest));
}

void UMMOQuestComponent::ServerTurnInQuest_Implementation(FGameplayTag QuestID)
{
	const int32 Slot = GetActiveQuestSlot(QuestID);
	if (Slot == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Turn in failed: quest %s not active."), *QuestID.ToString());
		return;
	}

	FMMOActiveQuestState& State = ActiveQuests[Slot];
	if (!State.IsFullyComplete())
	{
		UE_LOG(LogTemp, Warning, TEXT("Turn in failed: quest %s objectives not complete."), *QuestID.ToString());
		return;
	}

	const UMMOQuestBase* QuestData = GetQuestData(QuestID);
	if (QuestData)
	{
		InternalCompleteQuest(QuestData);
	}
}

bool UMMOQuestComponent::ServerTurnInQuest_Validate(FGameplayTag QuestID)
{
	return QuestID.IsValid();
}

void UMMOQuestComponent::InternalCompleteQuest(const UMMOQuestBase* Quest)
{
	if (!Quest)
	{
		return;
	}

	const int32 Slot = GetActiveQuestSlot(Quest->QuestID);
	if (Slot == INDEX_NONE)
	{
		return;
	}

	const FMMOActiveQuestState& State = ActiveQuests[Slot];
	GrantQuestRewards(const_cast<UMMOQuestBase*>(Quest), INDEX_NONE);

	FMMOCompletedQuestEntry CompletedEntry;
	CompletedEntry.QuestID = Quest->QuestID;
	CompletedEntry.TimesCompleted = 1;
	CompletedEntry.LastCompletedTime = FPlatformTime::Seconds();

	bool bFound = false;
	for (FMMOCompletedQuestEntry& Entry : CompletedQuests)
	{
		if (Entry.QuestID == Quest->QuestID)
		{
			Entry.TimesCompleted++;
			Entry.LastCompletedTime = CompletedEntry.LastCompletedTime;
			bFound = true;
			break;
		}
	}
	if (!bFound)
	{
		if (CompletedQuests.Num() >= MaxCompletedHistory)
		{
			CompletedQuests.RemoveAt(0);
		}
		CompletedQuests.Add(CompletedEntry);
	}

	ActiveQuests.RemoveAt(Slot);
	for (int32 i = Slot; i < ActiveQuests.Num(); ++i)
	{
		ActiveQuests[i].QuestSlot = i;
	}

	OnQuestCompleted.Broadcast(const_cast<UMMOQuestBase*>(Quest), Slot);
	ClientQuestStateUpdated(Quest->QuestID, Quest->QuestType, false, true, false);
}

void UMMOQuestComponent::ServerAbandonQuest_Implementation(FGameplayTag QuestID)
{
	const int32 Slot = GetActiveQuestSlot(QuestID);
	if (Slot == INDEX_NONE)
	{
		return;
	}

	const UMMOQuestBase* QuestData = GetQuestData(QuestID);
	if (QuestData && QuestData->bCancelable)
	{
		InternalAbandonQuest(QuestData);
	}
}

bool UMMOQuestComponent::ServerAbandonQuest_Validate(FGameplayTag QuestID)
{
	return QuestID.IsValid();
}

void UMMOQuestComponent::InternalAbandonQuest(const UMMOQuestBase* Quest)
{
	if (!Quest)
	{
		return;
	}

	const int32 Slot = GetActiveQuestSlot(Quest->QuestID);
	if (Slot == INDEX_NONE)
	{
		return;
	}

	ActiveQuests.RemoveAt(Slot);
	for (int32 i = Slot; i < ActiveQuests.Num(); ++i)
	{
		ActiveQuests[i].QuestSlot = i;
	}

	OnQuestAbandoned.Broadcast(const_cast<UMMOQuestBase*>(Quest), Slot);
	ClientQuestStateUpdated(Quest->QuestID, Quest->QuestType, false, false, true);
}

void UMMOQuestComponent::ServerUpdateObjective_Implementation(FGameplayTag QuestID, int32 ObjectiveIndex, int32 Amount)
{
	const int32 Slot = GetActiveQuestSlot(QuestID);
	if (Slot == INDEX_NONE)
	{
		return;
	}

	FMMOActiveQuestState& State = ActiveQuests[Slot];
	if (ObjectiveIndex < 0 || ObjectiveIndex >= State.ObjectiveProgress.Num())
	{
		return;
	}

	FMMOQuestObjective& Obj = State.ObjectiveProgress[ObjectiveIndex];
	const int32 OldCount = Obj.CurrentCount;
	Obj.AddProgress(Amount);

	if (OldCount != Obj.CurrentCount)
	{
		OnObjectiveUpdated.Broadcast(GetQuestData(QuestID), ObjectiveIndex, Obj.CurrentCount);

		if (State.IsFullyComplete())
		{
			UE_LOG(LogTemp, Log, TEXT("Quest %s all objectives complete."), *QuestID.ToString());
		}
	}
}

bool UMMOQuestComponent::ServerUpdateObjective_Validate(FGameplayTag QuestID, int32 ObjectiveIndex, int32 Amount)
{
	return QuestID.IsValid() && ObjectiveIndex >= 0 && Amount > 0;
}

void UMMOQuestComponent::ClientQuestStateUpdated_Implementation(FGameplayTag QuestID, EQuestType QuestType, bool bAccepted, bool bCompleted, bool bAbandoned)
{
	UE_LOG(LogTemp, Verbose, TEXT("Quest state updated: %s accepted=%d completed=%d abandoned=%d"),
		*QuestID.ToString(), bAccepted, bCompleted, bAbandoned);
}

EQuestGiverStatus UMMOQuestComponent::QueryQuestGiverStatus(FGameplayTag NPCID) const
{
	for (const FMMOActiveQuestState& State : ActiveQuests)
	{
		const UMMOQuestBase* Quest = GetQuestData(State.QuestID);
		if (Quest && (Quest->QuestGiverNPC == NPCID || Quest->TurnInNPC == NPCID))
		{
			if (State.IsFullyComplete())
			{
				return EQuestGiverStatus::Completed;
			}
			return EQuestGiverStatus::InProgress;
		}
	}

	for (const FMMOCompletedQuestEntry& Entry : CompletedQuests)
	{
		const UMMOQuestBase* Quest = GetQuestData(Entry.QuestID);
		if (Quest)
		{
			if (Quest->QuestGiverNPC == NPCID || Quest->TurnInNPC == NPCID)
			{
				return EQuestGiverStatus::Completed;
			}
		}
	}

	if (UWorld* World = GetWorld())
	{
		if (UMMOQuestDatabase* DB = World->GetSubsystem<UMMOQuestDatabase>())
		{
			TArray<UMMOQuestBase*> NPCQuests = DB->GetQuestsByGiverNPC(NPCID);
			if (NPCQuests.Num() > 0)
			{
				for (UMMOQuestBase* Quest : NPCQuests)
				{
					if (DB->IsQuestAvailableForPlayer(Quest->QuestID, this))
					{
						return EQuestGiverStatus::Available;
					}
				}
			}
		}
	}

	return EQuestGiverStatus::None;
}

void UMMOQuestComponent::SetQuestTracked(FGameplayTag QuestID, bool bTracked)
{
	const int32 Slot = GetActiveQuestSlot(QuestID);
	if (Slot != INDEX_NONE)
	{
		ActiveQuests[Slot].bIsTracked = bTracked;
		if (bTracked)
		{
			TrackedQuestID = QuestID;
		}
		else if (TrackedQuestID == QuestID)
		{
			TrackedQuestID = FGameplayTag();
		}
		OnQuestTrackedChanged.Broadcast(GetQuestData(QuestID), bTracked);
	}
}

bool UMMOQuestComponent::HasActiveQuest(FGameplayTag QuestID) const
{
	for (const FMMOActiveQuestState& State : ActiveQuests)
	{
		if (State.QuestID == QuestID)
		{
			return true;
		}
	}
	return false;
}

bool UMMOQuestComponent::HasCompletedQuest(FGameplayTag QuestID) const
{
	for (const FMMOCompletedQuestEntry& Entry : CompletedQuests)
	{
		if (Entry.QuestID == QuestID)
		{
			return true;
		}
	}
	return false;
}

int32 UMMOQuestComponent::GetActiveQuestSlot(FGameplayTag QuestID) const
{
	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		if (ActiveQuests[i].QuestID == QuestID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

FMMOActiveQuestState UMMOQuestComponent::GetActiveQuestState(FGameplayTag QuestID) const
{
	for (const FMMOActiveQuestState& State : ActiveQuests)
	{
		if (State.QuestID == QuestID)
		{
			return State;
		}
	}
	return FMMOActiveQuestState();
}

TArray<FMMOActiveQuestState> UMMOQuestComponent::GetActiveQuestsByType(EQuestType Type) const
{
	TArray<FMMOActiveQuestState> Results;
	for (const FMMOActiveQuestState& State : ActiveQuests)
	{
		const UMMOQuestBase* Quest = GetQuestData(State.QuestID);
		if (Quest && Quest->QuestType == Type)
		{
			Results.Add(State);
		}
	}
	return Results;
}

UMMOQuestBase* UMMOQuestComponent::GetQuestData(FGameplayTag QuestID) const
{
	if (const UMMOQuestBase* const* Found = QuestDataCache.Find(QuestID))
	{
		return const_cast<UMMOQuestBase*>(*Found);
	}
	if (UWorld* World = GetWorld())
	{
		if (UMMOQuestDatabase* DB = World->GetSubsystem<UMMOQuestDatabase>())
		{
			UMMOQuestBase* Quest = DB->GetQuestByID(QuestID);
			if (Quest)
			{
				const_cast<UMMOQuestComponent*>(this)->QuestDataCache.Add(QuestID, Quest);
			}
			return Quest;
		}
	}
	return nullptr;
}

void UMMOQuestComponent::ResetDailyQuests()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	DailyQuestResets++;
	ActiveQuests.RemoveAll([this](const FMMOActiveQuestState& State)
	{
		const UMMOQuestBase* Quest = GetQuestData(State.QuestID);
		return Quest && Quest->QuestType == EQuestType::Daily;
	});
	UE_LOG(LogTemp, Log, TEXT("Daily quests reset. Total resets: %d"), DailyQuestResets);
}

void UMMOQuestComponent::ResetWeeklyQuests()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	WeeklyQuestResets++;
	ActiveQuests.RemoveAll([this](const FMMOActiveQuestState& State)
	{
		const UMMOQuestBase* Quest = GetQuestData(State.QuestID);
		return Quest && Quest->QuestType == EQuestType::Weekly;
	});
	UE_LOG(LogTemp, Log, TEXT("Weekly quests reset. Total resets: %d"), WeeklyQuestResets);
}

bool UMMOQuestComponent::IsQuestOnCooldown(const UMMOQuestBase* Quest) const
{
	if (!Quest || !Quest->bRepeatable || Quest->RepeatCooldownSeconds <= 0)
	{
		return false;
	}
	for (const FMMOCompletedQuestEntry& Entry : CompletedQuests)
	{
		if (Entry.QuestID == Quest->QuestID)
		{
			const double Elapsed = FPlatformTime::Seconds() - Entry.LastCompletedTime;
			return Elapsed < (double)Quest->RepeatCooldownSeconds;
		}
	}
	return false;
}

void UMMOQuestComponent::GrantQuestRewards(UMMOQuestBase* Quest, int32 ChoiceIndex)
{
	if (!Quest)
	{
		return;
	}

	GrantExperience(Quest->Rewards.ExperienceReward);
	GrantCurrency(Quest->Rewards.CurrencyReward);
	GrantItems(Quest->Rewards.FixedItems);
	GrantReputation(Quest->Rewards.ReputationReward);
	GrantSpells(Quest->Rewards.SpellUnlocks);

	if (Quest->Rewards.ChoiceItems.Num() > 0)
	{
		int32 Index = ChoiceIndex;
		if (Index < 0 || Index >= Quest->Rewards.ChoiceItems.Num())
		{
			Index = 0;
		}
		GrantChoiceItems(Quest->Rewards.ChoiceItems, Index);
	}

	if (Quest->NextQuestID.IsValid())
	{
		const UMMOQuestBase* NextQuest = GetQuestData(Quest->NextQuestID);
		if (NextQuest && NextQuest->bAutoAccept)
		{
			InternalAcceptQuest(NextQuest);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Rewards granted for quest %s."), *Quest->QuestID.ToString());
}

void UMMOQuestComponent::ProgressObjective(FGameplayTag QuestID, FGameplayTag TargetID, int32 Amount)
{
	const int32 Slot = GetActiveQuestSlot(QuestID);
	if (Slot == INDEX_NONE)
	{
		return;
	}

	FMMOActiveQuestState& State = ActiveQuests[Slot];
	for (int32 i = 0; i < State.ObjectiveProgress.Num(); ++i)
	{
		FMMOQuestObjective& Obj = State.ObjectiveProgress[i];
		if (Obj.TargetID == TargetID)
		{
			ServerUpdateObjective(QuestID, i, Amount);
			return;
		}
	}
}

void UMMOQuestComponent::ProgressObjectiveByType(FGameplayTag QuestID, EQuestObjectiveType Type, FGameplayTag TargetID, int32 Amount)
{
	const int32 Slot = GetActiveQuestSlot(QuestID);
	if (Slot == INDEX_NONE)
	{
		return;
	}

	FMMOActiveQuestState& State = ActiveQuests[Slot];
	for (int32 i = 0; i < State.ObjectiveProgress.Num(); ++i)
	{
		FMMOQuestObjective& Obj = State.ObjectiveProgress[i];
		if (Obj.ObjectiveType == Type && Obj.TargetID == TargetID)
		{
			ServerUpdateObjective(QuestID, i, Amount);
			return;
		}
	}
}

bool UMMOQuestComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void UMMOQuestComponent::AddReferencedQuest(UMMOQuestBase* Quest)
{
	if (Quest)
	{
		QuestDataCache.Add(Quest->QuestID, Quest);
	}
}

void UMMOQuestComponent::GrantExperience(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		if (AController* Ctrl = Char->GetController())
		{
			UE_LOG(LogTemp, Log, TEXT("Granting %d experience to %s."), Amount, *GetOwner()->GetName());
		}
	}
}

void UMMOQuestComponent::GrantCurrency(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Granting %d currency to %s."), Amount, *GetOwner()->GetName());
}

void UMMOQuestComponent::GrantItems(const TMap<FGameplayTag, int32>& Items)
{
	for (const TPair<FGameplayTag, int32>& Entry : Items)
	{
		UE_LOG(LogTemp, Log, TEXT("Granting item %s x%d to %s."), *Entry.Key.ToString(), Entry.Value, *GetOwner()->GetName());
	}
}

void UMMOQuestComponent::GrantReputation(const TMap<FName, int32>& RepChanges)
{
	for (const TPair<FName, int32>& Entry : RepChanges)
	{
		UE_LOG(LogTemp, Log, TEXT("Reputation change %s %+d for %s."), *Entry.Key.ToString(), Entry.Value, *GetOwner()->GetName());
	}
}

void UMMOQuestComponent::GrantSpells(const TArray<FGameplayTag>& SpellTags)
{
	for (const FGameplayTag& SpellTag : SpellTags)
	{
		UE_LOG(LogTemp, Log, TEXT("Unlocking spell %s for %s."), *SpellTag.ToString(), *GetOwner()->GetName());
	}
}

void UMMOQuestComponent::GrantChoiceItems(const TArray<TMap<FGameplayTag, int32>>& Choices, int32 ChoiceIndex)
{
	if (ChoiceIndex >= 0 && ChoiceIndex < Choices.Num())
	{
		GrantItems(Choices[ChoiceIndex]);
	}
}

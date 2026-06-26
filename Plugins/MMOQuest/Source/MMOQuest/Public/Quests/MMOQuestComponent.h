#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MMOQuestObjective.h"
#include "MMOQuestBase.h"
#include "MMOQuestComponent.generated.h"

class UMMOQuestBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestAccepted, UMMOQuestBase*, Quest, int32, QuestSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestCompleted, UMMOQuestBase*, Quest, int32, QuestSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestAbandoned, UMMOQuestBase*, Quest, int32, QuestSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, UMMOQuestBase*, Quest, int32, ObjectiveIndex, int32, CurrentCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestTrackedChanged, UMMOQuestBase*, Quest, bool, bTracked);

USTRUCT(BlueprintType)
struct MMOQUEST_API FMMOActiveQuestState
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag QuestID;

	UPROPERTY()
	TArray<FMMOQuestObjective> ObjectiveProgress;

	UPROPERTY()
	int32 QuestSlot = INDEX_NONE;

	UPROPERTY()
	double AcceptedTime = 0.0;

	UPROPERTY()
	double CompletedTime = 0.0;

	UPROPERTY()
	bool bTurnedIn = false;

	UPROPERTY()
	bool bIsTracked = false;

	bool IsFullyComplete() const
	{
		for (const FMMOQuestObjective& Obj : ObjectiveProgress)
		{
			if (!Obj.IsCompleted())
			{
				return false;
			}
		}
		return true;
	}
};

USTRUCT(BlueprintType)
struct MMOQUEST_API FMMOCompletedQuestEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag QuestID;

	UPROPERTY()
	int32 TimesCompleted = 1;

	UPROPERTY()
	double LastCompletedTime = 0.0;
};

UENUM(BlueprintType)
enum class EQuestGiverStatus : uint8
{
	None			UMETA(DisplayName = "None"),
	Available		UMETA(DisplayName = "Available"),
	InProgress		UMETA(DisplayName = "In Progress"),
	Completed		UMETA(DisplayName = "Completed"),
	NotAvailable	UMETA(DisplayName = "Not Available")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MMOQUEST_API UMMOQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOQuestComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestAccepted OnQuestAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestCompleted OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestAbandoned OnQuestAbandoned;

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnObjectiveUpdated OnObjectiveUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestTrackedChanged OnQuestTrackedChanged;

	UPROPERTY(ReplicatedUsing = OnRep_ActiveQuests, BlueprintReadOnly, Category = "Quest State")
	TArray<FMMOActiveQuestState> ActiveQuests;

	UFUNCTION()
	void OnRep_ActiveQuests();

	UPROPERTY(ReplicatedUsing = OnRep_CompletedQuests, BlueprintReadOnly, Category = "Quest State")
	TArray<FMMOCompletedQuestEntry> CompletedQuests;

	UFUNCTION()
	void OnRep_CompletedQuests();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	int32 MaxActiveQuests = 25;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	int32 MaxCompletedHistory = 200;

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Quest Actions")
	void ServerAcceptQuest(FGameplayTag QuestID);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Quest Actions")
	void ServerTurnInQuest(FGameplayTag QuestID);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Quest Actions")
	void ServerAbandonQuest(FGameplayTag QuestID);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Quest Actions")
	void ServerUpdateObjective(FGameplayTag QuestID, int32 ObjectiveIndex, int32 Amount);

	UFUNCTION(BlueprintCallable, Client, Reliable, Category = "Quest Notifications")
	void ClientQuestStateUpdated(FGameplayTag QuestID, EQuestType QuestType, bool bAccepted, bool bCompleted, bool bAbandoned);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Queries")
	EQuestGiverStatus QueryQuestGiverStatus(FGameplayTag NPCID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest Tracking")
	void SetQuestTracked(FGameplayTag QuestID, bool bTracked);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Queries")
	bool HasActiveQuest(FGameplayTag QuestID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Queries")
	bool HasCompletedQuest(FGameplayTag QuestID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Queries")
	int32 GetActiveQuestSlot(FGameplayTag QuestID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Queries")
	FMMOActiveQuestState GetActiveQuestState(FGameplayTag QuestID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Queries")
	TArray<FMMOActiveQuestState> GetActiveQuestsByType(EQuestType Type) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Queries")
	UMMOQuestBase* GetQuestData(FGameplayTag QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest Reset")
	void ResetDailyQuests();

	UFUNCTION(BlueprintCallable, Category = "Quest Reset")
	void ResetWeeklyQuests();

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Cooldown")
	bool IsQuestOnCooldown(const UMMOQuestBase* Quest) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Reward")
	void GrantQuestRewards(UMMOQuestBase* Quest, int32 ChoiceIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Objectives")
	void ProgressObjective(FGameplayTag QuestID, FGameplayTag TargetID, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Objectives")
	void ProgressObjectiveByType(FGameplayTag QuestID, EQuestObjectiveType Type, FGameplayTag TargetID, int32 Amount = 1);

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	void AddReferencedQuest(UMMOQuestBase* Quest);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Quest State")
	int32 DailyQuestResets = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Quest State")
	int32 WeeklyQuestResets = 0;

protected:
	UFUNCTION(BlueprintCallable, Category = "Quest Internal")
	bool CanAcceptQuest(FGameplayTag QuestID, FString& OutReason) const;

	UFUNCTION(BlueprintCallable, Category = "Quest Internal")
	void InternalAcceptQuest(const UMMOQuestBase* Quest);

	UFUNCTION(BlueprintCallable, Category = "Quest Internal")
	void InternalCompleteQuest(const UMMOQuestBase* Quest);

	UFUNCTION(BlueprintCallable, Category = "Quest Internal")
	void InternalAbandonQuest(const UMMOQuestBase* Quest);

	void GrantExperience(int32 Amount);
	void GrantCurrency(int32 Amount);
	void GrantItems(const TMap<FGameplayTag, int32>& Items);
	void GrantReputation(const TMap<FName, int32>& RepChanges);
	void GrantSpells(const TArray<FGameplayTag>& SpellTags);
	void GrantChoiceItems(const TArray<TMap<FGameplayTag, int32>>& Choices, int32 ChoiceIndex);

	TMap<FGameplayTag, UMMOQuestBase*> QuestDataCache;

	UPROPERTY(Replicated)
	FGameplayTag TrackedQuestID;
};

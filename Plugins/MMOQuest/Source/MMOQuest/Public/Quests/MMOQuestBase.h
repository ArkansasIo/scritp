#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MMOQuestObjective.h"
#include "MMOQuestBase.generated.h"

class ACharacter;

UENUM(BlueprintType)
enum class EQuestType : uint8
{
	Main		UMETA(DisplayName = "Main"),
	Side		UMETA(DisplayName = "Side"),
	Daily		UMETA(DisplayName = "Daily"),
	Weekly		UMETA(DisplayName = "Weekly"),
	Event		UMETA(DisplayName = "Event"),
	Class		UMETA(DisplayName = "Class"),
	Faction		UMETA(DisplayName = "Faction"),
	Profession	UMETA(DisplayName = "Profession")
};

USTRUCT(BlueprintType)
struct MMOQUEST_API FMMOQuestReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 ExperienceReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 CurrencyReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TMap<FGameplayTag, int32> FixedItems;

	TArray<TMap<FGameplayTag, int32>> ChoiceItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TMap<FName, int32> ReputationReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TArray<FGameplayTag> SpellUnlocks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 FactionReward = 0;
};

UCLASS(BlueprintType, Const)
class MMOQUEST_API UMMOQuestBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", AssetRegistrySearchable)
	FGameplayTag QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	TArray<FText> ObjectiveTexts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 QuestLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 MinLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 MaxLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	EQuestType QuestType = EQuestType::Side;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	FGameplayTag QuestGiverNPC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	FGameplayTag TurnInNPC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FGameplayTag> RequiredQuestIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	FGameplayTag NextQuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	bool bAutoAccept = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	bool bShareable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	bool bCancelable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	bool bRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RepeatCooldownSeconds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	bool bPartyProgress = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objectives")
	TArray<FMMOQuestObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FMMOQuestReward Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Restrictions")
	TArray<FGameplayTag> AllowedClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Restrictions")
	TArray<FGameplayTag> AllowedRaces;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Restrictions")
	TArray<FGameplayTag> AllowedFactions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Categorization")
	FGameplayTagContainer QuestTags;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest")
	bool IsRepeatableNow(const class UMMOQuestComponent* Component) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest")
	bool MeetsPrerequisites(const class UMMOQuestComponent* Component) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest")
	virtual FText GetQuestObjectiveDisplay(int32 ObjectiveIndex) const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

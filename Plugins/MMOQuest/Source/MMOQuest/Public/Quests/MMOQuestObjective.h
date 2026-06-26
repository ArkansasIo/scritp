#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MMOQuestObjective.generated.h"

UENUM(BlueprintType)
enum class EQuestObjectiveType : uint8
{
	Kill			UMETA(DisplayName = "Kill"),
	Collect			UMETA(DisplayName = "Collect"),
	Interact		UMETA(DisplayName = "Interact"),
	TalkTo			UMETA(DisplayName = "Talk To"),
	Escort			UMETA(DisplayName = "Escort"),
	ReachLocation	UMETA(DisplayName = "Reach Location"),
	Timer			UMETA(DisplayName = "Timer"),
	UseItem			UMETA(DisplayName = "Use Item")
};

USTRUCT(BlueprintType)
struct MMOQUEST_API FMMOQuestObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	EQuestObjectiveType ObjectiveType = EQuestObjectiveType::Kill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FGameplayTag TargetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 RequiredCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 CurrentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	float Radius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	float TimerDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bIsHidden = false;

	bool IsCompleted() const
	{
		return CurrentCount >= RequiredCount;
	}

	void ResetProgress()
	{
		CurrentCount = 0;
	}

	void AddProgress(int32 Amount = 1)
	{
		CurrentCount = FMath::Min(CurrentCount + Amount, RequiredCount);
	}
};

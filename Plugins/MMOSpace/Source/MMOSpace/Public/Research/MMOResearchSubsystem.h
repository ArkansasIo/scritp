#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Space/MMOSpaceTypes.h"
#include "MMOResearchSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResearchCompleted, EResearchCategory, Category);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTechUnlocked, int32, TechId);

USTRUCT(BlueprintType)
struct FTechDefinition
{
	GENERATED_BODY()

	bool operator==(const FTechDefinition& Other) const
	{
		return TechId == Other.TechId && TechName == Other.TechName
			&& Description == Other.Description && Category == Other.Category
			&& Tier == Other.Tier && Prerequisites == Other.Prerequisites
			&& ResearchCost == Other.ResearchCost && ResearchTime == Other.ResearchTime
			&& Bonuses.OrderIndependentCompareEqual(Other.Bonuses);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	int32 TechId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	FString TechName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	EResearchCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	int32 Tier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	TArray<int32> Prerequisites;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	FResourceSet ResearchCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	double ResearchTime = 60.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	TMap<FString, double> Bonuses;
};

USTRUCT(BlueprintType)
struct FResearchQueueItem
{
	GENERATED_BODY()

	bool operator==(const FResearchQueueItem& Other) const
	{
		return TechId == Other.TechId && Category == Other.Category
			&& StartTime == Other.StartTime && EndTime == Other.EndTime
			&& bPaused == Other.bPaused;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	int32 TechId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	EResearchCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	double StartTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	double EndTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	bool bPaused = false;
};

USTRUCT(BlueprintType)
struct FResearchLabData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	int32 Level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	int32 QueueSlots = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	double ResearchSpeedMultiplier = 1.0;
};

UCLASS()
class MMOSPACE_API UMMOResearchSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Research")
	FOnResearchCompleted OnResearchCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Research")
	FOnTechUnlocked OnTechUnlocked;

	UFUNCTION(BlueprintCallable, Category = "Research")
	void InitializeResearchTree();

	UFUNCTION(BlueprintCallable, Category = "Research")
	bool StartResearch(int32 TechId, const FResearchLabData& LabData);

	UFUNCTION(BlueprintCallable, Category = "Research")
	void CancelResearch(int32 TechId);

	UFUNCTION(BlueprintCallable, Category = "Research")
	void CompleteResearch(int32 TechId);

	UFUNCTION(BlueprintCallable, Category = "Research")
	TArray<FTechDefinition> GetAvailableTechs() const;

	UFUNCTION(BlueprintCallable, Category = "Research")
	TArray<FTechDefinition> GetCompletedTechs() const;

	UFUNCTION(BlueprintCallable, Category = "Research")
	TArray<FTechDefinition> GetTechsByCategory(EResearchCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "Research")
	bool IsTechAvailable(int32 TechId) const;

	UFUNCTION(BlueprintCallable, Category = "Research")
	bool IsTechCompleted(int32 TechId) const;

	UFUNCTION(BlueprintCallable, Category = "Research")
	FTechDefinition GetTechDefinition(int32 TechId) const;

	UFUNCTION(BlueprintCallable, Category = "Research")
	TArray<FResearchQueueItem> GetResearchQueue() const { return ResearchQueue; }

	UFUNCTION(BlueprintCallable, Category = "Research")
	double GetResearchBonus(const FString& BonusKey) const;

	UFUNCTION(BlueprintCallable, Category = "Research")
	int32 GetResearchLevel(EResearchCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "Research")
	void UpdateResearchProgress(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Research")
	void SetResearchLabLevel(const FResearchLabData& LabData) { CurrentLab = LabData; }

	UFUNCTION(BlueprintCallable, Category = "Research")
	FResearchLabData GetResearchLabData() const { return CurrentLab; }

	UFUNCTION(BlueprintCallable, Category = "Research")
	void SpeedUpResearch(int32 TechId, double GamerAccelerateCount);

protected:
	static TArray<FTechDefinition> GenerateDefaultTechTree();
	bool CheckPrerequisites(const FTechDefinition& Tech) const;

private:
	TArray<FTechDefinition> TechTree;
	TArray<int32> CompletedTechs;
	TArray<FResearchQueueItem> ResearchQueue;
	TMap<EResearchCategory, int32> ResearchLevels;
	TMap<FString, double> AccumulatedBonuses;
	FResearchLabData CurrentLab;
};

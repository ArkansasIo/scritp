#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/CurveTable.h"
#include "GameplayTagContainer.h"
#include "MMOLevelData.generated.h"

USTRUCT(BlueprintType)
struct FMMOLevelStatGain
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	int32 Strength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	int32 Dexterity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	int32 Intelligence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	int32 Vitality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	int32 Wisdom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	int32 Luck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	float MaxHPGain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	float MaxMPGain;

	FMMOLevelStatGain()
		: Strength(0)
		, Dexterity(0)
		, Intelligence(0)
		, Vitality(0)
		, Wisdom(0)
		, Luck(0)
		, MaxHPGain(0.0f)
		, MaxMPGain(0.0f)
	{}
};

USTRUCT(BlueprintType)
struct FMMOAbilityUnlock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	int32 UnlockLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	FName AbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
	bool bAutoUnlock;
};

UCLASS(BlueprintType)
class MMCHARACTER_API UMMOLevelDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	int32 MaxLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	TObjectPtr<UCurveTable> ExperienceCurveTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	FName ExperienceCurveRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	TObjectPtr<UCurveFloat> ExperienceCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	int32 BaseExperienceRequirement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	float ExperienceExponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	TArray<FMMOLevelStatGain> StatGainsPerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	TArray<FMMOAbilityUnlock> AbilityUnlocks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	TMap<int32, int32> ExperienceRequirements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	float HPMultiplierPerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	float MPMultiplierPerLevel;

	UFUNCTION(BlueprintCallable, Category = "MMO|Levels")
	virtual int64 GetExperienceRequiredForLevel(int32 TargetLevel) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Levels")
	virtual int32 GetLevelFromExperience(int64 TotalExperience) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Levels")
	virtual FMMOLevelStatGain GetStatGainForLevel(int32 Level) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Levels")
	virtual TArray<FMMOAbilityUnlock> GetAbilitiesForLevel(int32 Level) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Levels")
	virtual bool HasMaxLevelBeenReached(int32 CurrentLevel) const;

	UMMOLevelDataAsset();
};

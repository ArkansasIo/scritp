#pragma once

#include "MMOCore.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MMOGlobalData.generated.h"

USTRUCT(BlueprintType)
struct FMMOExperienceEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience")
	int32 ExperienceRequired = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience")
	int32 CumulativeExperience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience")
	float StatMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FMMOStatDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	FName StatName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	FGameplayTag StatTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float BaseValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float PerLevelValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float MaxValue = 999999.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	bool bClampToBounds = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	FText DisplayName;
};

USTRUCT(BlueprintType)
struct FMMODamageTypeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DamageType")
	FName TypeName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DamageType")
	FGameplayTag DamageTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DamageType")
	FLinearColor DamageColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DamageType")
	float GlobalResistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DamageType")
	TSoftObjectPtr<UTexture2D> Icon;
};

USTRUCT(BlueprintType)
struct FMMODropRateEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DropRate")
	FPrimaryAssetId ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DropRate")
	float DropChance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DropRate")
	int32 MinDropCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DropRate")
	int32 MaxDropCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DropRate")
	TArray<FGameplayTag> RequiredTags;
};

USTRUCT(BlueprintType)
struct FMMOGlobalConstant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constant")
	FName ConstantName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constant")
	float FloatValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constant")
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constant")
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constant")
	FText Description;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalDataLoaded);

UCLASS(BlueprintType, Blueprintable)
class MMOCORE_API UMMOGlobalData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UMMOGlobalData();

	UFUNCTION(BlueprintPure, Category = "MMO|GlobalData")
	static UMMOGlobalData* Get();

	UFUNCTION(BlueprintPure, Category = "MMO|Experience")
	int32 GetExperienceForLevel(int32 Level) const;

	UFUNCTION(BlueprintPure, Category = "MMO|Experience")
	int32 GetCumulativeExperienceForLevel(int32 Level) const;

	UFUNCTION(BlueprintPure, Category = "MMO|Experience")
	float GetLevelStatMultiplier(int32 Level) const;

	const FMMOStatDefinition* GetStatDefinition(FName StatName) const;

	const FMMOStatDefinition* GetStatDefinitionByTag(FGameplayTag StatTag) const;

	const FMMODamageTypeDefinition* GetDamageTypeDefinition(FName TypeName) const;

	UFUNCTION(BlueprintPure, Category = "MMO|Constants")
	float GetGlobalFloatConstant(FName ConstantName, float DefaultValue = 0.0f) const;

	UFUNCTION(BlueprintPure, Category = "MMO|Constants")
	int32 GetGlobalIntConstant(FName ConstantName, int32 DefaultValue = 0) const;

	UFUNCTION(BlueprintPure, Category = "MMO|Constants")
	void AddGlobalConstant(const FMMOGlobalConstant& Constant);

	UFUNCTION(BlueprintPure, Category = "MMO|DropRates")
	TArray<FMMODropRateEntry> GetDropRatesForContainer(FName ContainerID) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience")
	TArray<FMMOExperienceEntry> ExperienceCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	TArray<FMMOStatDefinition> StatDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DamageTypes")
	TArray<FMMODamageTypeDefinition> DamageTypeDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constants")
	TArray<FMMOGlobalConstant> GlobalConstants;

	TMap<FName, TArray<FMMODropRateEntry>> DropRateTables;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTags")
	FGameplayTagContainer GameplayTags;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnGlobalDataLoaded OnGlobalDataLoaded;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("MMOGlobalData"), TEXT("GlobalData"));
	}

protected:
	bool LoadExperienceCurve();
	bool ValidateData();
};

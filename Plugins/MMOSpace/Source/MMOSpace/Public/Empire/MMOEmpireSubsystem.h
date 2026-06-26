#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Space/MMOSpaceTypes.h"
#include "MMOEmpireSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmpireLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGovernmentChanged, EGovernmentType, NewGovernment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceTick, const FResourceSet&, NewResources);

USTRUCT(BlueprintType)
struct FEmpireProgression
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 EmpireLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int64 EmpireExperience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 Tier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int64 TierExperience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 PrestigeLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 KardashevLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	EGovernmentType Government = EGovernmentType::Democracy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 TotalTurns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 CurrentTurns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 TurnRegenRate = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	double LastTurnUpdate = 0;
};

USTRUCT(BlueprintType)
struct FGovernmentBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Government")
	EGovernmentType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Government")
	FString BonusDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Government")
	double ResourceMultiplier = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Government")
	double FleetMultiplier = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Government")
	double ResearchMultiplier = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Government")
	double CombatMultiplier = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Government")
	double DiplomaticMultiplier = 1.0;
};

UCLASS()
class MMOSPACE_API UMMOEmpireSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Empire")
	FOnEmpireLevelUp OnEmpireLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Empire")
	FOnGovernmentChanged OnGovernmentChanged;

	UPROPERTY(BlueprintAssignable, Category = "Empire")
	FOnResourceTick OnResourceTick;

	UFUNCTION(BlueprintCallable, Category = "Empire")
	void InitializeEmpire(const FString& PlayerId, const FString& PlanetName);

	UFUNCTION(BlueprintCallable, Category = "Empire")
	void AddExperience(int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "Empire")
	void SetGovernment(EGovernmentType NewGovernment);

	UFUNCTION(BlueprintCallable, Category = "Empire")
	EGovernmentType GetCurrentGovernment() const { return Progression.Government; }

	UFUNCTION(BlueprintCallable, Category = "Empire")
	FEmpireProgression GetProgression() const { return Progression; }

	UFUNCTION(BlueprintCallable, Category = "Empire")
	FGovernmentBonus GetGovernmentBonus(EGovernmentType GovernmentType) const;

	UFUNCTION(BlueprintCallable, Category = "Empire")
	void AddTurns(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Empire")
	bool SpendTurns(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Empire")
	void UpdateResourceProduction(const TArray<FBuildingLevel>& Buildings);

	UFUNCTION(BlueprintCallable, Category = "Empire")
	FResourceSet CalculateProduction(const TArray<FBuildingLevel>& Buildings) const;

	UFUNCTION(BlueprintCallable, Category = "Empire")
	void SetResources(const FResourceSet& NewResources);

	UFUNCTION(BlueprintCallable, Category = "Empire")
	FResourceSet GetResources() const { return Resources; }

	UFUNCTION(BlueprintCallable, Category = "Empire")
	bool SpendResources(const FResourceSet& Cost);

	UFUNCTION(BlueprintCallable, Category = "Empire")
	FString GetEmpireSummary() const;

	UFUNCTION(BlueprintCallable, Category = "Empire")
	void TickResourceGeneration(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Empire")
	static int32 GetEmpireLevelUpThreshold();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Empire")
	FString PlayerEmpireId;

protected:
	static TMap<EGovernmentType, FGovernmentBonus> InitializeGovernmentBonuses();

private:
	FEmpireProgression Progression;
	FResourceSet Resources;
	FResourceSet ProductionPerHour;
	TMap<EGovernmentType, FGovernmentBonus> GovernmentBonuses;
	float ResourceTickAccumulator = 0.0f;
};

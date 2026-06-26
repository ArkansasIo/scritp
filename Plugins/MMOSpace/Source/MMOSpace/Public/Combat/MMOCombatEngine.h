#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Space/MMOSpaceTypes.h"
#include "Fleet/MMOFleetSubsystem.h"
#include "MMOCombatEngine.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBattleStarted, FString, BattleId, const TArray<FFleetData>&, Participants);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBattleRound, int32, Round, TArray<FString>, DamageLog, TArray<FFleetUnit>, Survivors);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleEnded, FString, BattleId);

USTRUCT(BlueprintType)
struct FCombatRoundResult
{
	GENERATED_BODY()

	bool operator==(const FCombatRoundResult& Other) const
	{
		return Round == Other.Round && AttackerRemaining == Other.AttackerRemaining
			&& DefenderRemaining == Other.DefenderRemaining
			&& AttackerLosses == Other.AttackerLosses
			&& DefenderLosses == Other.DefenderLosses
			&& LootCaptured == Other.LootCaptured
			&& ExplosiveRatio == Other.ExplosiveRatio;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 Round = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FFleetUnit> AttackerRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FFleetUnit> DefenderRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FFleetUnit> AttackerLosses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FFleetUnit> DefenderLosses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FResourceSet> LootCaptured;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	double ExplosiveRatio = 0.0;
};

USTRUCT(BlueprintType)
struct FCombatResult
{
	GENERATED_BODY()

	bool operator==(const FCombatResult& Other) const
	{
		return BattleId == Other.BattleId && AttackerId == Other.AttackerId
			&& DefenderId == Other.DefenderId && bAttackerWon == Other.bAttackerWon
			&& TotalRounds == Other.TotalRounds && RoundResults == Other.RoundResults
			&& TotalLoot == Other.TotalLoot && DebrisMetal == Other.DebrisMetal
			&& DebrisCrystal == Other.DebrisCrystal && WinningFormation == Other.WinningFormation;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FString BattleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FString AttackerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FString DefenderId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bAttackerWon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 TotalRounds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FCombatRoundResult> RoundResults;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FResourceSet> TotalLoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 DebrisMetal = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 DebrisCrystal = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	EFleetFormation WinningFormation = EFleetFormation::LineAbreast;
};

UCLASS()
class MMOSPACE_API UMMOCombatEngine : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnBattleStarted OnBattleStarted;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnBattleRound OnBattleRound;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnBattleEnded OnBattleEnded;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	FString StartBattle(const TArray<FFleetUnit>& AttackerFleet, const TArray<FFleetUnit>& DefenderFleet,
		const FString& AttackerId, const FString& DefenderId,
		EFleetFormation AttackerFormation = EFleetFormation::LineAbreast,
		EFleetFormation DefenderFormation = EFleetFormation::LineAbreast);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SimulateCombat(FCombatResult& Result);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	FCombatResult GetLastResult() const { return LastResult; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	FCombatRoundResult SimulateRound(TArray<FFleetUnit>& Attacker, TArray<FFleetUnit>& Defender,
		EFleetFormation AttackForm, EFleetFormation DefendForm);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	double CalculateFormationBonus(EFleetFormation Formation, bool bIsAttacker) const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 CalculateDamage(const FShipStats& Attacker, const FShipStats& Defender, double FormationBonus) const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	double CalculateHitChance(EShipClass AttackerClass, EShipClass DefenderClass, double TechLevel) const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	TArray<FFleetUnit> CalculateLosses(const TArray<FFleetUnit>& Fleet, double ExplosiveRatio) const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsBattling() const { return bSimulating; }

protected:
	static double GetWeaponRating(EShipClass ShipClass);
	static double GetArmorRating(EShipClass ShipClass);
	static double GetShieldRating(EShipClass ShipClass);
	static double GetFormationDamageBonus(EFleetFormation Formation);
	static double GetFormationDefenseBonus(EFleetFormation Formation);
	static double GetFormationSpeedBonus(EFleetFormation Formation);

private:
	FCombatResult LastResult;
	bool bSimulating = false;
	int32 BattleCounter = 0;

	FString GenerateBattleId();
	double CalculateExplosiveRatio(const TArray<FFleetUnit>& Fleet) const;
};

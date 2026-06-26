#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/CurveTable.h"
#include "MMOStatComponent.generated.h"

class UMMOCharacterDataAsset;

UENUM(BlueprintType)
enum class EMMOPrimaryStat : uint8
{
	Strength,
	Dexterity,
	Intelligence,
	Vitality,
	Wisdom,
	Luck
};

UENUM(BlueprintType)
enum class EMMOSecondaryStat : uint8
{
	AttackPower,
	SpellPower,
	Defense,
	MagicDefense,
	CriticalHitChance,
	CriticalHitDamage,
	DodgeChance,
	ParryChance,
	BlockChance,
	HitChance,
	Haste,
	MovementSpeedModifier
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPrimaryStatChanged, EMMOPrimaryStat, StatType, int32, OldValue, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSecondaryStatChanged, EMMOSecondaryStat, StatType, float, OldValue, float, NewValue);

USTRUCT(BlueprintType)
struct FMMOPrimaryStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 Strength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 Dexterity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 Intelligence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 Vitality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 Wisdom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 Luck;

	FMMOPrimaryStats()
		: Strength(1)
		, Dexterity(1)
		, Intelligence(1)
		, Vitality(1)
		, Wisdom(1)
		, Luck(1)
	{}
};

USTRUCT(BlueprintType)
struct FMMOSecondaryStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float SpellPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Defense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MagicDefense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CriticalHitChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CriticalHitDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DodgeChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ParryChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BlockChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float HitChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Haste;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MovementSpeedModifier;

	FMMOSecondaryStats()
		: AttackPower(0.0f)
		, SpellPower(0.0f)
		, Defense(0.0f)
		, MagicDefense(0.0f)
		, CriticalHitChance(0.0f)
		, CriticalHitDamage(0.0f)
		, DodgeChance(0.0f)
		, ParryChance(0.0f)
		, BlockChance(0.0f)
		, HitChance(0.0f)
		, Haste(0.0f)
		, MovementSpeedModifier(1.0f)
	{}
};

UCLASS(ClassGroup = (MMO), meta = (BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class MMOCHARACTER_API UMMOStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOStatComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Stats|Data")
	TObjectPtr<UCurveTable> StatCurveTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Stats|Data")
	TObjectPtr<UMMOCharacterDataAsset> CharacterDataAsset;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "MMO|Stats|Primary")
	FMMOPrimaryStats BasePrimaryStats;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "MMO|Stats|Primary")
	FMMOPrimaryStats BonusPrimaryStats;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Stats|Secondary")
	FMMOSecondaryStats SecondaryStats;

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats|Primary")
	virtual int32 GetEffectivePrimaryStat(EMMOPrimaryStat StatType) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats|Primary")
	virtual void ModifyBasePrimaryStat(EMMOPrimaryStat StatType, int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats|Primary")
	virtual void SetBasePrimaryStat(EMMOPrimaryStat StatType, int32 NewValue);

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats|Primary")
	virtual void AddBonusPrimaryStat(EMMOPrimaryStat StatType, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats|Primary")
	virtual void RemoveBonusPrimaryStat(EMMOPrimaryStat StatType, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats|Primary")
	virtual void ClearBonusStats();

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats|Secondary")
	virtual float GetSecondaryStat(EMMOSecondaryStat StatType) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats|Calculation")
	virtual void CalculateSecondaryStats();

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats|Calculation")
	virtual float LookupStatCurve(FName CurveRowName, float InputValue) const;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnPrimaryStatChanged OnPrimaryStatChanged;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnSecondaryStatChanged OnSecondaryStatChanged;

protected:
	UFUNCTION()
	void OnRep_PrimaryStats();

	virtual void UpdateDerivedStats();

	void RecalculateAll();
};

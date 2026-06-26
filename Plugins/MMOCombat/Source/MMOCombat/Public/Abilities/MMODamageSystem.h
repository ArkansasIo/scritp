#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "MMODamageSystem.generated.h"

class AMMOCharacterBase;

UENUM(BlueprintType)
enum class EMMODamageType : uint8
{
	Physical UMETA(DisplayName = "Physical"),
	Magic UMETA(DisplayName = "Magic"),
	Fire UMETA(DisplayName = "Fire"),
	Ice UMETA(DisplayName = "Ice"),
	Frost UMETA(DisplayName = "Frost"),
	Holy UMETA(DisplayName = "Holy"),
	Shadow UMETA(DisplayName = "Shadow"),
	Nature UMETA(DisplayName = "Nature"),
	Arcane UMETA(DisplayName = "Arcane"),
	TrueDamage UMETA(DisplayName = "True Damage"),
};

UENUM(BlueprintType)
enum class EMMOHitResult : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Critical UMETA(DisplayName = "Critical"),
	Glancing UMETA(DisplayName = "Glancing"),
	Crushing UMETA(DisplayName = "Crushing"),
	Blocked UMETA(DisplayName = "Blocked"),
	Block UMETA(DisplayName = "Block"),
	Parried UMETA(DisplayName = "Parried"),
	Parry UMETA(DisplayName = "Parry"),
	Dodged UMETA(DisplayName = "Dodged"),
	Dodge UMETA(DisplayName = "Dodge"),
	Missed UMETA(DisplayName = "Missed"),
	Miss UMETA(DisplayName = "Miss"),
	Absorbed UMETA(DisplayName = "Absorbed"),
	Immune UMETA(DisplayName = "Immune"),
};

UENUM(BlueprintType)
enum class EMMODamageMitigation : uint8
{
	Armor UMETA(DisplayName = "Armor"),
	MagicResistance UMETA(DisplayName = "Magic Resistance"),
	Absorption UMETA(DisplayName = "Absorption Shield"),
	Immunity UMETA(DisplayName = "Immunity"),
};

USTRUCT(BlueprintType)
struct FMMODamageResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float RawDamage = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float MitigatedDamage = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float FinalDamage = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float RawHealing = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float FinalHealing = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	EMMODamageType DamageType = EMMODamageType::Physical;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	EMMOHitResult HitResult = EMMOHitResult::Normal;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TArray<EMMODamageMitigation> AppliedMitigations;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float ThreatGenerated = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float OverkillDamage = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float Overheal = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	FGameplayTagContainer DamageTags;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	bool bIsPeriodic = false;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	AActor* Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	AActor* Causer = nullptr;
};

USTRUCT(BlueprintType)
struct FMMOAttackAttributes
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	float AttackPower = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	float SpellPower = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	float CritChance = 5.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	float CritMultiplier = 2.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	float HitChance = 95.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	float Expertise = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	float ArmorPenetration = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	float MagicPenetration = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	float HastePercent = 0.0f;
};

USTRUCT(BlueprintType)
struct FMMODefenseAttributes
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Defense")
	float Armor = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Defense")
	float MagicResistance = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Defense")
	float DodgeChance = 5.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Defense")
	float ParryChance = 5.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Defense")
	float BlockChance = 5.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Defense")
	float BlockValue = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Defense")
	float CritAvoidance = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Defense")
	float Resilience = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Defense")
	float DefenseRating = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Defense")
	float AbsorptionShield = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Defense")
	FGameplayTagContainer ImmunityTags;
};

USTRUCT(BlueprintType)
struct FMMODoTData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "DoT")
	float TickInterval = 2.0f;

	UPROPERTY(BlueprintReadWrite, Category = "DoT")
	float TotalDuration = 10.0f;

	UPROPERTY(BlueprintReadWrite, Category = "DoT")
	float BaseDamagePerTick = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "DoT")
	float BaseHealingPerTick = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "DoT")
	EMMODamageType DamageType = EMMODamageType::Physical;

	UPROPERTY(BlueprintReadWrite, Category = "DoT")
	FGameplayTagContainer DoTTags;

	UPROPERTY(BlueprintReadWrite, Category = "DoT")
	int32 MaxTicks = 5;

	UPROPERTY(BlueprintReadWrite, Category = "DoT")
	bool bScalesWithAttackPower = true;

	UPROPERTY(BlueprintReadWrite, Category = "DoT")
	float ScalingFactor = 0.1f;
};

UCLASS()
class MMOCOMBAT_API UMMODamageSystem : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// --- Main Damage Calculation ---
	UFUNCTION(BlueprintCallable, Category = "MMO Combat | Damage")
	static FMMODamageResult CalculateDamage(
		AMMOCharacterBase* Attacker,
		AMMOCharacterBase* Defender,
		float BaseDamage,
		EMMODamageType DamageType,
		const FGameplayTagContainer& DamageTags,
		bool bCanCrit = true,
		bool bCanBeMitigated = true
	);

	UFUNCTION(BlueprintCallable, Category = "MMO Combat | Damage")
	static FMMODamageResult CalculateHealing(
		AMMOCharacterBase* Healer,
		AMMOCharacterBase* Target,
		float BaseHealing,
		const FGameplayTagContainer& HealingTags,
		bool bCanCrit = true
	);

	// --- Hit Determination ---
	UFUNCTION(BlueprintPure, Category = "MMO Combat | Damage")
	static EMMOHitResult DetermineHitResult(
		const FMMOAttackAttributes& AttackerStats,
		const FMMODefenseAttributes& DefenderStats
	);

	// --- Mitigation ---
	UFUNCTION(BlueprintPure, Category = "MMO Combat | Damage")
	static float CalculateArmorMitigation(float Armor, float AttackerLevel, float DefenderLevel);

	UFUNCTION(BlueprintPure, Category = "MMO Combat | Damage")
	static float CalculateResistanceMitigation(float Resistance, float AttackerLevel);

	UFUNCTION(BlueprintPure, Category = "MMO Combat | Damage")
	static float CalculateCritMultiplier(float BaseCritMultiplier, float Resilience);

	// --- Threat ---
	UFUNCTION(BlueprintPure, Category = "MMO Combat | Damage")
	static float CalculateThreatFromDamage(float Damage, EMMOHitResult HitResult, float ThreatMultiplier = 1.0f);

	UFUNCTION(BlueprintPure, Category = "MMO Combat | Damage")
	static float CalculateThreatFromHealing(float Healing, float ThreatMultiplier = 0.5f);

	// --- DoT / Periodic ---
	UFUNCTION(BlueprintCallable, Category = "MMO Combat | Damage")
	static void ApplyPeriodicDamage(
		AMMOCharacterBase* Target,
		const FMMODoTData& DoTData,
		AActor* Instigator
	);

	// --- Utility ---
	UFUNCTION(BlueprintPure, Category = "MMO Combat | Damage")
	static bool RollCrit(float CritChance, float CritAvoidance);

	UFUNCTION(BlueprintPure, Category = "MMO Combat | Damage")
	static float ClampDamage(float Damage, float MinDamage = 1.0f);

	// --- Stat Retrieval Helpers ---
	UFUNCTION(BlueprintPure, Category = "MMO Combat | Damage")
	static FMMOAttackAttributes GetAttackAttributes(AMMOCharacterBase* Character);

	UFUNCTION(BlueprintPure, Category = "MMO Combat | Damage")
	static FMMODefenseAttributes GetDefenseAttributes(AMMOCharacterBase* Character);
};

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "NiagaraSystem.h"
#include "Animation/AnimMontage.h"
#include "Engine/Texture2D.h"
#include "MMOAbilityBase.generated.h"

class AMMOCharacterBase;
class UMMOAbilityComponent;

UENUM(BlueprintType)
enum class EMMOTargetType : uint8
{
	Self UMETA(DisplayName = "Self"),
	SingleEnemy UMETA(DisplayName = "Single Enemy"),
	SingleAlly UMETA(DisplayName = "Single Ally"),
	AreaOfEffect UMETA(DisplayName = "Area of Effect"),
	Cone UMETA(DisplayName = "Cone"),
	Line UMETA(DisplayName = "Line"),
	LocationAtRange UMETA(DisplayName = "Location at Range"),
};

UENUM(BlueprintType)
enum class EMMOAbilityType : uint8
{
	Active UMETA(DisplayName = "Active"),
	Passive UMETA(DisplayName = "Passive"),
	Toggle UMETA(DisplayName = "Toggle"),
	Triggered UMETA(DisplayName = "Triggered"),
};

UENUM(BlueprintType)
enum class EMMOWeaponType : uint8
{
	None UMETA(DisplayName = "None"),
	OneHandSword UMETA(DisplayName = "One-Hand Sword"),
	TwoHandSword UMETA(DisplayName = "Two-Hand Sword"),
	Dagger UMETA(DisplayName = "Dagger"),
	Staff UMETA(DisplayName = "Staff"),
	Bow UMETA(DisplayName = "Bow"),
	Shield UMETA(DisplayName = "Shield"),
	Fist UMETA(DisplayName = "Fist"),
	Spear UMETA(DisplayName = "Spear"),
	Axe UMETA(DisplayName = "Axe"),
	Mace UMETA(DisplayName = "Mace"),
	Wand UMETA(DisplayName = "Wand"),
};

UENUM(BlueprintType)
enum class EMMOStance : uint8
{
	None UMETA(DisplayName = "None"),
	Battle UMETA(DisplayName = "Battle"),
	Defensive UMETA(DisplayName = "Defensive"),
	Stealth UMETA(DisplayName = "Stealth"),
	Flight UMETA(DisplayName = "Flight"),
	Swim UMETA(DisplayName = "Swim"),
	Sit UMETA(DisplayName = "Sit"),
};

USTRUCT(BlueprintType)
struct FMMOAbilityResourceCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	float Health = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	float Mana = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	float Stamina = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	float Rage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	float Energy = 0.0f;
};

USTRUCT(BlueprintType)
struct FMMOAbilityLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
	float Cooldown = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
	float CastTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
	float ChannelTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
	float Range = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
	float HealingMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
	FMMOAbilityResourceCost ResourceCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
	int32 ComboIndex = -1;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class MMOCOMBAT_API UMMOAbilityBase : public UObject
{
	GENERATED_BODY()

public:
	UMMOAbilityBase();

// --- Identity ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName AbilityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText AbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	TSoftObjectPtr<UTexture2D> Icon;

// --- Type ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EMMOAbilityType AbilityType = EMMOAbilityType::Active;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EMMOTargetType TargetType = EMMOTargetType::SingleEnemy;

// --- Timing ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float BaseCooldown = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float BaseCastTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float BaseChannelTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float Range = 30.0f;

// --- Resource Cost ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	FMMOAbilityResourceCost ResourceCost;

// --- Level & Rank ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 MaxRank = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 CurrentRank = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	TArray<FMMOAbilityLevelData> RankData;

// --- Tags ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer AbilityTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer ModifierTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer CancelTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer BlockedByTags;

// --- Animation & VFX ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UAnimMontage> CastMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UNiagaraSystem> CastVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UNiagaraSystem> ImpactVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<USoundBase> CastSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<USoundBase> ImpactSound;

// --- Requirements ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	EMMOStance RequiredStance = EMMOStance::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	EMMOWeaponType RequiredWeaponType = EMMOWeaponType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	FGameplayTagContainer RequiredTags;

// --- Combo / Chain ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	TArray<TSoftObjectPtr<UMMOAbilityBase>> ChainAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	float ComboWindow = 1.0f;

// --- Damage / Effect ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	float BaseDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	float BaseHealing = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	float DamagePerLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	float HealingPerLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<TSubclassOf<class UMMOBuff>> AppliedBuffs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	float BuffChance = 100.0f;

// --- State ---
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TWeakObjectPtr<UMMOAbilityComponent> OwningComponent;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float CurrentCooldownRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float CurrentCastProgress = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsOnCooldown = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsCasting = false;

// --- Core Functions ---
	UFUNCTION(BlueprintPure, Category = "Ability")
	virtual bool CanCast(const AMMOCharacterBase* Caster, const AMMOCharacterBase* Target) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void OnCastStart(AMMOCharacterBase* Caster, const FVector& TargetLocation, AMMOCharacterBase* Target);
	virtual void OnCastStart_Implementation(AMMOCharacterBase* Caster, const FVector& TargetLocation, AMMOCharacterBase* Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void OnCastTick(AMMOCharacterBase* Caster, float DeltaTime);
	virtual void OnCastTick_Implementation(AMMOCharacterBase* Caster, float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void OnCastCompleted(AMMOCharacterBase* Caster, const FVector& TargetLocation, AMMOCharacterBase* Target);
	virtual void OnCastCompleted_Implementation(AMMOCharacterBase* Caster, const FVector& TargetLocation, AMMOCharacterBase* Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void OnCastCancelled(AMMOCharacterBase* Caster);
	virtual void OnCastCancelled_Implementation(AMMOCharacterBase* Caster);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	float GetCooldownForRank(int32 Rank) const;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	float GetCastTimeForRank(int32 Rank) const;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	float GetRangeForRank(int32 Rank) const;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	FMMOAbilityResourceCost GetResourceCostForRank(int32 Rank) const;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	float GetDamageForRank(int32 Rank, float CasterLevel) const;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	float GetHealingForRank(int32 Rank, float CasterLevel) const;
};

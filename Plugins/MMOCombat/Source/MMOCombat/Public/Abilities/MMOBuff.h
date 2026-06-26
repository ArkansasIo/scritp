#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/Texture2D.h"
#include "Abilities/MMODamageSystem.h"
#include "MMOBuff.generated.h"

class AMMOCharacterBase;
class UMMOBuffSystemComponent;

UENUM(BlueprintType)
enum class EMMOBuffStackType : uint8
{
	NoStack UMETA(DisplayName = "No Stack"),
	Single UMETA(DisplayName = "Single"),
	Multi UMETA(DisplayName = "Multi"),
	Limited UMETA(DisplayName = "Limited"),
};

UENUM(BlueprintType)
enum class EMMOBuffType : uint8
{
	Beneficial UMETA(DisplayName = "Beneficial"),
	Harmful UMETA(DisplayName = "Harmful"),
	Hidden UMETA(DisplayName = "Hidden"),
};

UENUM(BlueprintType)
enum class EMMODispelType : uint8
{
	None UMETA(DisplayName = "None"),
	Magic UMETA(DisplayName = "Magic"),
	Curse UMETA(DisplayName = "Curse"),
	Poison UMETA(DisplayName = "Poison"),
	Disease UMETA(DisplayName = "Disease"),
	Bleed UMETA(DisplayName = "Bleed"),
};

UENUM(BlueprintType)
enum class EMMOStatModifier : uint8
{
	None UMETA(DisplayName = "None"),
	Strength UMETA(DisplayName = "Strength"),
	Agility UMETA(DisplayName = "Agility"),
	Intellect UMETA(DisplayName = "Intellect"),
	Stamina UMETA(DisplayName = "Stamina"),
	Spirit UMETA(DisplayName = "Spirit"),
	AttackPower UMETA(DisplayName = "Attack Power"),
	SpellPower UMETA(DisplayName = "Spell Power"),
	Armor UMETA(DisplayName = "Armor"),
	MagicResistance UMETA(DisplayName = "Magic Resistance"),
	CritChance UMETA(DisplayName = "Crit Chance"),
	Haste UMETA(DisplayName = "Haste"),
	MoveSpeed UMETA(DisplayName = "Move Speed"),
	DamageMultiplier UMETA(DisplayName = "Damage Multiplier"),
	HealingMultiplier UMETA(DisplayName = "Healing Multiplier"),
	MaxHealth UMETA(DisplayName = "Max Health"),
	MaxMana UMETA(DisplayName = "Max Mana"),
	HealthRegen UMETA(DisplayName = "Health Regen"),
	ManaRegen UMETA(DisplayName = "Mana Regen"),
	DodgeChance UMETA(DisplayName = "Dodge Chance"),
	ParryChance UMETA(DisplayName = "Parry Chance"),
	BlockChance UMETA(DisplayName = "Block Chance"),
};

USTRUCT(BlueprintType)
struct FMMOStatModifierValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	EMMOStatModifier Stat = EMMOStatModifier::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	bool bIsPercentage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	FGameplayTag ModifierTag;
};

USTRUCT(BlueprintType)
struct FMMOPeriodicEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periodic")
	float TickInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periodic")
	float BaseDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periodic")
	float BaseHealing = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periodic")
	EMMODamageType DamageType = EMMODamageType::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periodic")
	float ScalingFactor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periodic")
	bool bScalesWithAttackPower = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Periodic")
	FGameplayTagContainer EffectTags;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class MMOCOMBAT_API UMMOBuff : public UObject
{
	GENERATED_BODY()

public:
	UMMOBuff();

// --- Identity ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName BuffID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText BuffName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	TSoftObjectPtr<UTexture2D> Icon;

// --- Type ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EMMOBuffType BuffType = EMMOBuffType::Beneficial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EMMODispelType DispelType = EMMODispelType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EMMOBuffStackType StackType = EMMOBuffStackType::NoStack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	FGameplayTagContainer BuffTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	FGameplayTagContainer ImmunityCategories;

// --- Duration & Timing ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float Duration = 10.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float RemainingTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float TickRate = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	bool bInfiniteDuration = false;

// --- Stacking ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stacking")
	int32 MaxStacks = 1;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 CurrentStacks = 1;

// --- Stat Modifiers ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers")
	TArray<FMMOStatModifierValue> StatModifiers;

// --- Periodic Effects ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FMMOPeriodicEffect PeriodicEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<TSubclassOf<UMMOBuff>> ChildBuffsOnApply;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<TSubclassOf<UMMOBuff>> ChildBuffsOnExpire;

// --- Visuals ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<class UNiagaraSystem> BuffVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<USoundBase> ApplySound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<USoundBase> ExpireSound;

// --- State ---
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TWeakObjectPtr<AActor> SourceActor;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	TWeakObjectPtr<UMMOBuffSystemComponent> OwningSystem;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float TimeSinceLastTick = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float TimeSinceApplied = 0.0f;

// --- Event Handlers ---
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Buff")
	void OnApplied(AActor* Source, AActor* Target);
	virtual void OnApplied_Implementation(AActor* Source, AActor* Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Buff")
	void OnTick(AActor* Target, float DeltaTime);
	virtual void OnTick_Implementation(AActor* Target, float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Buff")
	void OnExpired(AActor* Target);
	virtual void OnExpired_Implementation(AActor* Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Buff")
	void OnRemoved(AActor* Target, bool bDispelled);
	virtual void OnRemoved_Implementation(AActor* Target, bool bDispelled);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Buff")
	void OnStackChanged(int32 OldStacks, int32 NewStacks);
	virtual void OnStackChanged_Implementation(int32 OldStacks, int32 NewStacks);

// --- Utility ---
	UFUNCTION(BlueprintPure, Category = "Buff")
	bool IsExpired() const;

	UFUNCTION(BlueprintCallable, Category = "Buff")
	void RefreshDuration();

	UFUNCTION(BlueprintCallable, Category = "Buff")
	void AddStack(int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Buff")
	void RemoveStack(int32 Amount = 1);

	UFUNCTION(BlueprintPure, Category = "Buff")
	float GetRemainingTimePercent() const;
};

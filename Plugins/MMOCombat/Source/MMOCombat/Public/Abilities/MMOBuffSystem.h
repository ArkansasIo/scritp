#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Abilities/MMOBuff.h"
#include "MMOBuffSystem.generated.h"

class AMMOCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBuffApplied, UMMOBuff*, Buff, AActor*, Source, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuffExpired, UMMOBuff*, Buff, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBuffRemoved, UMMOBuff*, Buff, AActor*, Target, bool, bDispelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBuffStackChanged, UMMOBuff*, Buff, int32, OldStacks, int32, NewStacks);

UENUM(BlueprintType)
enum class EMMODispelResult : uint8
{
	Success UMETA(DisplayName = "Success"),
	FailedWrongType UMETA(DisplayName = "Failed - Wrong Type"),
	FailedImmune UMETA(DisplayName = "Failed - Immune"),
	FailedNotFound UMETA(DisplayName = "Failed - Not Found"),
};

USTRUCT(BlueprintType)
struct FMMOActiveBuffEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Buff")
	UMMOBuff* BuffInstance = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Buff")
	float TimeRemaining = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Buff")
	float Duration = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Buff")
	int32 StackCount = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Buff")
	bool bInfinite = false;

	UPROPERTY(BlueprintReadWrite, Category = "Buff")
	AActor* SourceActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Buff")
	float TimeSinceLastTick = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Buff")
	int32 TickCount = 0;
};

UCLASS(ClassGroup=(MMO), meta=(BlueprintSpawnableComponent))
class MMOCOMBAT_API UMMOBuffSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOBuffSystemComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

// --- Active Buffs ---
	UPROPERTY(ReplicatedUsing = OnRep_ActiveBuffs, BlueprintReadOnly, Category = "Buffs")
	TArray<FMMOActiveBuffEntry> ActiveBuffEntries;

	UFUNCTION()
	void OnRep_ActiveBuffs();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBuffApplied OnBuffApplied;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBuffExpired OnBuffExpired;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBuffRemoved OnBuffRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBuffStackChanged OnBuffStackChanged;

// --- Core Operations ---
	UFUNCTION(BlueprintCallable, Category = "Buffs")
	UMMOBuff* ApplyBuff(TSubclassOf<UMMOBuff> BuffClass, AActor* Source, AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	bool RemoveBuff(UMMOBuff* BuffInstance, bool bDispelled = false);

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	bool RemoveBuffByID(FName BuffID, bool bDispelled = false);

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	void RemoveAllBuffs(bool bRemoveBeneficial = true, bool bRemoveHarmful = true, bool bDispelled = false);

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	void RemoveAllBuffsByTag(const FGameplayTagContainer& Tags, bool bDispelled = false);

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	void RemoveBuffsByType(EMMOBuffType BuffType, bool bDispelled = false);

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	void RefreshBuff(UMMOBuff* BuffInstance);

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	bool HasBuff(FName BuffID) const;

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	UMMOBuff* FindBuffByID(FName BuffID) const;

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	TArray<UMMOBuff*> FindBuffsByTag(const FGameplayTagContainer& Tags) const;

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	int32 GetBuffStackCount(FName BuffID) const;

// --- Dispel ---
	UFUNCTION(BlueprintCallable, Category = "Buffs")
	EMMODispelResult DispelBuff(EMMODispelType DispelType, int32 NumBuffsToDispel = 1);

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	EMMODispelResult DispelBuffsByType(EMMOBuffType BuffType, int32 NumBuffsToDispel = 1);

// --- Immunity ---
	UFUNCTION(BlueprintCallable, Category = "Buffs")
	bool IsImmuneToBuff(const UMMOBuff* Buff) const;

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	void AddImmunityCategory(const FGameplayTag& ImmunityTag, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	void RemoveImmunityCategory(const FGameplayTag& ImmunityTag);

	UPROPERTY(BlueprintReadOnly, Category = "Immunity")
	TArray<FGameplayTag> ActiveImmunityCategories;

// --- Periodic Effects ---
	void ProcessPeriodicEffect(const FMMOActiveBuffEntry& Entry, float DeltaTime);

// --- Cleanup ---
	void ExpireBuff(FMMOActiveBuffEntry& Entry);
	void InternalRemoveBuff(int32 Index, bool bDispelled);

protected:
	void Internal_ApplyBuff(UMMOBuff* BuffInstance, AActor* Source, AActor* Target);
	void Internal_RemoveBuff(int32 EntryIndex, bool bDispelled);

	UPROPERTY()
	TArray<UMMOBuff*> BuffPool;

	int32 FindBuffEntryIndex(FName BuffID) const;
	int32 FindBuffEntryIndexByInstance(const UMMOBuff* BuffInstance) const;
};

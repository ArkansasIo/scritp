#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Abilities/MMOAbilityBase.h"
#include "MMOAbilityComponent.generated.h"

class AMMOCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityCastStarted, UMMOAbilityBase*, Ability, AMMOCharacterBase*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityCastCompleted, UMMOAbilityBase*, Ability, AMMOCharacterBase*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityCastCancelled, UMMOAbilityBase*, Ability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityCooldownStarted, UMMOAbilityBase*, Ability, float, CooldownDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityCooldownExpired, UMMOAbilityBase*, Ability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityLearned, UMMOAbilityBase*, Ability, int32, Rank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCastResult, UMMOAbilityBase*, Ability, AMMOCharacterBase*, Target, bool, bCriticalHit);

USTRUCT(BlueprintType)
struct FMMOAbilitySlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Slot")
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Slot")
	TSoftObjectPtr<UMMOAbilityBase> Ability;

	UPROPERTY(BlueprintReadWrite, Category = "Slot")
	FKey BoundKey;
};

USTRUCT(BlueprintType)
struct FMMOAbilityCooldown
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Cooldown")
	TSoftObjectPtr<UMMOAbilityBase> Ability;

	UPROPERTY(BlueprintReadWrite, Category = "Cooldown")
	float Duration = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Cooldown")
	float Remaining = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Cooldown")
	float StartTime = 0.0f;
};

UCLASS(ClassGroup=(MMO), meta=(BlueprintSpawnableComponent))
class MMOCOMBAT_API UMMOAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOAbilityComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

// --- Known Abilities ---
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<UMMOAbilityBase*> KnownAbilities;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<FMMOAbilitySlot> HotbarSlots;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAbilityLearned OnAbilityLearned;

// --- Casting State ---
	UPROPERTY(ReplicatedUsing = OnRep_IsCasting, BlueprintReadOnly, Category = "Casting")
	bool bIsCasting = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Casting")
	UMMOAbilityBase* CurrentCastingAbility = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Casting")
	AMMOCharacterBase* CurrentCastTarget = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Casting")
	FVector CurrentCastLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Casting")
	float CurrentCastTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Casting")
	float CastElapsedTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Casting")
	FTimerHandle CastTimerHandle;

	UFUNCTION()
	void OnRep_IsCasting();

// --- Cooldowns ---
	UPROPERTY(BlueprintReadOnly, Category = "Cooldowns")
	TArray<FMMOAbilityCooldown> ActiveCooldowns;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAbilityCooldownStarted OnCooldownStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAbilityCooldownExpired OnCooldownExpired;

// --- Events ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAbilityCastStarted OnCastStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAbilityCastCompleted OnCastCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAbilityCastCancelled OnCastCancelled;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCastResult OnCastResult;

// --- Ability Learning ---
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool LearnAbility(TSoftObjectPtr<UMMOAbilityBase> AbilityClass, int32 Rank = 1);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool UnlearnAbility(FName AbilityID);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool RankUpAbility(FName AbilityID);

	UFUNCTION(BlueprintPure, Category = "Abilities")
	UMMOAbilityBase* FindAbilityByID(FName AbilityID) const;

	UFUNCTION(BlueprintPure, Category = "Abilities")
	bool HasAbility(FName AbilityID) const;

// --- Hotbar ---
	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	bool AssignAbilityToSlot(int32 SlotIndex, TSoftObjectPtr<UMMOAbilityBase> Ability);

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	bool RemoveAbilityFromSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	UMMOAbilityBase* GetAbilityInSlot(int32 SlotIndex) const;

// --- Casting ---
	UFUNCTION(BlueprintCallable, Category = "Casting")
	bool StartCast(int32 SlotIndex, const FVector& TargetLocation, AMMOCharacterBase* Target);

	UFUNCTION(BlueprintCallable, Category = "Casting")
	bool StartCastByAbility(UMMOAbilityBase* Ability, const FVector& TargetLocation, AMMOCharacterBase* Target);

	UFUNCTION(BlueprintCallable, Category = "Casting")
	void CancelCast(bool bPlayCancelledAnim = true);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartCast(UMMOAbilityBase* Ability, const FVector& TargetLocation, AMMOCharacterBase* Target);
	bool Server_StartCast_Validate(UMMOAbilityBase* Ability, const FVector& TargetLocation, AMMOCharacterBase* Target);
	void Server_StartCast_Implementation(UMMOAbilityBase* Ability, const FVector& TargetLocation, AMMOCharacterBase* Target);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CancelCast();
	bool Server_CancelCast_Validate();
	void Server_CancelCast_Implementation();

// --- Cast Results (Client RPCs) ---
	UFUNCTION(Client, Reliable)
	void Client_CastResult_Hit(UMMOAbilityBase* Ability, AMMOCharacterBase* Target, float Damage, bool bCritical);
	void Client_CastResult_Hit_Implementation(UMMOAbilityBase* Ability, AMMOCharacterBase* Target, float Damage, bool bCritical);

	UFUNCTION(Client, Reliable)
	void Client_CastResult_Miss(UMMOAbilityBase* Ability, AMMOCharacterBase* Target);
	void Client_CastResult_Miss_Implementation(UMMOAbilityBase* Ability, AMMOCharacterBase* Target);

	UFUNCTION(Client, Reliable)
	void Client_CastResult_Crit(UMMOAbilityBase* Ability, AMMOCharacterBase* Target, float Damage);
	void Client_CastResult_Crit_Implementation(UMMOAbilityBase* Ability, AMMOCharacterBase* Target, float Damage);

// --- Cooldown ---
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
	void StartCooldown(UMMOAbilityBase* Ability);

	UFUNCTION(BlueprintPure, Category = "Cooldown")
	bool IsOnCooldown(UMMOAbilityBase* Ability) const;

	UFUNCTION(BlueprintPure, Category = "Cooldown")
	float GetCooldownRemaining(UMMOAbilityBase* Ability) const;

// --- Client Prediction ---
	UFUNCTION(BlueprintCallable, Category = "Prediction")
	void PredictiveStartCast(UMMOAbilityBase* Ability, const FVector& TargetLocation, AMMOCharacterBase* Target);

	UFUNCTION(BlueprintCallable, Category = "Prediction")
	void ConfirmPredictedCast(UMMOAbilityBase* Ability);

	UFUNCTION(BlueprintCallable, Category = "Prediction")
	void RejectPredictedCast(UMMOAbilityBase* Ability);

// --- Utility ---
	UFUNCTION(BlueprintPure, Category = "Utility")
	AMMOCharacterBase* GetOwnerCharacter() const;

protected:
	void Internal_StartCast(UMMOAbilityBase* Ability, const FVector& TargetLocation, AMMOCharacterBase* Target);
	void Internal_CompleteCast();
	void AdvanceCast(float DeltaTime);

	UPROPERTY()
	TArray<UMMOAbilityBase*> PredictedCasts;

	UPROPERTY()
	TMap<FName, int32> AbilityRanks;
};

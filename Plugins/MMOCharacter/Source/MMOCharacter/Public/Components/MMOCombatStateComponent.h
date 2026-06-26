#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MMOCombatStateComponent.generated.h"

class AMMOCharacterBase;

UENUM(BlueprintType)
enum class EMMOCombatState : uint8
{
	OutOfCombat,
	InCombat,
	InPvP
};

USTRUCT(BlueprintType)
struct FThreatEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> ThreatActor;

	UPROPERTY()
	float ThreatValue;

	UPROPERTY()
	float LastDamageTime;

	FThreatEntry()
		: ThreatActor(nullptr)
		, ThreatValue(0.0f)
		, LastDamageTime(0.0f)
	{}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChanged, EMMOCombatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetLocked, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetUnlocked, AActor*, Target);

UCLASS(ClassGroup = (MMO), meta = (BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class MMCHARACTER_API UMMOCombatStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOCombatStateComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState, BlueprintReadOnly, Category = "MMO|Combat")
	EMMOCombatState CombatState;

	UFUNCTION()
	virtual void OnRep_CombatState(EMMOCombatState OldState);

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat")
	virtual void SetCombatState(EMMOCombatState NewState);

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat")
	virtual void EnterCombat();

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat")
	virtual void ExitCombat();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Combat|Timer")
	float CombatTimeoutDuration;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Combat|Timer")
	float CurrentCombatTimer;

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat|Timer")
	virtual void ResetCombatTimer();

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Combat|LockOn")
	TObjectPtr<AActor> LockedOnTarget;

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat|LockOn")
	virtual void LockOnTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat|LockOn")
	virtual void UnlockTarget();

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat|LockOn")
	virtual bool IsTargetLocked() const;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Combat|Threat")
	TArray<FThreatEntry> ThreatList;

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat|Threat")
	virtual void AddThreat(AActor* Actor, float ThreatAmount);

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat|Threat")
	virtual void RemoveThreat(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat|Threat")
	virtual void ClearThreatList();

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat|Threat")
	virtual AActor* GetHighestThreatTarget() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat|Threat")
	virtual float GetThreatValue(AActor* Actor) const;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Combat|History")
	TObjectPtr<AActor> LastAttacker;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Combat|History")
	float LastDamageTime;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Combat|History")
	float LastDamageAmount;

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat|History")
	virtual void RecordDamage(AActor* Attacker, float DamageAmount);

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnCombatStateChanged OnCombatStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnTargetLocked OnTargetLocked;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnTargetUnlocked OnTargetUnlocked;

protected:
	virtual void UpdateCombatTimer(float DeltaTime);
};

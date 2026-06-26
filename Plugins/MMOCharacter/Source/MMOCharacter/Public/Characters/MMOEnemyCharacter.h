#pragma once

#include "CoreMinimal.h"
#include "Characters/MMOCharacterBase.h"
#include "Engine/DataTable.h"
#include "MMOEnemyCharacter.generated.h"

class AAIController;

UENUM(BlueprintType)
enum class EMMOEnemyDifficultyTier : uint8
{
	Normal,
	Elite,
	Boss
};

UCLASS(Blueprintable, BlueprintType)
class MMCHARACTER_API AMMOEnemyCharacter : public AMMOCharacterBase
{
	GENERATED_BODY()

public:
	AMMOEnemyCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Die() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|AI")
	TSubclassOf<AAIController> AIControllerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|AI")
	TObjectPtr<AAIController> AIController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Combat")
	float AggroRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Combat")
	float LeashRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Combat")
	float LeashReturnSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Combat")
	float DeAggroTime;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Enemy|Combat")
	AActor* CurrentTarget;

	UFUNCTION(BlueprintCallable, Category = "MMO|Enemy|Combat")
	virtual void SetCurrentTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "MMO|Enemy|Combat")
	virtual bool IsInAggroRange(AActor* TargetActor) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Enemy|Combat")
	virtual bool IsInLeashRange() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Spawn")
	FName SpawnGroupID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Spawn")
	EMMOEnemyDifficultyTier DifficultyTier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Loot")
	FPrimaryAssetId LootTableID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Loot")
	bool bAllowDynamicLoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Rewards")
	int64 ExperienceReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Rewards")
	TMap<FPrimaryAssetId, int32> CurrencyRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Respawn")
	float RespawnTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Respawn")
	bool bAutoRespawn;

	UFUNCTION(BlueprintCallable, Category = "MMO|Enemy|Respawn")
	virtual void StartRespawn();

	UFUNCTION(BlueprintCallable, Category = "MMO|Enemy|Respawn")
	virtual void OnRespawnComplete();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Patrol")
	TArray<FVector> PatrolRoutePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Patrol")
	int32 CurrentPatrolIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Patrol")
	float PatrolWaitTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Patrol")
	bool bLoopPatrol;

	UFUNCTION(BlueprintCallable, Category = "MMO|Enemy|Patrol")
	virtual FVector GetNextPatrolPoint();

	UFUNCTION(BlueprintCallable, Category = "MMO|Enemy|Patrol")
	virtual void AdvancePatrol();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Quest")
	FName BountyQuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Quest")
	TArray<FName> QuestTargetTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Visual")
	TObjectPtr<class UParticleSystemComponent> SpawnEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Enemy|Visual")
	TObjectPtr<class UParticleSystemComponent> DeathEffect;

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "MMO|Enemy|Visual")
	virtual void MulticastSpawnEffect();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "MMO|Enemy|Visual")
	virtual void MulticastDeathEffect();

protected:
	void OnDeath() override;

	UFUNCTION()
	void SpawnVisualEffects();

	UPROPERTY()
	FTimerHandle RespawnTimerHandle;

	FVector SpawnOrigin;

	virtual void InitializeEnemyData();
};

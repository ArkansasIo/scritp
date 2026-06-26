#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "MMOCharacterBase.generated.h"

class UMMOCharacterMovementComponent;
class UMMOStatComponent;
class UMMOCombatStateComponent;
class UMMOInteractionComponent;
class UAbilitySystemComponent;
class UAnimInstance;

UENUM(BlueprintType)
enum class EMMOCharacterState : uint8
{
	Alive,
	Dead,
	Respawning,
	Spawned
};

UENUM(BlueprintType)
enum class EMMOCharacterType : uint8
{
	Player,
	NonPlayer,
	Enemy
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterStatChanged, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterRespawn);

UCLASS(Blueprintable, BlueprintType, Abstract)
class MMCHARACTER_API AMMOCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AMMOCharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "MMO|CharacterInfo")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "MMO|CharacterInfo")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "MMO|CharacterInfo")
	FText GuildTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "MMO|CharacterInfo")
	EMMOCharacterType CharacterType;

	UPROPERTY(ReplicatedUsing = OnRep_Level, BlueprintReadOnly, Category = "MMO|Stats")
	int32 Level;

	UPROPERTY(ReplicatedUsing = OnRep_Experience, BlueprintReadOnly, Category = "MMO|Stats")
	int64 Experience;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, BlueprintReadOnly, Category = "MMO|Stats")
	float CurrentHP;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentMP, BlueprintReadOnly, Category = "MMO|Stats")
	float CurrentMP;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Stats")
	float MaxHP;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Stats")
	float MaxMP;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Stats")
	float Stamina;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Stats")
	float MaxStamina;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "MMO|State")
	EMMOCharacterState CharacterState;

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats")
	virtual void AddExperience(int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats")
	virtual void SetLevel(int32 NewLevel);

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats")
	virtual void ModifyHealth(float Delta);

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats")
	virtual void ModifyMana(float Delta);

	UFUNCTION(BlueprintCallable, Category = "MMO|Stats")
	virtual void ModifyStamina(float Delta);

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat")
	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat")
	virtual void Respawn();

	UFUNCTION(BlueprintCallable, Category = "MMO|Combat")
	virtual void StartRespawnTimer(float RespawnDelay);

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual void OnBeginFocus();

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual void OnEndFocus();

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual void OnInteract(AMMOCharacterBase* InstigatorCharacter);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "MMO|Effects")
	virtual void MulticastLevelUpEffect();

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Effects")
	void BP_LevelUpEffect();

	UFUNCTION(BlueprintCallable, Category = "MMO|Animation")
	virtual void SetAnimationBlueprintClass(TSubclassOf<UAnimInstance> AnimClass);

	UFUNCTION(BlueprintCallable, Category = "MMO|Animation")
	virtual void SetAnimationBlueprintClassByPath(const FString& AnimBlueprintPath);

	UFUNCTION()
	virtual void OnRep_Level(int32 OldLevel);

	UFUNCTION()
	virtual void OnRep_Experience(int64 OldExperience);

	UFUNCTION()
	virtual void OnRep_CurrentHP(float OldHP);

	UFUNCTION()
	virtual void OnRep_CurrentMP(float OldMP);

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnCharacterLevelUp OnLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnCharacterDeath OnCharacterDeath;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnCharacterRespawn OnCharacterRespawn;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnCharacterStatChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnCharacterStatChanged OnMPChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Components")
	TObjectPtr<UMMOStatComponent> StatComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Components")
	TObjectPtr<UMMOCombatStateComponent> CombatStateComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Components")
	TObjectPtr<UMMOInteractionComponent> InteractionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMO|Movement")
	TSubclassOf<UMMOCharacterMovementComponent> MovementComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMO|Animation")
	TSubclassOf<UAnimInstance> WalkAnimInstanceClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMO|Animation")
	TSubclassOf<UAnimInstance> CombatAnimInstanceClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMO|Animation")
	TSubclassOf<UAnimInstance> MountAnimInstanceClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Highlight")
	int32 CustomDepthStencilValue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Highlight")
	bool bHighlightable;

	UFUNCTION(BlueprintCallable, Category = "MMO|Highlight")
	virtual void SetHighlight(bool bEnabled);

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Regen")
	float HealthRegenRate;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Regen")
	float ManaRegenRate;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Regen")
	float StaminaRegenRate;

	UPROPERTY()
	FTimerHandle RegenTimerHandle;

	UFUNCTION()
	virtual void ApplyRegeneration();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Regen")
	float RegenInterval;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Regen")
	float OutOfCombatRegenMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Interaction")
	bool bIsInFocus;

protected:
	virtual void InitializeComponents();
	virtual void StartRegeneration();
	virtual void StopRegeneration();
	void TickCharacter(float DeltaTime);

	bool bIsRespawnTimerActive;

	UPROPERTY()
	FTimerHandle RespawnTimerHandle;

	UFUNCTION()
	void OnRespawnTimerComplete();

	virtual void OnDeath();
	virtual void OnRespawn();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastDeath();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastRespawn();
};

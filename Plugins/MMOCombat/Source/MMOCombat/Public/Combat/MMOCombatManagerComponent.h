#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Abilities/MMODamageSystem.h"
#include "MMOCombatManagerComponent.generated.h"

class AMMOCharacterBase;

UENUM(BlueprintType)
enum class EMMODuelState : uint8
{
	None UMETA(DisplayName = "None"),
	Requested UMETA(DisplayName = "Requested"),
	Accepted UMETA(DisplayName = "Accepted"),
	Active UMETA(DisplayName = "Active"),
	Completed UMETA(DisplayName = "Completed"),
	Declined UMETA(DisplayName = "Declined"),
};

UENUM(BlueprintType)
enum class EMMOPvPState : uint8
{
	Peaceful UMETA(DisplayName = "Peaceful"),
	PvPEnabled UMETA(DisplayName = "PvP Enabled"),
	InCombat UMETA(DisplayName = "In Combat"),
	Flagged UMETA(DisplayName = "Flagged"),
	RecentlyEngaged UMETA(DisplayName = "Recently Engaged"),
};

UENUM(BlueprintType)
enum class EMMORespawnMethod : uint8
{
	BindPoint UMETA(DisplayName = "Bind Point"),
	Graveyard UMETA(DisplayName = "Graveyard"),
	ReleaseSpirit UMETA(DisplayName = "Release Spirit"),
	Instant UMETA(DisplayName = "Instant"),
};

USTRUCT(BlueprintType)
struct FMMOThreatEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Threat")
	AActor* ThreatSource = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Threat")
	float ThreatValue = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Threat")
	float LastThreatUpdateTime = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Threat")
	bool bIsInCombat = false;
};

USTRUCT(BlueprintType)
struct FMMOCombatLogEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Log")
	float Timestamp = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Log")
	FText EventDescription;

	UPROPERTY(BlueprintReadWrite, Category = "Log")
	AActor* SourceActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Log")
	AActor* TargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Log")
	float Value = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Log")
	EMMOHitResult HitResult = EMMOHitResult::Normal;

	UPROPERTY(BlueprintReadWrite, Category = "Log")
	FGameplayTagContainer EventTags;
};

USTRUCT(BlueprintType)
struct FMMODuelInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Duel")
	AMMOCharacterBase* Challenger = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Duel")
	AMMOCharacterBase* Opponent = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Duel")
	EMMODuelState DuelState = EMMODuelState::None;

	UPROPERTY(BlueprintReadWrite, Category = "Duel")
	float RequestTime = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Duel")
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Duel")
	AMMOCharacterBase* Winner = nullptr;
};

USTRUCT(BlueprintType)
struct FMMORespawnData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Respawn")
	EMMORespawnMethod RespawnMethod = EMMORespawnMethod::BindPoint;

	UPROPERTY(BlueprintReadWrite, Category = "Respawn")
	FVector RespawnLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Respawn")
	float RespawnDelay = 5.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Respawn")
	bool bHasReleasedSpirit = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatEnter, AActor*, SelfActor, AActor*, Opponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatExit, AActor*, SelfActor, AActor*, Opponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorDeath, AActor*, DeadActor, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageDealt, AActor*, Source, AActor*, Target, const FMMODamageResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageTaken, AActor*, Source, AActor*, Target, const FMMODamageResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDuelRequested, AMMOCharacterBase*, Challenger, AMMOCharacterBase*, Opponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDuelStarted, AMMOCharacterBase*, Challenger, AMMOCharacterBase*, Opponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDuelEnded, AMMOCharacterBase*, Winner, AMMOCharacterBase*, Loser, bool, bForfeit);

UCLASS(ClassGroup=(MMO), meta=(BlueprintSpawnableComponent))
class MMOCOMBAT_API UMMOCombatManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOCombatManagerComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

// --- Combat State ---
	UPROPERTY(ReplicatedUsing = OnRep_CombatState, BlueprintReadOnly, Category = "Combat")
	bool bIsInCombat = false;

	UPROPERTY(ReplicatedUsing = OnRep_PvPState, BlueprintReadOnly, Category = "Combat")
	EMMOPvPState PvPState = EMMOPvPState::Peaceful;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	float CombatTimer = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float OutOfCombatDelay = 6.0f;

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION()
	void OnRep_PvPState();

// --- Events ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCombatEnter OnCombatEnter;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCombatExit OnCombatExit;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnActorDeath OnActorDeath;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDamageDealt OnDamageDealt;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDamageTaken OnDamageTaken;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDuelRequested OnDuelRequested;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDuelStarted OnDuelStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDuelEnded OnDuelEnded;

// --- Threat Management ---
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	TArray<FMMOThreatEntry> ThreatList;

	UFUNCTION(BlueprintCallable, Category = "Threat")
	void AddThreat(AActor* Source, float Amount);

	UFUNCTION(BlueprintCallable, Category = "Threat")
	void RemoveThreat(AActor* Source);

	UFUNCTION(BlueprintCallable, Category = "Threat")
	void ClearAllThreat();

	UFUNCTION(BlueprintPure, Category = "Threat")
	AActor* GetHighestThreatTarget() const;

	UFUNCTION(BlueprintPure, Category = "Threat")
	float GetThreatForActor(AActor* Source) const;

// --- PvP ---
	UFUNCTION(BlueprintCallable, Category = "PvP")
	void EnablePvP();

	UFUNCTION(BlueprintCallable, Category = "PvP")
	void DisablePvP();

	UFUNCTION(BlueprintPure, Category = "PvP")
	bool CanAttack(AActor* Target) const;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_FlagForPvP();
	bool Server_FlagForPvP_Validate();
	void Server_FlagForPvP_Implementation();

// --- Duel System ---
	UFUNCTION(BlueprintCallable, Category = "Duel")
	void RequestDuel(AMMOCharacterBase* Opponent);

	UFUNCTION(BlueprintCallable, Category = "Duel")
	void AcceptDuel();

	UFUNCTION(BlueprintCallable, Category = "Duel")
	void DeclineDuel();

	UFUNCTION(BlueprintCallable, Category = "Duel")
	void ForfeitDuel();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestDuel(AMMOCharacterBase* Opponent);
	bool Server_RequestDuel_Validate(AMMOCharacterBase* Opponent);
	void Server_RequestDuel_Implementation(AMMOCharacterBase* Opponent);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AcceptDuel(AMMOCharacterBase* Challenger);
	bool Server_AcceptDuel_Validate(AMMOCharacterBase* Challenger);
	void Server_AcceptDuel_Implementation(AMMOCharacterBase* Challenger);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_DeclineDuel(AMMOCharacterBase* Challenger);
	bool Server_DeclineDuel_Validate(AMMOCharacterBase* Challenger);
	void Server_DeclineDuel_Implementation(AMMOCharacterBase* Challenger);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_EndDuel(AMMOCharacterBase* Winner, AMMOCharacterBase* Loser, bool bForfeit);
	bool Server_EndDuel_Validate(AMMOCharacterBase* Winner, AMMOCharacterBase* Loser, bool bForfeit);
	void Server_EndDuel_Implementation(AMMOCharacterBase* Winner, AMMOCharacterBase* Loser, bool bForfeit);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Duel")
	FMMODuelInfo ActiveDuel;

	UPROPERTY(BlueprintReadOnly, Category = "Duel")
	float DuelRequestTimeout = 30.0f;

// --- Death & Respawn ---
	UFUNCTION(BlueprintCallable, Category = "Death")
	void OnDeath(AActor* Killer);

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SetBindPoint(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void ReleaseSpirit();

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void RespawnAtBindPoint();

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void RespawnAtGraveyard();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Respawn(EMMORespawnMethod Method);
	bool Server_Respawn_Validate(EMMORespawnMethod Method);
	void Server_Respawn_Implementation(EMMORespawnMethod Method);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Respawn")
	FVector BindPointLocation;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Respawn")
	bool bIsDead = false;

// --- Combat Log ---
	UPROPERTY(BlueprintReadOnly, Category = "Log")
	TArray<FMMOCombatLogEntry> CombatLog;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log")
	int32 MaxCombatLogEntries = 200;

	UFUNCTION(BlueprintCallable, Category = "Log")
	void AddCombatLogEntry(const FText& Description, AActor* Source, AActor* Target, float Value = 0.0f, EMMOHitResult HitResult = EMMOHitResult::Normal, const FGameplayTagContainer& EventTags = FGameplayTagContainer());

	UFUNCTION(BlueprintCallable, Category = "Log")
	void ClearCombatLog();

// --- Combat Helpers ---
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EnterCombat(AActor* Opponent);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ExitCombat(AActor* Opponent);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanAttackTarget(AActor* Target) const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	AMMOCharacterBase* GetOwnerCharacter() const;
};

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Engine/HitResult.h"
#include "MMOTargetingSystem.generated.h"

class AMMOCharacterBase;

UENUM(BlueprintType)
enum class EMMOTargetPriority : uint8
{
	Nearest UMETA(DisplayName = "Nearest"),
	LowestHealth UMETA(DisplayName = "Lowest Health"),
	HighestThreat UMETA(DisplayName = "Highest Threat"),
	Focused UMETA(DisplayName = "Focused Target"),
	ConeInFront UMETA(DisplayName = "Cone In Front"),
	CurrentTarget UMETA(DisplayName = "Current Target"),
};

UENUM(BlueprintType)
enum class EMMOTargetTypeFilter : uint8
{
	Enemy UMETA(DisplayName = "Enemy"),
	Friendly UMETA(DisplayName = "Friendly"),
	Self UMETA(DisplayName = "Self"),
	Corpse UMETA(DisplayName = "Corpse"),
	Any UMETA(DisplayName = "Any"),
	All UMETA(DisplayName = "All"),
};

UENUM(BlueprintType)
enum class EMMOTargetingMode : uint8
{
	Soft UMETA(DisplayName = "Soft (Hover)"),
	Hard UMETA(DisplayName = "Hard (Selected)"),
};

USTRUCT(BlueprintType)
struct FMMOTargetingFilter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	EMMOTargetTypeFilter TypeFilter = EMMOTargetTypeFilter::Enemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	float MaxRange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bAliveOnly = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bIncludeSelf = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	FGameplayTagContainer BlockedTags;
};

USTRUCT(BlueprintType)
struct FMMOTargetInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Target")
	AMMOCharacterBase* TargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Target")
	EMMOTargetingMode TargetingMode = EMMOTargetingMode::Hard;

	UPROPERTY(BlueprintReadWrite, Category = "Target")
	float LastValidatedTime = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Target")
	FVector GroundTargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Target")
	bool bHasGroundTarget = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, AMMOCharacterBase*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoftTargetChanged, AMMOCharacterBase*, NewSoftTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGroundTargetChanged, const FVector&, Location);

UCLASS(ClassGroup=(MMO), meta=(BlueprintSpawnableComponent))
class MMOCOMBAT_API UMMOTargetingSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOTargetingSystemComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

// --- Current Target ---
	UPROPERTY(ReplicatedUsing = OnRep_HardTarget, BlueprintReadOnly, Category = "Target")
	AMMOCharacterBase* HardTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Target")
	AMMOCharacterBase* SoftTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Target")
	FVector GroundTargetLocation = FVector::ZeroVector;

	UFUNCTION()
	void OnRep_HardTarget();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTargetChanged OnHardTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSoftTargetChanged OnSoftTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGroundTargetChanged OnGroundTargetChanged;

// --- Target Selection ---
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void SetHardTarget(AMMOCharacterBase* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ClearHardTarget();

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void SetSoftTarget(AMMOCharacterBase* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ClearSoftTarget();

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void SetGroundTarget(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ClearGroundTarget();

// --- Tab Targeting ---
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void TabTargetForward(const FMMOTargetingFilter& Filter);

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void TabTargetBackward(const FMMOTargetingFilter& Filter);

// --- Smart Selection ---
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	AMMOCharacterBase* FindBestTarget(EMMOTargetPriority Priority, const FMMOTargetingFilter& Filter) const;

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	TArray<AMMOCharacterBase*> FindTargetsInCone(float HalfAngle, float Range, const FMMOTargetingFilter& Filter) const;

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	TArray<AMMOCharacterBase*> FindTargetsInRadius(const FVector& Origin, float Radius, const FMMOTargetingFilter& Filter) const;

// --- Click Targeting ---
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	bool ClickToTarget(APlayerController* PC, EMMOTargetTypeFilter TargetFilter);

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	bool TraceForTarget(const FVector& Start, const FVector& End, FHitResult& OutHit, EMMOTargetTypeFilter TargetFilter) const;

// --- Validation ---
	UFUNCTION(BlueprintPure, Category = "Targeting")
	bool IsValidTarget(AMMOCharacterBase* Target, const FMMOTargetingFilter& Filter) const;

	UFUNCTION(BlueprintPure, Category = "Targeting")
	bool HasLineOfSight(AMMOCharacterBase* Target) const;

	UFUNCTION(BlueprintPure, Category = "Targeting")
	bool IsInRange(AMMOCharacterBase* Target, float Range) const;

	UFUNCTION(BlueprintPure, Category = "Targeting")
	bool IsValidTargetForAbility(AMMOCharacterBase* Target, class UMMOAbilityBase* Ability) const;

// --- Retical / Decal ---
	UFUNCTION(BlueprintImplementableEvent, Category = "Targeting")
	void ShowGroundTargetingDecal(const FVector& Location, float Radius);

	UFUNCTION(BlueprintImplementableEvent, Category = "Targeting")
	void HideGroundTargetingDecal();

	UFUNCTION(BlueprintImplementableEvent, Category = "Targeting")
	void ShowTargetingReticle(AMMOCharacterBase* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Targeting")
	void HideTargetingReticle();

// --- Get Targets ---
	UFUNCTION(BlueprintPure, Category = "Targeting")
	AMMOCharacterBase* GetHardTarget() const { return HardTarget; }

	UFUNCTION(BlueprintPure, Category = "Targeting")
	AMMOCharacterBase* GetSoftTarget() const { return SoftTarget; }

	UFUNCTION(BlueprintPure, Category = "Targeting")
	AMMOCharacterBase* GetEffectiveTarget() const;

protected:
	UPROPERTY()
	TArray<AMMOCharacterBase*> CachedEnemyList;

	void RefreshEnemyList();
	bool IsEnemy(AMMOCharacterBase* Target) const;
	bool IsFriendly(AMMOCharacterBase* Target) const;
	int32 CurrentTabIndex = 0;
};

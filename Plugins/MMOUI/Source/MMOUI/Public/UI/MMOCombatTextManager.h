#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Abilities/MMODamageSystem.h"
#include "MMOCombatTextManager.generated.h"

class AMMOFloatingTextActor;

USTRUCT(BlueprintType)
struct MMOUI_API FMMOFloatingTextEntry
{
	GENERATED_BODY()

	UPROPERTY()
	float Value = 0.0f;

	UPROPERTY()
	EMMOHitResult HitResult = EMMOHitResult::Normal;

	UPROPERTY()
	EMMODamageType DamageType = EMMODamageType::Physical;

	UPROPERTY()
	bool bIsHealing = false;

	UPROPERTY()
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY()
	bool bIsBigHit = false;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMMOOnCombatTextSpawned, const FMMOFloatingTextEntry&, Entry, AMMOFloatingTextActor*, TextActor);

UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class MMOUI_API UMMOCombatTextManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOCombatTextManager();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Combat Text")
	void SpawnDamageText(float Damage, EMMOHitResult HitResult, EMMODamageType DamageType, AActor* Target, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Combat Text")
	void SpawnHealingText(float Healing, EMMOHitResult HitResult, AActor* Target, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Combat Text")
	void SpawnMissText(AActor* Target, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Combat Text")
	void SpawnDodgeText(AActor* Target, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Combat Text")
	void SpawnParryText(AActor* Target, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Combat Text")
	void SpawnBlockText(AActor* Target, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Combat Text")
	void SpawnGlanceText(AActor* Target, const FVector& Location);

	UFUNCTION(BlueprintPure, Category = "Combat Text")
	FLinearColor GetDamageColor(EMMODamageType DamageType) const;

	UFUNCTION(BlueprintPure, Category = "Combat Text")
	FLinearColor GetHealingColor() const { return FLinearColor::Green; }

	UFUNCTION(BlueprintCallable, Category = "Combat Text")
	void PlayScreenShake(float Intensity = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Pooling")
	AMMOFloatingTextActor* AcquireFloatingText();

	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void ReleaseFloatingText(AMMOFloatingTextActor* TextActor);

	UPROPERTY(BlueprintAssignable, Category = "Combat Text")
	FMMOOnCombatTextSpawned OnCombatTextSpawned;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pooling")
	int32 PoolSize = 50;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pooling")
	TSubclassOf<AMMOFloatingTextActor> FloatingTextActorClass;

	UPROPERTY()
	TArray<TObjectPtr<AMMOFloatingTextActor>> FloatingTextPool;

	UPROPERTY()
	TArray<TObjectPtr<AMMOFloatingTextActor>> ActiveFloatingTexts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Screen Shake")
	TSubclassOf<UCameraShakeBase> BigHitShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FLinearColor PhysicalColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FLinearColor MagicColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FLinearColor FireColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FLinearColor FrostColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FLinearColor NatureColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FLinearColor ShadowColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FLinearColor HolyColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FLinearColor ArcaneColor;

	void InitializePool();
	AMMOFloatingTextActor* CreateFloatingTextActor();
	void SpawnTextIndicator(const FMMOFloatingTextEntry& Entry);
	void CleanupExpiredTexts();
};

UCLASS()
class MMOUI_API AMMOFloatingTextActor : public AActor
{
	GENERATED_BODY()

public:
	AMMOFloatingTextActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Floating Text")
	void OnSpawn(const FMMOFloatingTextEntry& Entry);

	UFUNCTION(BlueprintCallable, Category = "Floating Text")
	void SetTextEntry(const FMMOFloatingTextEntry& Entry);

	UFUNCTION(BlueprintCallable, Category = "Floating Text")
	void StartFloating();

	UFUNCTION(BlueprintCallable, Category = "Floating Text")
	void ReturnToPool();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Floating Text")
	float FloatDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Floating Text")
	float FloatHeight = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Floating Text")
	float FloatSpeed = 50.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsFloating = false;

protected:
	FMMOFloatingTextEntry TextEntry;

	float ElapsedTime;
};

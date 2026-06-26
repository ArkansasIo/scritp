#pragma once

#include "CoreMinimal.h"
#include "Items/MMOItemBase.h"
#include "MMOConsumableItem.generated.h"

UCLASS(BlueprintType, Blueprintable, EditInlineNew)
class MMOINVENTORY_API UMMOConsumableItem : public UMMOItemBase
{
	GENERATED_BODY()

public:
	UMMOConsumableItem();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Effect")
	EMMOConsumableEffect EffectType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Effect")
	TArray<float> EffectValues;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Duration")
	float EffectDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Cooldown")
	float CooldownDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Use")
	bool bIsInstant;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Use")
	int32 StacksConsumedPerUse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable|Target")
	EMMOConsumableTarget TargetType;

	virtual bool CanUse_Implementation(APlayerController* User) override;
	virtual bool OnUse_Implementation(APlayerController* User) override;

	UFUNCTION(BlueprintCallable, Category = "Consumable")
	bool ApplyEffect(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Consumable")
	float GetTotalEffectValue() const;

	UFUNCTION(BlueprintCallable, Category = "Consumable")
	bool HasCooldown() const;
};

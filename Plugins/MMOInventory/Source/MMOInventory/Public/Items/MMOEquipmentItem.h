#pragma once

#include "CoreMinimal.h"
#include "Items/MMOItemBase.h"
#include "MMOEquipmentItem.generated.h"

class USkeletalMesh;

USTRUCT(BlueprintType)
struct MMOINVENTORY_API FSetBonusInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Set Bonus")
	FName SetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Set Bonus")
	FText SetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Set Bonus")
	TMap<int32, FText> PieceBonuses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Set Bonus")
	int32 PiecesRequired;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew)
class MMOINVENTORY_API UMMOEquipmentItem : public UMMOItemBase
{
	GENERATED_BODY()

public:
	UMMOEquipmentItem();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Slot")
	EMMOEquipmentSlot EquipmentSlot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Stats")
	TMap<FName, float> PrimaryStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Stats")
	TMap<FName, float> SecondaryStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Stats")
	float ArmorValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Stats")
	float DamageValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Durability")
	float Durability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Durability")
	float MaxDurability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Sockets")
	int32 SocketCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	TSoftObjectPtr<USkeletalMesh> EquippedMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Enchantment")
	bool bCanBeEnchanted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Enchantment")
	int32 EnchantmentSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|SetBonus")
	FSetBonusInfo SetBonus;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	float GetStatValue(FName StatName) const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	TMap<FName, float> GetAllStats() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	float GetDamageReduction() const;

	virtual bool CanUse_Implementation(APlayerController* User) override;
	virtual bool OnUse_Implementation(APlayerController* User) override;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void DamageItem(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void RepairItem(float RepairAmount);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool NeedsRepair() const;
};

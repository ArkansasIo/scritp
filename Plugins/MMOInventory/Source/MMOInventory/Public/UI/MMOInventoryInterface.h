#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Items/MMOItemBase.h"
#include "Items/MMOItemContainer.h"
#include "MMOInventoryInterface.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint = false))
class MMOINVENTORY_API UMMOInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

class MMOINVENTORY_API IMMOInventoryInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	class UMMOInventoryComponent* GetInventoryComponent() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	UMMOItemContainer* GetInventoryContainer() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	UMMOItemContainer* GetEquipmentContainer() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void GetCurrency(int32& OutGold, int32& OutSilver, int32& OutCopper) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void NotifyInventoryChanged();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void ShowItemTooltip(const FItemInstance& Item, const FVector2D& ScreenPosition);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void HideTooltip();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void ShowContextMenu(const FItemInstance& Item, const FVector2D& ScreenPosition);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void OnItemUsed(const FItemInstance& Item);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void OnItemEquipped(const FItemInstance& Item);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void OnItemUnequipped(EMMOEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void OnItemDropped(const FItemInstance& Item, int32 Quantity);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void OnItemSplit(const FItemInstance& Item, int32 Quantity);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void OnItemMovedToBank(const FItemInstance& Item, int32 Quantity);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void OnItemMovedFromBank(const FItemInstance& Item, int32 Quantity);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|UI")
	void OnCurrencyUpdated(int32 Gold, int32 Silver, int32 Copper);
};

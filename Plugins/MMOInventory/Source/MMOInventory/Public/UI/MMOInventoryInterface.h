#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Items/MMOItemBase.h"
#include "Items/MMOItemContainer.h"
#include "MMOInventoryInterface.generated.h"

UINTERFACE()
class MMOINVENTORY_API UMMOInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

class MMOINVENTORY_API IMMOInventoryInterface
{
	GENERATED_BODY()

public:
	virtual class UMMOInventoryComponent* GetInventoryComponent() const = 0;

	virtual UMMOItemContainer* GetInventoryContainer() const = 0;

	virtual UMMOItemContainer* GetEquipmentContainer() const = 0;

	virtual void GetCurrency(int32& OutGold, int32& OutSilver, int32& OutCopper) const = 0;

	virtual void NotifyInventoryChanged() = 0;

	virtual void ShowItemTooltip(const FItemInstance& Item, const FVector2D& ScreenPosition) = 0;

	virtual void HideTooltip() = 0;

	virtual void ShowContextMenu(const FItemInstance& Item, const FVector2D& ScreenPosition) = 0;

	virtual void OnItemUsed(const FItemInstance& Item) = 0;

	virtual void OnItemEquipped(const FItemInstance& Item) = 0;

	virtual void OnItemUnequipped(EMMOEquipmentSlot Slot) = 0;

	virtual void OnItemDropped(const FItemInstance& Item, int32 Quantity) = 0;

	virtual void OnItemSplit(const FItemInstance& Item, int32 Quantity) = 0;

	virtual void OnItemMovedToBank(const FItemInstance& Item, int32 Quantity) = 0;

	virtual void OnItemMovedFromBank(const FItemInstance& Item, int32 Quantity) = 0;

	virtual void OnCurrencyUpdated(int32 Gold, int32 Silver, int32 Copper) = 0;
};

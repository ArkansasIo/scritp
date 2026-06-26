#pragma once

#include "CoreMinimal.h"
#include "Items/MMOItemBase.h"
#include "MMOItemContainer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerItemAdded, const FItemInstance&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerItemRemoved, const FItemInstance&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnContainerItemChanged, int32, Index, const FItemInstance&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContainerUpdated);

UCLASS(BlueprintType, Blueprintable, EditInlineNew)
class MMOINVENTORY_API UMMOItemContainer : public UObject
{
	GENERATED_BODY()

public:
	UMMOItemContainer();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	TArray<FItemInstance> Items;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
	EMMOContainerType ContainerType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
	int32 MaxSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
	float MaxWeight;

	UPROPERTY(BlueprintAssignable, Category = "Container|Events")
	FOnContainerItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Container|Events")
	FOnContainerItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Container|Events")
	FOnContainerItemChanged OnItemChanged;

	UPROPERTY(BlueprintAssignable, Category = "Container|Events")
	FOnContainerUpdated OnContainerUpdated;

	UFUNCTION(BlueprintCallable, Category = "Container")
	int32 AddItem(const FItemInstance& Item);

	UFUNCTION(BlueprintCallable, Category = "Container")
	bool RemoveItem(int32 Index, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Container")
	bool MoveItem(int32 FromIndex, int32 ToIndex);

	UFUNCTION(BlueprintCallable, Category = "Container")
	bool SwapItems(int32 IndexA, int32 IndexB);

	UFUNCTION(BlueprintCallable, Category = "Container")
	float GetTotalWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Container")
	int32 GetItemCount() const;

	UFUNCTION(BlueprintCallable, Category = "Container")
	int32 GetEmptySlots() const;

	UFUNCTION(BlueprintCallable, Category = "Container")
	int32 FindItemIndex(FName ItemId) const;

	UFUNCTION(BlueprintCallable, Category = "Container")
	bool HasSpaceFor(const FItemInstance& Item) const;

	UFUNCTION(BlueprintCallable, Category = "Container")
	void ClearContainer();

	UFUNCTION(BlueprintCallable, Category = "Container")
	void SortByID();

	UFUNCTION(BlueprintCallable, Category = "Container")
	void SortByRarity();

	UFUNCTION(BlueprintCallable, Category = "Container")
	void SortByType();

protected:
	UFUNCTION(BlueprintCallable, Category = "Container")
	int32 FindOrCreateStack(const FItemInstance& Item);
};

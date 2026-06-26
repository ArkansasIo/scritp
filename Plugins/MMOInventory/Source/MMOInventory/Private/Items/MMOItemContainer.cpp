#include "Items/MMOItemContainer.h"

UMMOItemContainer::UMMOItemContainer()
	: ContainerType(EMMOContainerType::Inventory)
	, MaxSlots(40)
	, MaxWeight(100.0f)
{
}

int32 UMMOItemContainer::AddItem(const FItemInstance& Item)
{
	if (!Item.ItemDefinition.IsValid() && Item.ItemDefinition.IsNull())
	{
		return INDEX_NONE;
	}

	if (!HasSpaceFor(Item))
	{
		return INDEX_NONE;
	}

	UMMOItemBase* ItemDef = Item.ItemDefinition.LoadSynchronous();
	if (!ItemDef)
	{
		return INDEX_NONE;
	}

	int32 StackIndex = FindOrCreateStack(Item);

	if (StackIndex != INDEX_NONE)
	{
		FItemInstance& ExistingItem = Items[StackIndex];
		int32 AddAmount = FMath::Min(Item.Quantity, ItemDef->MaxStackSize - ExistingItem.Quantity);
		ExistingItem.Quantity += AddAmount;
		OnItemChanged.Broadcast(StackIndex, ExistingItem);
		OnContainerUpdated.Broadcast();
		return StackIndex;
	}

	if (Items.Num() < MaxSlots)
	{
		FItemInstance NewItem = Item;
		NewItem.UniqueId = FGuid::NewGuid();
		if (ItemDef->MaxStackSize > 0)
		{
			NewItem.Quantity = FMath::Min(Item.Quantity, ItemDef->MaxStackSize);
		}
		int32 NewIndex = Items.Add(NewItem);
		OnItemAdded.Broadcast(NewItem);
		OnContainerUpdated.Broadcast();
		return NewIndex;
	}

	return INDEX_NONE;
}

bool UMMOItemContainer::RemoveItem(int32 Index, int32 Quantity)
{
	if (!Items.IsValidIndex(Index))
	{
		return false;
	}

	FItemInstance& Item = Items[Index];
	int32 RemoveAmount = FMath::Min(Quantity, Item.Quantity);
	Item.Quantity -= RemoveAmount;

	if (Item.Quantity <= 0)
	{
		FItemInstance RemovedItem = Item;
		Items.RemoveAt(Index);
		OnItemRemoved.Broadcast(RemovedItem);
	}
	else
	{
		OnItemChanged.Broadcast(Index, Item);
	}

	OnContainerUpdated.Broadcast();
	return true;
}

bool UMMOItemContainer::MoveItem(int32 FromIndex, int32 ToIndex)
{
	if (!Items.IsValidIndex(FromIndex) || !Items.IsValidIndex(ToIndex))
	{
		return false;
	}

	if (FromIndex == ToIndex)
	{
		return true;
	}

	FItemInstance Temp = Items[FromIndex];
	Items.RemoveAt(FromIndex);
	Items.Insert(Temp, ToIndex);

	OnContainerUpdated.Broadcast();
	return true;
}

bool UMMOItemContainer::SwapItems(int32 IndexA, int32 IndexB)
{
	if (!Items.IsValidIndex(IndexA) || !Items.IsValidIndex(IndexB))
	{
		return false;
	}

	if (IndexA == IndexB)
	{
		return true;
	}

	Items.Swap(IndexA, IndexB);

	OnItemChanged.Broadcast(IndexA, Items[IndexA]);
	OnItemChanged.Broadcast(IndexB, Items[IndexB]);
	OnContainerUpdated.Broadcast();
	return true;
}

float UMMOItemContainer::GetTotalWeight() const
{
	float Total = 0.0f;
	for (const FItemInstance& Item : Items)
	{
		if (Item.ItemDefinition.IsValid())
		{
			UMMOItemBase* ItemDef = Item.ItemDefinition.LoadSynchronous();
			if (ItemDef)
			{
				Total += ItemDef->Weight * Item.Quantity;
			}
		}
	}
	return Total;
}

int32 UMMOItemContainer::GetItemCount() const
{
	int32 Count = 0;
	for (const FItemInstance& Item : Items)
	{
		Count += Item.Quantity;
	}
	return Count;
}

int32 UMMOItemContainer::GetEmptySlots() const
{
	return FMath::Max(0, MaxSlots - Items.Num());
}

int32 UMMOItemContainer::FindItemIndex(FName ItemId) const
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i].ItemDefinition.IsValid())
		{
			UMMOItemBase* ItemDef = Items[i].ItemDefinition.LoadSynchronous();
			if (ItemDef && ItemDef->ItemID == ItemId)
			{
				return i;
			}
		}
	}
	return INDEX_NONE;
}

bool UMMOItemContainer::HasSpaceFor(const FItemInstance& Item) const
{
	if (Item.ItemDefinition.IsNull())
	{
		return false;
	}

	if (Items.Num() >= MaxSlots)
	{
		UMMOItemBase* ItemDef = Item.ItemDefinition.LoadSynchronous();
		if (!ItemDef)
		{
			return false;
		}

		bool bFoundStack = false;
		for (const FItemInstance& ExistingItem : Items)
		{
			if (ExistingItem.ItemDefinition.IsValid())
			{
				UMMOItemBase* ExistingDef = ExistingItem.ItemDefinition.LoadSynchronous();
				if (ExistingDef && ExistingDef->ItemID == ItemDef->ItemID && ExistingItem.Quantity < ExistingDef->MaxStackSize)
				{
					bFoundStack = true;
					break;
				}
			}
		}

		if (!bFoundStack)
		{
			return false;
		}
	}

	if (MaxWeight > 0.0f)
	{
		UMMOItemBase* ItemDef = Item.ItemDefinition.LoadSynchronous();
		if (ItemDef)
		{
			float CurrentWeight = GetTotalWeight();
			float ItemTotalWeight = ItemDef->Weight * Item.Quantity;
			if (CurrentWeight + ItemTotalWeight > MaxWeight)
			{
				return false;
			}
		}
	}

	return true;
}

void UMMOItemContainer::ClearContainer()
{
	Items.Empty();
	OnContainerUpdated.Broadcast();
}

void UMMOItemContainer::SortByID()
{
	Items.Sort([](const FItemInstance& A, const FItemInstance& B)
	{
		UMMOItemBase* DefA = A.ItemDefinition.LoadSynchronous();
		UMMOItemBase* DefB = B.ItemDefinition.LoadSynchronous();
		if (!DefA || !DefB)
		{
			return DefA != nullptr;
		}
		return DefA->ItemID.LexicalLess(DefB->ItemID);
	});
	OnContainerUpdated.Broadcast();
}

void UMMOItemContainer::SortByRarity()
{
	Items.Sort([](const FItemInstance& A, const FItemInstance& B)
	{
		UMMOItemBase* DefA = A.ItemDefinition.LoadSynchronous();
		UMMOItemBase* DefB = B.ItemDefinition.LoadSynchronous();
		if (!DefA || !DefB)
		{
			return DefA != nullptr;
		}
		return DefA->Rarity > DefB->Rarity;
	});
	OnContainerUpdated.Broadcast();
}

void UMMOItemContainer::SortByType()
{
	Items.Sort([](const FItemInstance& A, const FItemInstance& B)
	{
		UMMOItemBase* DefA = A.ItemDefinition.LoadSynchronous();
		UMMOItemBase* DefB = B.ItemDefinition.LoadSynchronous();
		if (!DefA || !DefB)
		{
			return DefA != nullptr;
		}
		return DefA->ItemType < DefB->ItemType;
	});
	OnContainerUpdated.Broadcast();
}

int32 UMMOItemContainer::FindOrCreateStack(const FItemInstance& Item)
{
	UMMOItemBase* ItemDef = Item.ItemDefinition.LoadSynchronous();
	if (!ItemDef)
	{
		return INDEX_NONE;
	}

	for (int32 i = 0; i < Items.Num(); i++)
	{
		UMMOItemBase* ExistingDef = Items[i].ItemDefinition.LoadSynchronous();
		if (ExistingDef && ExistingDef->ItemID == ItemDef->ItemID && Items[i].Quantity < ExistingDef->MaxStackSize)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

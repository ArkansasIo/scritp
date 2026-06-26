#include "Items/MMOItemBase.h"
#include "Engine/Texture2D.h"

UMMOItemBase::UMMOItemBase()
	: ItemID(NAME_None)
	, ItemType(EMMOItemType::Material)
	, Rarity(EMMOItemRarity::Common)
	, RequiredLevel(1)
	, StackSize(1)
	, MaxStackSize(99)
	, BuyPrice(0)
	, SellPrice(0)
	, Weight(0.1f)
{
}

bool UMMOItemBase::CanUse_Implementation(APlayerController* User)
{
	if (!User)
	{
		return false;
	}

	return true;
}

bool UMMOItemBase::OnUse_Implementation(APlayerController* User)
{
	return false;
}

bool UMMOItemBase::CanStack(UMMOItemBase* OtherItem) const
{
	if (!OtherItem)
	{
		return false;
	}

	return ItemID == OtherItem->ItemID
		&& ItemType == OtherItem->ItemType
		&& Rarity == OtherItem->Rarity
		&& StackSize < MaxStackSize;
}

FLinearColor UMMOItemBase::GetQualityColor() const
{
	return GetRarityColor(Rarity);
}

FLinearColor UMMOItemBase::GetRarityColor(EMMOItemRarity InRarity)
{
	switch (InRarity)
	{
	case EMMOItemRarity::Common:
		return FLinearColor::White;
	case EMMOItemRarity::Uncommon:
		return FLinearColor(0.0f, 1.0f, 0.0f);
	case EMMOItemRarity::Rare:
		return FLinearColor(0.0f, 0.5f, 1.0f);
	case EMMOItemRarity::Epic:
		return FLinearColor(0.6f, 0.2f, 0.8f);
	case EMMOItemRarity::Legendary:
		return FLinearColor(1.0f, 0.6f, 0.0f);
	case EMMOItemRarity::Mythic:
		return FLinearColor(1.0f, 0.2f, 0.2f);
	default:
		return FLinearColor::White;
	}
}

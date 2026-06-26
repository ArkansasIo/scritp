#include "Items/MMOEquipmentItem.h"
#include "Engine/World.h"
#include "Engine/SkeletalMesh.h"
#include "UI/MMOInventoryInterface.h"

UMMOEquipmentItem::UMMOEquipmentItem()
	: EquipmentSlot(EMMOEquipmentSlot::MainHand)
	, ArmorValue(0.0f)
	, DamageValue(0.0f)
	, Durability(100.0f)
	, MaxDurability(100.0f)
	, SocketCount(0)
	, bCanBeEnchanted(false)
	, EnchantmentSlots(0)
{
	ItemType = EMMOItemType::Equipment;
	MaxStackSize = 1;
	Weight = 1.0f;
}

float UMMOEquipmentItem::GetStatValue(FName StatName) const
{
	if (const float* Primary = PrimaryStats.Find(StatName))
	{
		return *Primary;
	}
	if (const float* Secondary = SecondaryStats.Find(StatName))
	{
		return *Secondary;
	}
	return 0.0f;
}

TMap<FName, float> UMMOEquipmentItem::GetAllStats() const
{
	TMap<FName, float> AllStats = PrimaryStats;
	for (const TPair<FName, float>& Pair : SecondaryStats)
	{
		if (AllStats.Contains(Pair.Key))
		{
			AllStats[Pair.Key] += Pair.Value;
		}
		else
		{
			AllStats.Add(Pair.Key, Pair.Value);
		}
	}
	return AllStats;
}

float UMMOEquipmentItem::GetDamageReduction() const
{
	return ArmorValue * 0.01f;
}

bool UMMOEquipmentItem::CanUse_Implementation(APlayerController* User)
{
	if (!User)
	{
		return false;
	}

	if (RequiredLevel > 0)
	{
		if (APawn* Pawn = User->GetPawn())
		{
			IMMOInventoryInterface* InvInterface = Cast<IMMOInventoryInterface>(Pawn);
			if (!InvInterface)
			{
				return false;
			}
		}
	}

	return true;
}

bool UMMOEquipmentItem::OnUse_Implementation(APlayerController* User)
{
	if (!CanUse(User))
	{
		return false;
	}

	return true;
}

void UMMOEquipmentItem::DamageItem(float DamageAmount)
{
	Durability = FMath::Clamp(Durability - DamageAmount, 0.0f, MaxDurability);
}

void UMMOEquipmentItem::RepairItem(float RepairAmount)
{
	Durability = FMath::Clamp(Durability + RepairAmount, 0.0f, MaxDurability);
}

bool UMMOEquipmentItem::NeedsRepair() const
{
	return Durability < MaxDurability;
}

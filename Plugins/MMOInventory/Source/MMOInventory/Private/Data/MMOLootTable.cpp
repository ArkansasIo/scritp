#include "Data/MMOLootTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UMMOLootTable::UMMOLootTable()
	: LootCategory(EMMOLootCategory::Monster)
	, MaxDropsPerRoll(5)
	, MinDropsPerRoll(1)
	, LuckModifier(1.0f)
{
}

TArray<FItemInstance> UMMOLootTable::RollLoot(AActor* Context)
{
	TArray<FItemInstance> AllLoot;

	TArray<FItemInstance> Guaranteed = RollGuaranteedLoot();
	AllLoot.Append(Guaranteed);

	TArray<FItemInstance> Chanced = RollChanceLoot(Context);
	AllLoot.Append(Chanced);

	if (AllLoot.Num() > MaxDropsPerRoll)
	{
		AllLoot.SetNum(MaxDropsPerRoll);
	}

	return AllLoot;
}

TArray<FItemInstance> UMMOLootTable::RollGuaranteedLoot()
{
	TArray<FItemInstance> Results;

	for (const FCuratedLootEntry& Curated : CuratedLoot)
	{
		if (Curated.bAlwaysDrop && !Curated.Item.IsNull())
		{
			FItemInstance Instance;
			Instance.ItemDefinition = Curated.Item;
			Instance.Quantity = Curated.Quantity > 0 ? Curated.Quantity : 1;
			Results.Add(Instance);
		}
	}

	for (const FLootDropEntry& Entry : GuaranteedDrops)
	{
		if (!Entry.Item.IsNull())
		{
			FItemInstance Instance;
			Instance.ItemDefinition = Entry.Item;
			Instance.Quantity = FMath::RandRange(Entry.MinQuantity, Entry.MaxQuantity);
			Results.Add(Instance);
		}
	}

	return Results;
}

TArray<FItemInstance> UMMOLootTable::RollChanceLoot(AActor* Context)
{
	TArray<FItemInstance> Results;

	for (const FLootDropEntry& Entry : DropEntries)
	{
		if (Entry.Item.IsNull())
		{
			continue;
		}

		if (Entry.bIsGuaranteed)
		{
			FItemInstance Instance;
			Instance.ItemDefinition = Entry.Item;
			Instance.Quantity = FMath::RandRange(Entry.MinQuantity, Entry.MaxQuantity);
			Results.Add(Instance);
			continue;
		}

		float EffectiveChance = Entry.DropChance * LuckModifier;
		if (RollChance(EffectiveChance))
		{
			FItemInstance Instance;
			Instance.ItemDefinition = Entry.Item;
			Instance.Quantity = FMath::RandRange(Entry.MinQuantity, Entry.MaxQuantity);
			Results.Add(Instance);
		}
	}

	return Results;
}

bool UMMOLootTable::RollChance(float Chance) const
{
	if (Chance >= 100.0f)
	{
		return true;
	}
	if (Chance <= 0.0f)
	{
		return false;
	}

	float Roll = FMath::FRandRange(0.0f, 100.0f);
	return Roll <= Chance;
}

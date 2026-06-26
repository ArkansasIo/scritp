#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Items/MMOItemBase.h"
#include "MMOLootTable.generated.h"

UENUM(BlueprintType)
enum class EMMOLootCategory : uint8
{
	Monster		UMETA(DisplayName = "Monster"),
	Chest		UMETA(DisplayName = "Chest"),
	QuestReward	UMETA(DisplayName = "Quest Reward"),
	Gathering	UMETA(DisplayName = "Gathering"),
	Boss		UMETA(DisplayName = "Boss")
};

USTRUCT(BlueprintType)
struct MMOINVENTORY_API FLootDropEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	TSoftObjectPtr<UMMOItemBase> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	float DropChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MinQuantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MaxQuantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	TArray<FGameplayTag> RequiredConditions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	TArray<FName> RequiredQuestFlags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	bool bIsGuaranteed;

	FLootDropEntry()
		: DropChance(0.0f)
		, MinQuantity(1)
		, MaxQuantity(1)
		, bIsGuaranteed(false)
	{
	}
};

USTRUCT(BlueprintType)
struct MMOINVENTORY_API FCuratedLootEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curated")
	TSoftObjectPtr<UMMOItemBase> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curated")
	bool bAlwaysDrop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curated")
	int32 Quantity;
};

UCLASS(BlueprintType)
class MMOINVENTORY_API UMMOLootTable : public UDataAsset
{
	GENERATED_BODY()

public:
	UMMOLootTable();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	EMMOLootCategory LootCategory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TArray<FLootDropEntry> DropEntries;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TArray<FLootDropEntry> GuaranteedDrops;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TArray<FCuratedLootEntry> CuratedLoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	int32 MaxDropsPerRoll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	int32 MinDropsPerRoll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	float LuckModifier;

	UFUNCTION(BlueprintCallable, Category = "Loot")
	TArray<FItemInstance> RollLoot(AActor* Context = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Loot")
	TArray<FItemInstance> RollGuaranteedLoot();

	UFUNCTION(BlueprintCallable, Category = "Loot")
	TArray<FItemInstance> RollChanceLoot(AActor* Context = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Loot")
	bool RollChance(float Chance) const;
};

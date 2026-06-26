#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "MMOItemBase.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EMMOItemType : uint8
{
	Consumable	UMETA(DisplayName = "Consumable"),
	Equipment	UMETA(DisplayName = "Equipment"),
	QuestItem	UMETA(DisplayName = "Quest Item"),
	Material	UMETA(DisplayName = "Material"),
	Currency	UMETA(DisplayName = "Currency"),
	Key			UMETA(DisplayName = "Key"),
	Recipe		UMETA(DisplayName = "Recipe")
};

UENUM(BlueprintType)
enum class EMMOItemRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Epic		UMETA(DisplayName = "Epic"),
	Legendary	UMETA(DisplayName = "Legendary"),
	Mythic		UMETA(DisplayName = "Mythic")
};

UENUM(BlueprintType)
enum class EMMOEquipmentSlot : uint8
{
	Head		UMETA(DisplayName = "Head"),
	Chest		UMETA(DisplayName = "Chest"),
	Legs		UMETA(DisplayName = "Legs"),
	Feet		UMETA(DisplayName = "Feet"),
	Hands		UMETA(DisplayName = "Hands"),
	MainHand	UMETA(DisplayName = "Main Hand"),
	OffHand		UMETA(DisplayName = "Off Hand"),
	TwoHand		UMETA(DisplayName = "Two Hand"),
	Ring		UMETA(DisplayName = "Ring"),
	Necklace	UMETA(DisplayName = "Necklace"),
	Trinket		UMETA(DisplayName = "Trinket"),
	Shoulders	UMETA(DisplayName = "Shoulders"),
	Waist		UMETA(DisplayName = "Waist"),
	Back		UMETA(DisplayName = "Back")
};

UENUM(BlueprintType)
enum class EMMOContainerType : uint8
{
	Inventory	UMETA(DisplayName = "Inventory"),
	Bank		UMETA(DisplayName = "Bank"),
	Bag			UMETA(DisplayName = "Bag"),
	Vendor		UMETA(DisplayName = "Vendor"),
	Loot		UMETA(DisplayName = "Loot"),
	Mail		UMETA(DisplayName = "Mail")
};

UENUM(BlueprintType)
enum class EMMOConsumableEffect : uint8
{
	Heal				UMETA(DisplayName = "Heal"),
	Mana				UMETA(DisplayName = "Mana"),
	Buff				UMETA(DisplayName = "Buff"),
	RemoveDebuff		UMETA(DisplayName = "Remove Debuff"),
	TemporaryStatBoost	UMETA(DisplayName = "Temporary Stat Boost"),
	Teleport			UMETA(DisplayName = "Teleport")
};

UENUM(BlueprintType)
enum class EMMOConsumableTarget : uint8
{
	Self			UMETA(DisplayName = "Self"),
	SingleTarget	UMETA(DisplayName = "Single Target"),
	AoE				UMETA(DisplayName = "Area of Effect")
};

USTRUCT(BlueprintType)
struct MMOINVENTORY_API FItemStatModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FName StatName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float PercentBonus;
};

USTRUCT(BlueprintType)
struct MMOINVENTORY_API FItemInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<class UMMOItemBase> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Quantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float CurrentDurability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 EnchantmentLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FName> SocketedGems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName EnchantmentId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGuid UniqueId;

	FItemInstance()
		: Quantity(0)
		, CurrentDurability(-1.0f)
		, EnchantmentLevel(0)
		, UniqueId(FGuid::NewGuid())
	{
	}

	bool IsValid() const
	{
		return ItemDefinition.IsValid() || !ItemDefinition.IsNull();
	}

	void Clear()
	{
		ItemDefinition = nullptr;
		Quantity = 0;
		CurrentDurability = -1.0f;
		EnchantmentLevel = 0;
		SocketedGems.Empty();
		EnchantmentId = NAME_None;
		UniqueId = FGuid::NewGuid();
	}
};

USTRUCT(BlueprintType)
struct MMOINVENTORY_API FMMODropEntry : public FTableRowBase
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
	bool bIsGuaranteed;

	FMMODropEntry()
		: DropChance(0.0f)
		, MinQuantity(1)
		, MaxQuantity(1)
		, bIsGuaranteed(false)
	{
	}
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, Abstract)
class MMOINVENTORY_API UMMOItemBase : public UObject
{
	GENERATED_BODY()

public:
	UMMOItemBase();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Identity")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Identity")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Identity", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Identity", meta = (MultiLine = true))
	FText FlavorText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Display")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Type")
	EMMOItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Rarity")
	EMMOItemRarity Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Requirements")
	int32 RequiredLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Requirements")
	FName RequiredClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Requirements")
	FName RequiredRace;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stacking")
	int32 StackSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stacking")
	int32 MaxStackSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Value")
	int32 BuyPrice;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Value")
	int32 SellPrice;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Value")
	float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Tags")
	FGameplayTagContainer GameplayTags;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	bool CanUse(class APlayerController* User);
	virtual bool CanUse_Implementation(class APlayerController* User);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	bool OnUse(class APlayerController* User);
	virtual bool OnUse_Implementation(class APlayerController* User);

	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual bool CanStack(UMMOItemBase* OtherItem) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	FLinearColor GetQualityColor() const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	static FLinearColor GetRarityColor(EMMOItemRarity InRarity);
};

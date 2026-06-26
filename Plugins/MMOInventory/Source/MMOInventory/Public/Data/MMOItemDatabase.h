#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Items/MMOItemBase.h"
#include "Engine/DataTable.h"
#include "MMOItemDatabase.generated.h"

class UDataTable;

USTRUCT(BlueprintType)
struct MMOINVENTORY_API FMMOItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftClassPtr<UMMOItemBase> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EMMOItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EMMOItemRarity Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 RequiredLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName RequiredClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGameplayTagContainer GameplayTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<UMMOItemBase> ItemAsset;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDatabaseReady, class UMMOItemDatabase*, Database);

UCLASS(BlueprintType)
class MMOINVENTORY_API UMMOItemDatabase : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Database")
	FOnItemDatabaseReady OnDatabaseReady;

	UFUNCTION(BlueprintCallable, Category = "Database")
	void LoadDatabase(UDataTable* ItemDataTable);

	UFUNCTION(BlueprintCallable, Category = "Database")
	UMMOItemBase* GetItemByID(FName ItemID);

	UFUNCTION(BlueprintCallable, Category = "Database")
	TArray<UMMOItemBase*> GetItemsByType(EMMOItemType ItemType);

	UFUNCTION(BlueprintCallable, Category = "Database")
	TArray<UMMOItemBase*> GetItemsByLevelRange(int32 MinLevel, int32 MaxLevel);

	UFUNCTION(BlueprintCallable, Category = "Database")
	TArray<UMMOItemBase*> GetItemsByClass(FName ClassName);

	UFUNCTION(BlueprintCallable, Category = "Database")
	TArray<UMMOItemBase*> GetItemsByTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Database")
	TArray<UMMOItemBase*> SearchItems(const FString& SearchTerm, EMMOItemType TypeFilter = EMMOItemType::Consumable, bool bFilterByType = false);

	UFUNCTION(BlueprintCallable, Category = "Database")
	void LoadItemAssets(TArray<FName> ItemIds);

	UFUNCTION(BlueprintCallable, Category = "Database")
	bool IsLoaded() const;

	UFUNCTION(BlueprintCallable, Category = "Database")
	int32 GetItemCount() const;

protected:
	UPROPERTY()
	TMap<FName, TSoftObjectPtr<UMMOItemBase>> ItemMap;

	UPROPERTY()
	TMap<FName, UMMOItemBase*> LoadedItems;

	UPROPERTY()
	UDataTable* ItemDataTable;

	bool bDatabaseLoaded;

	void BuildItemIndex();
	TSoftObjectPtr<UMMOItemBase> FindRowItem(FName ItemID) const;
};

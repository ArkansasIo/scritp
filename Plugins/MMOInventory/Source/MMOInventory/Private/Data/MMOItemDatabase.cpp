#include "Data/MMOItemDatabase.h"
#include "Engine/DataTable.h"

void UMMOItemDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bDatabaseLoaded = false;
}

void UMMOItemDatabase::Deinitialize()
{
	LoadedItems.Empty();
	ItemMap.Empty();
	Super::Deinitialize();
}

void UMMOItemDatabase::LoadDatabase(UDataTable* InItemDataTable)
{
	if (!InItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("MMOItemDatabase: No DataTable provided."));
		return;
	}

	ItemDataTable = InItemDataTable;
	LoadedItems.Empty();
	ItemMap.Empty();

	TArray<FMMOItemTableRow*> Rows;
	ItemDataTable->GetAllRows<FMMOItemTableRow>(TEXT("MMOItemDatabase"), Rows);

	for (const FMMOItemTableRow* Row : Rows)
	{
		if (Row && !Row->ItemID.IsNone())
		{
			ItemMap.Add(Row->ItemID, Row->ItemAsset);
		}
	}

	bDatabaseLoaded = true;
	UE_LOG(LogTemp, Log, TEXT("MMOItemDatabase: Loaded %d item entries."), ItemMap.Num());
	OnDatabaseReady.Broadcast(this);
}

UMMOItemBase* UMMOItemDatabase::GetItemByID(FName ItemID)
{
	if (ItemID.IsNone())
	{
		return nullptr;
	}

	if (UMMOItemBase** Found = LoadedItems.Find(ItemID))
	{
		return *Found;
	}

	TSoftObjectPtr<UMMOItemBase>* SoftRef = ItemMap.Find(ItemID);
	if (SoftRef && !SoftRef->IsNull())
	{
		UMMOItemBase* LoadedItem = SoftRef->LoadSynchronous();
		if (LoadedItem)
		{
			LoadedItems.Add(ItemID, LoadedItem);
			return LoadedItem;
		}
	}

	return nullptr;
}

TArray<UMMOItemBase*> UMMOItemDatabase::GetItemsByType(EMMOItemType ItemType)
{
	TArray<UMMOItemBase*> Results;
	for (const TPair<FName, TSoftObjectPtr<UMMOItemBase>>& Pair : ItemMap)
	{
		UMMOItemBase* Item = GetItemByID(Pair.Key);
		if (Item && Item->ItemType == ItemType)
		{
			Results.Add(Item);
		}
	}
	return Results;
}

TArray<UMMOItemBase*> UMMOItemDatabase::GetItemsByLevelRange(int32 MinLevel, int32 MaxLevel)
{
	TArray<UMMOItemBase*> Results;
	for (const TPair<FName, TSoftObjectPtr<UMMOItemBase>>& Pair : ItemMap)
	{
		UMMOItemBase* Item = GetItemByID(Pair.Key);
		if (Item && Item->RequiredLevel >= MinLevel && Item->RequiredLevel <= MaxLevel)
		{
			Results.Add(Item);
		}
	}
	return Results;
}

TArray<UMMOItemBase*> UMMOItemDatabase::GetItemsByClass(FName ClassName)
{
	TArray<UMMOItemBase*> Results;
	for (const TPair<FName, TSoftObjectPtr<UMMOItemBase>>& Pair : ItemMap)
	{
		UMMOItemBase* Item = GetItemByID(Pair.Key);
		if (Item && Item->RequiredClass == ClassName)
		{
			Results.Add(Item);
		}
	}
	return Results;
}

TArray<UMMOItemBase*> UMMOItemDatabase::GetItemsByTag(FGameplayTag Tag)
{
	TArray<UMMOItemBase*> Results;
	for (const TPair<FName, TSoftObjectPtr<UMMOItemBase>>& Pair : ItemMap)
	{
		UMMOItemBase* Item = GetItemByID(Pair.Key);
		if (Item && Item->GameplayTags.HasTag(Tag))
		{
			Results.Add(Item);
		}
	}
	return Results;
}

TArray<UMMOItemBase*> UMMOItemDatabase::SearchItems(const FString& SearchTerm, EMMOItemType TypeFilter, bool bFilterByType)
{
	TArray<UMMOItemBase*> Results;
	FString LowerSearch = SearchTerm.ToLower();

	for (const TPair<FName, TSoftObjectPtr<UMMOItemBase>>& Pair : ItemMap)
	{
		UMMOItemBase* Item = GetItemByID(Pair.Key);
		if (!Item)
		{
			continue;
		}

		if (bFilterByType && Item->ItemType != TypeFilter)
		{
			continue;
		}

		FString ItemName = Item->ItemName.ToString().ToLower();
		FString ItemDesc = Item->Description.ToString().ToLower();
		FString ItemID = Item->ItemID.ToString().ToLower();

		if (ItemName.Contains(LowerSearch) || ItemDesc.Contains(LowerSearch) || ItemID.Contains(LowerSearch))
		{
			Results.Add(Item);
		}
	}

	return Results;
}

void UMMOItemDatabase::LoadItemAssets(TArray<FName> ItemIds)
{
	for (const FName& ID : ItemIds)
	{
		GetItemByID(ID);
	}
}

bool UMMOItemDatabase::IsLoaded() const
{
	return bDatabaseLoaded;
}

int32 UMMOItemDatabase::GetItemCount() const
{
	return ItemMap.Num();
}

void UMMOItemDatabase::BuildItemIndex()
{
}

TSoftObjectPtr<UMMOItemBase> UMMOItemDatabase::FindRowItem(FName ItemID) const
{
	const TSoftObjectPtr<UMMOItemBase>* Found = ItemMap.Find(ItemID);
	if (Found)
	{
		return *Found;
	}
	return nullptr;
}

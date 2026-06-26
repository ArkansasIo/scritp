#include "Engine/MMOAssetManager.h"
#include "Data/MMOGlobalData.h"
#include "Engine/DataTable.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManagerSettings.h"

UMMOAssetManager& UMMOAssetManager::Get()
{
	UMMOAssetManager* Manager = Cast<UMMOAssetManager>(GEngine->AssetManager);
	if (Manager)
	{
		return *Manager;
	}

	UE_LOG(LogMMOCore, Fatal, TEXT("Invalid AssetManager in GEngine - must be UMMOAssetManager"));
	return *NewObject<UMMOAssetManager>();
}

UMMOAssetManager* UMMOAssetManager::GetAssetManager()
{
	return Cast<UMMOAssetManager>(GEngine->AssetManager);
}

void UMMOAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	UE_LOG(LogMMOCore, Log, TEXT("MMOAssetManager starting initial loading"));

	FPrimaryAssetTypeInfo ItemTypeInfo = FPrimaryAssetTypeInfo(
		TEXT("MMOItem"),
		TEXT(""),
		TEXT(""),
		TArray<FDirectoryPath>(),
		TArray<FString>()
	);
	ItemTypeInfo.bIsDynamicAsset = true;
	RegisterPrimaryAssetType(ItemTypeInfo);

	FPrimaryAssetTypeInfo AbilityTypeInfo = FPrimaryAssetTypeInfo(
		TEXT("MMOAbility"),
		TEXT(""),
		TEXT(""),
		TArray<FDirectoryPath>(),
		TArray<FString>()
	);
	AbilityTypeInfo.bIsDynamicAsset = true;
	RegisterPrimaryAssetType(AbilityTypeInfo);

	FPrimaryAssetTypeInfo QuestTypeInfo = FPrimaryAssetTypeInfo(
		TEXT("MMOQuest"),
		TEXT(""),
		TEXT(""),
		TArray<FDirectoryPath>(),
		TArray<FString>()
	);
	QuestTypeInfo.bIsDynamicAsset = true;
	RegisterPrimaryAssetType(QuestTypeInfo);

	FPrimaryAssetTypeInfo GlobalDataTypeInfo = FPrimaryAssetTypeInfo(
		TEXT("MMOGlobalData"),
		TEXT(""),
		TEXT(""),
		TArray<FDirectoryPath>(),
		TArray<FString>()
	);
	GlobalDataTypeInfo.bIsDynamicAsset = true;
	RegisterPrimaryAssetType(GlobalDataTypeInfo);
}

void UMMOAssetManager::PostInitialLoadComplete()
{
	Super::PostInitialLoadComplete();

	UE_LOG(LogMMOCore, Log, TEXT("MMOAssetManager post initial load complete"));

	LoadGlobalData();
	bDataLoaded = true;
}

void UMMOAssetManager::LoadItemDatabase()
{
	UE_LOG(LogMMOCore, Log, TEXT("Loading item database"));
}

void UMMOAssetManager::UnloadItemDatabase()
{
	UE_LOG(LogMMOCore, Log, TEXT("Unloading item database"));
	ItemDataTables.Empty();
}

void UMMOAssetManager::LoadAbilityDatabase()
{
	UE_LOG(LogMMOCore, Log, TEXT("Loading ability database"));
}

void UMMOAssetManager::UnloadAbilityDatabase()
{
	UE_LOG(LogMMOCore, Log, TEXT("Unloading ability database"));
	AbilityDataTables.Empty();
}

void UMMOAssetManager::LoadQuestDatabase()
{
	UE_LOG(LogMMOCore, Log, TEXT("Loading quest database"));
}

void UMMOAssetManager::UnloadQuestDatabase()
{
	UE_LOG(LogMMOCore, Log, TEXT("Unloading quest database"));
	QuestDataTables.Empty();
}

void UMMOAssetManager::LoadGlobalData()
{
	FPrimaryAssetId GlobalDataId = FPrimaryAssetId(TEXT("MMOGlobalData"), TEXT("GlobalData"));

	TArray<FName> Bundles;
	Bundles.Add(TEXT("Data"));

	FStreamableManager& Streamable = StreamableManager;
	FSoftObjectPath GlobalDataPath = GlobalDataPtr.ToSoftObjectPath();

	if (GlobalDataPath.IsValid())
	{
		GlobalDataCache = Cast<UMMOGlobalData>(Streamable.LoadSynchronous(GlobalDataPath));
		if (GlobalDataCache)
		{
			UE_LOG(LogMMOCore, Log, TEXT("Global data loaded successfully"));
			GlobalDataCache->OnGlobalDataLoaded.Broadcast();
		}
	}
	else
	{
		UE_LOG(LogMMOCore, Warning, TEXT("Global data path not set in asset manager"));
	}
}

void UMMOAssetManager::UnloadGlobalData()
{
	if (GlobalDataCache)
	{
		GlobalDataCache = nullptr;
		UE_LOG(LogMMOCore, Log, TEXT("Global data unloaded"));
	}
}

UMMOGlobalData* UMMOAssetManager::GetGlobalData() const
{
	return GlobalDataCache;
}

void UMMOAssetManager::LoadAssetBundle(FName BundleName)
{
	FMMOAssetBundle* Bundle = RegisteredBundles.Find(BundleName);
	if (!Bundle)
	{
		UE_LOG(LogMMOCore, Warning, TEXT("Asset bundle %s not found"), *BundleName.ToString());
		return;
	}

	TArray<FSoftObjectPath> PathsToLoad;
	for (const FPrimaryAssetId& AssetId : Bundle->AssetIDs)
	{
		FSoftObjectPath AssetPath = GetPrimaryAssetPath(AssetId);
		if (AssetPath.IsValid())
		{
			PathsToLoad.Add(AssetPath);
		}
	}

	if (PathsToLoad.Num() > 0)
	{
		FStreamableManager& Streamable = StreamableManager;
		Streamable.RequestAsyncLoad(PathsToLoad, FStreamableDelegate());
	}
}

void UMMOAssetManager::UnloadAssetBundle(FName BundleName)
{
	FMMOAssetBundle* Bundle = RegisteredBundles.Find(BundleName);
	if (!Bundle)
	{
		UE_LOG(LogMMOCore, Warning, TEXT("Asset bundle %s not found"), *BundleName.ToString());
		return;
	}

	for (const FPrimaryAssetId& AssetId : Bundle->AssetIDs)
	{
		UnloadPrimaryAsset(AssetId);
	}
}

bool UMMOAssetManager::RegisterAssetBundle(const FMMOAssetBundle& Bundle)
{
	if (RegisteredBundles.Contains(Bundle.BundleName))
	{
		UE_LOG(LogMMOCore, Warning, TEXT("Asset bundle %s already registered"), *Bundle.BundleName.ToString());
		return false;
	}

	RegisteredBundles.Add(Bundle.BundleName, Bundle);
	return true;
}

void UMMOAssetManager::LoadAllGameData()
{
	LoadGlobalData();
	LoadItemDatabase();
	LoadAbilityDatabase();
	LoadQuestDatabase();
	bDataLoaded = true;
}

void UMMOAssetManager::AsyncLoadPrimaryAsset(FPrimaryAssetId AssetId, const TArray<FName>& LoadBundles, FOnAssetsLoaded OnLoaded)
{
	TArray<FPrimaryAssetId> AssetIds;
	AssetIds.Add(AssetId);
	AsyncLoadPrimaryAssets(AssetIds, LoadBundles, OnLoaded);
}

void UMMOAssetManager::AsyncLoadPrimaryAssets(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, FOnAssetsLoaded OnLoaded)
{
	if (AssetIds.Num() == 0)
	{
		OnLoaded.ExecuteIfBound(AssetIds);
		return;
	}

	FStreamableManager& Streamable = StreamableManager;
	TArray<FSoftObjectPath> PathsToLoad;

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		FSoftObjectPath AssetPath = GetPrimaryAssetPath(AssetId);
		if (AssetPath.IsValid())
		{
			PathsToLoad.Add(AssetPath);
		}
	}

	if (PathsToLoad.Num() > 0)
	{
		Streamable.RequestAsyncLoad(PathsToLoad,
			FStreamableDelegate::CreateLambda([this, AssetIds, OnLoaded]()
			{
				OnLoaded.ExecuteIfBound(AssetIds);
			})
		);
	}
	else
	{
		OnLoaded.ExecuteIfBound(AssetIds);
	}
}

void UMMOAssetManager::RegisterPrimaryAssetType(FPrimaryAssetTypeInfo TypeInfo)
{
	TArray<FPrimaryAssetTypeInfo> ExistingTypes;
	if (GetAssetTypeInfo(TypeInfo.PrimaryAssetType, ExistingTypes))
	{
		UE_LOG(LogMMOCore, Verbose, TEXT("Asset type %s already registered"), *TypeInfo.PrimaryAssetType.ToString());
		return;
	}

	UAssetManagerSettings& Settings = const_cast<UAssetManagerSettings&>(GetSettings());
	Settings.PrimaryAssetTypesToScan.Add(TypeInfo);
	Settings.PostEditChange();
}

UObject* UMMOAssetManager::GetCachedAssetData(const FPrimaryAssetId& AssetId)
{
	return GetPrimaryAssetObject(AssetId);
}

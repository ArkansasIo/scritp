#pragma once

#include "MMOCore.h"
#include "Engine/AssetManager.h"
#include "Engine/AssetManagerTypes.h"
#include "MMOAssetManager.generated.h"

class UMMOGlobalData;
class UDataTable;

USTRUCT(BlueprintType)
struct FMMOAssetBundle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Bundle")
	FName BundleName;

	UPROPERTY(BlueprintReadOnly, Category = "Bundle")
	TArray<FPrimaryAssetId> AssetIDs;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssetsLoaded, const TArray<FPrimaryAssetId>&, LoadedAssets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssetLoadFailed, const FString&, ErrorMessage);

UCLASS(Blueprintable, BlueprintType, config = Game)
class MMCORE_API UMMOAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UMMOAssetManager& Get();

	template<typename T>
	T* GetCachedDataAsset(const FPrimaryAssetId& AssetId)
	{
		return Cast<T>(GetCachedAssetData(AssetId));
	}

	UFUNCTION(BlueprintPure, Category = "MMO|AssetManager")
	static UMMOAssetManager* GetAssetManager();

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	void LoadItemDatabase();

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	void UnloadItemDatabase();

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	void LoadAbilityDatabase();

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	void UnloadAbilityDatabase();

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	void LoadQuestDatabase();

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	void UnloadQuestDatabase();

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	void LoadGlobalData();

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	void UnloadGlobalData();

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	UMMOGlobalData* GetGlobalData() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	void LoadAssetBundle(FName BundleName);

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	void UnloadAssetBundle(FName BundleName);

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	bool RegisterAssetBundle(const FMMOAssetBundle& Bundle);

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager")
	void LoadAllGameData();

	UFUNCTION(BlueprintPure, Category = "MMO|AssetManager")
	bool IsDataLoaded() const { return bDataLoaded; }

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager", meta = (AdvancedDisplay = "3"))
	void AsyncLoadPrimaryAsset(FPrimaryAssetId AssetId, const TArray<FName>& LoadBundles, FOnAssetsLoaded OnLoaded);

	UFUNCTION(BlueprintCallable, Category = "MMO|AssetManager", meta = (AdvancedDisplay = "3"))
	void AsyncLoadPrimaryAssets(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, FOnAssetsLoaded OnLoaded);

	void RegisterPrimaryAssetType(FPrimaryAssetTypeInfo TypeInfo);

	template<typename T>
	T* LoadSynchronousPrimaryAsset(FPrimaryAssetId AssetId, const TArray<FName>& LoadBundles = TArray<FName>())
	{
		return Cast<T>(LoadPrimaryAsset(AssetId, LoadBundles));
	}

protected:
	virtual void StartInitialLoading() override;
	virtual void PostInitialLoadComplete() override;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|AssetManager")
	TMap<FName, FMMOAssetBundle> RegisteredBundles;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|AssetManager")
	TMap<FName, TSoftObjectPtr<UDataTable>> ItemDataTables;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|AssetManager")
	TMap<FName, TSoftObjectPtr<UDataTable>> AbilityDataTables;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|AssetManager")
	TMap<FName, TSoftObjectPtr<UDataTable>> QuestDataTables;

	UPROPERTY()
	TSoftObjectPtr<UMMOGlobalData> GlobalDataPtr;

	UPROPERTY()
	UMMOGlobalData* GlobalDataCache;

	bool bDataLoaded = false;

	UObject* GetCachedAssetData(const FPrimaryAssetId& AssetId);
};

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Quests/MMOQuestBase.h"
#include "MMOQuestDatabase.generated.h"

USTRUCT(BlueprintType)
struct FMMOQuestTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TSoftObjectPtr<UMMOQuestBase> Quest;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestDatabaseReady, bool, bSuccess);

UCLASS()
class MMOQUEST_API UMMOQuestDatabase : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Quest Database")
	FOnQuestDatabaseReady OnDatabaseReady;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Database")
	UMMOQuestBase* GetQuestByID(FGameplayTag QuestID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Database")
	TArray<UMMOQuestBase*> GetQuestsByLevelRange(int32 MinLevel, int32 MaxLevel) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Database")
	TArray<UMMOQuestBase*> GetQuestsByType(EQuestType Type) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Database")
	TArray<UMMOQuestBase*> GetQuestsByGiverNPC(FGameplayTag NPCID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Database")
	TArray<UMMOQuestBase*> GetQuestsByTurnInNPC(FGameplayTag NPCID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Database")
	TArray<UMMOQuestBase*> GetAllQuests() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Database")
	bool CheckQuestPrerequisites(FGameplayTag QuestID, const class UMMOQuestComponent* Component) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Database")
	bool IsQuestAvailableForPlayer(FGameplayTag QuestID, const class UMMOQuestComponent* Component) const;

	UFUNCTION(BlueprintCallable, Category = "Quest Database")
	void ReloadDatabase();

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Database")
	TArray<UMMOQuestBase*> GetQuestsByTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Database")
	TArray<UMMOQuestBase*> GetQuestsByTags(const FGameplayTagContainer& Tags, bool bRequireAll = false) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest Database")
	int32 GetQuestCount() const;

protected:
	void LoadFromPrimaryDataAssets();
	void LoadFromDataTable();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	TArray<TSoftObjectPtr<UMMOQuestBase>> QuestAssetDirectories;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	TSoftObjectPtr<UDataTable> QuestDataTable;

	UPROPERTY(BlueprintReadOnly, Category = "Quest Database")
	TMap<FGameplayTag, UMMOQuestBase*> QuestMap;

	UPROPERTY(BlueprintReadOnly, Category = "Quest Database")
	TArray<UMMOQuestBase*> QuestList;

	bool bDatabaseReady = false;
};

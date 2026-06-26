#include "Data/MMOQuestDatabase.h"
#include "Quests/MMOQuestComponent.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"

void UMMOQuestDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("MMOQuestDatabase initializing..."));
	LoadFromPrimaryDataAssets();
	LoadFromDataTable();
	bDatabaseReady = true;
	OnDatabaseReady.Broadcast(true);
}

void UMMOQuestDatabase::Deinitialize()
{
	QuestMap.Empty();
	QuestList.Empty();
	bDatabaseReady = false;
	Super::Deinitialize();
}

void UMMOQuestDatabase::LoadFromPrimaryDataAssets()
{
	for (const TSoftObjectPtr<UMMOQuestBase>& AssetPtr : QuestAssetDirectories)
	{
		if (AssetPtr.IsPending())
		{
			AssetPtr.LoadSynchronous();
		}
		if (UMMOQuestBase* Quest = AssetPtr.Get())
		{
			if (Quest->QuestID.IsValid())
			{
				QuestMap.Add(Quest->QuestID, Quest);
				QuestList.Add(Quest);
			}
		}
	}
}

void UMMOQuestDatabase::LoadFromDataTable()
{
	if (QuestDataTable.IsPending())
	{
		QuestDataTable.LoadSynchronous();
	}
	if (UDataTable* Table = QuestDataTable.Get())
	{
		static const FString ContextStr(TEXT("MMOQuestDatabase::LoadFromDataTable"));
		TArray<FMMOQuestTableRow*> Rows;
		Table->GetAllRows<FMMOQuestTableRow>(ContextStr, Rows);
		for (const FMMOQuestTableRow* Row : Rows)
		{
			if (Row)
			{
				UMMOQuestBase* Quest = Row->Quest.LoadSynchronous();
				if (Quest && Quest->QuestID.IsValid() && !QuestMap.Contains(Quest->QuestID))
				{
					QuestMap.Add(Quest->QuestID, Quest);
					QuestList.Add(Quest);
				}
			}
		}
	}
}

UMMOQuestBase* UMMOQuestDatabase::GetQuestByID(FGameplayTag QuestID) const
{
	const UMMOQuestBase* const* Found = QuestMap.Find(QuestID);
	return Found ? const_cast<UMMOQuestBase*>(*Found) : nullptr;
}

TArray<UMMOQuestBase*> UMMOQuestDatabase::GetQuestsByLevelRange(int32 MinLevel, int32 MaxLevel) const
{
	TArray<UMMOQuestBase*> Results;
	for (UMMOQuestBase* Quest : QuestList)
	{
		if (Quest)
		{
			if (Quest->QuestLevel >= MinLevel && (MaxLevel <= 0 || Quest->QuestLevel <= MaxLevel))
			{
				Results.Add(Quest);
			}
		}
	}
	return Results;
}

TArray<UMMOQuestBase*> UMMOQuestDatabase::GetQuestsByType(EQuestType Type) const
{
	TArray<UMMOQuestBase*> Results;
	for (UMMOQuestBase* Quest : QuestList)
	{
		if (Quest && Quest->QuestType == Type)
		{
			Results.Add(Quest);
		}
	}
	return Results;
}

TArray<UMMOQuestBase*> UMMOQuestDatabase::GetQuestsByGiverNPC(FGameplayTag NPCID) const
{
	TArray<UMMOQuestBase*> Results;
	for (UMMOQuestBase* Quest : QuestList)
	{
		if (Quest && Quest->QuestGiverNPC == NPCID)
		{
			Results.Add(Quest);
		}
	}
	return Results;
}

TArray<UMMOQuestBase*> UMMOQuestDatabase::GetQuestsByTurnInNPC(FGameplayTag NPCID) const
{
	TArray<UMMOQuestBase*> Results;
	for (UMMOQuestBase* Quest : QuestList)
	{
		if (Quest && Quest->TurnInNPC == NPCID)
		{
			Results.Add(Quest);
		}
	}
	return Results;
}

TArray<UMMOQuestBase*> UMMOQuestDatabase::GetAllQuests() const
{
	return QuestList;
}

bool UMMOQuestDatabase::CheckQuestPrerequisites(FGameplayTag QuestID, const UMMOQuestComponent* Component) const
{
	const UMMOQuestBase* Quest = GetQuestByID(QuestID);
	if (!Quest || !Component)
	{
		return false;
	}
	return Quest->MeetsPrerequisites(Component);
}

bool UMMOQuestDatabase::IsQuestAvailableForPlayer(FGameplayTag QuestID, const UMMOQuestComponent* Component) const
{
	if (!Component)
	{
		return false;
	}
	const UMMOQuestBase* Quest = GetQuestByID(QuestID);
	if (!Quest)
	{
		return false;
	}
	if (Component->HasActiveQuest(QuestID) || Component->HasCompletedQuest(QuestID))
	{
		return false;
	}
	if (!Quest->MeetsPrerequisites(Component))
	{
		return false;
	}
	if (Quest->MinLevel > 0)
	{
	}
	if (Quest->bRepeatable && !Quest->IsRepeatableNow(Component))
	{
		return false;
	}
	return true;
}

void UMMOQuestDatabase::ReloadDatabase()
{
	QuestMap.Empty();
	QuestList.Empty();
	LoadFromPrimaryDataAssets();
	LoadFromDataTable();
	OnDatabaseReady.Broadcast(true);
}

TArray<UMMOQuestBase*> UMMOQuestDatabase::GetQuestsByTag(FGameplayTag Tag) const
{
	TArray<UMMOQuestBase*> Results;
	for (UMMOQuestBase* Quest : QuestList)
	{
		if (Quest && Quest->QuestTags.HasTag(Tag))
		{
			Results.Add(Quest);
		}
	}
	return Results;
}

TArray<UMMOQuestBase*> UMMOQuestDatabase::GetQuestsByTags(const FGameplayTagContainer& Tags, bool bRequireAll) const
{
	TArray<UMMOQuestBase*> Results;
	for (UMMOQuestBase* Quest : QuestList)
	{
		if (Quest)
		{
			if (bRequireAll ? Quest->QuestTags.HasAll(Tags) : Quest->QuestTags.HasAny(Tags))
			{
				Results.Add(Quest);
			}
		}
	}
	return Results;
}

int32 UMMOQuestDatabase::GetQuestCount() const
{
	return QuestList.Num();
}

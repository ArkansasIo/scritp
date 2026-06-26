#include "World/MMOSpawnManagerSubsystem.h"
#include "MMOWorld.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"

void UMMOSpawnManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSpawnDefinitions();

	UE_LOG(LogMMOWorld, Log, TEXT("MMOSpawnManagerSubsystem initialized."));
}

void UMMOSpawnManagerSubsystem::Deinitialize()
{
	ActiveSpawns.Empty();
	SpawnDefinitions.Empty();
	SpawnPools.Empty();
	SpawnGroups.Empty();

	Super::Deinitialize();
}

void UMMOSpawnManagerSubsystem::Tick(float DeltaTime)
{
	ProcessRespawnTimers(DeltaTime);
}

TStatId UMMOSpawnManagerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMMOSpawnManagerSubsystem, STATGROUP_Tickables);
}

void UMMOSpawnManagerSubsystem::LoadSpawnDefinitions()
{
	if (!SpawnDataTable)
	{
		UE_LOG(LogMMOWorld, Warning, TEXT("SpawnDataTable is not set."));
		return;
	}

	static const FString ContextStr(TEXT("MMOSpawnManager::LoadSpawnDefinitions"));
	TArray<FMMOSpawnPointDefinition*> Rows;
	SpawnDataTable->GetAllRows<FMMOSpawnPointDefinition>(ContextStr, Rows);

	for (const FMMOSpawnPointDefinition* Row : Rows)
	{
		if (Row)
		{
			SpawnDefinitions.Add(Row->SpawnID, *Row);

			if (!Row->SpawnGroupID.IsNone())
			{
				SpawnGroups.FindOrAdd(Row->SpawnGroupID).AddUnique(Row->SpawnID);
			}

			FMMOSpawnPoolEntry& Pool = SpawnPools.FindOrAdd(Row->ActorClassID);
			Pool.ActorClassID = Row->ActorClassID;
			Pool.PoolSize = FMath::Max(1, Row->MaxActiveCopies * 2);
		}
	}

	UE_LOG(LogMMOWorld, Log, TEXT("Loaded %d spawn definitions from data table."), Rows.Num());
}

void UMMOSpawnManagerSubsystem::SpawnActorAtPoint(FName SpawnID)
{
	FMMOSpawnPointDefinition* SpawnDef = SpawnDefinitions.Find(SpawnID);
	if (!SpawnDef || !SpawnDef->bEnabled)
	{
		return;
	}

	if (IsSpawnDenied(SpawnID))
	{
		UE_LOG(LogMMOWorld, Verbose, TEXT("Spawn denied for %s - too many active copies."), *SpawnID.ToString());
		return;
	}

	if (ActiveSpawns.Contains(SpawnID))
	{
		UE_LOG(LogMMOWorld, Verbose, TEXT("Spawn %s already active."), *SpawnID.ToString());
		return;
	}

	AActor* SpawnedActor = GetOrCreatePooledActor(SpawnDef->ActorClassID);
	if (!SpawnedActor)
	{
		UE_LOG(LogMMOWorld, Error, TEXT("Failed to spawn actor for spawn point %s."), *SpawnID.ToString());
		return;
	}

	SpawnedActor->SetActorLocationAndRotation(SpawnDef->Location, SpawnDef->Rotation);

	FMMOActiveSpawn ActiveSpawn;
	ActiveSpawn.SpawnID = SpawnID;
	ActiveSpawn.SpawnedActor = SpawnedActor;
	ActiveSpawn.SpawnTime = GetWorld()->GetTimeSeconds();
	ActiveSpawn.RespawnTimeRemaining = SpawnDef->RespawnTimeSeconds;

	ActiveSpawns.Add(SpawnID, ActiveSpawn);

	OnActorSpawned.Broadcast(SpawnID, SpawnedActor);
}

void UMMOSpawnManagerSubsystem::DespawnActor(FName SpawnID)
{
	FMMOActiveSpawn* ActiveSpawn = ActiveSpawns.Find(SpawnID);
	if (!ActiveSpawn)
	{
		return;
	}

	FMMOSpawnPointDefinition* SpawnDef = SpawnDefinitions.Find(SpawnID);
	if (SpawnDef && ActiveSpawn->SpawnedActor)
	{
		ReturnActorToPool(ActiveSpawn->SpawnedActor, SpawnDef->ActorClassID);
	}

	ActiveSpawns.Remove(SpawnID);
	OnActorDespawned.Broadcast(SpawnID);
}

void UMMOSpawnManagerSubsystem::RespawnActor(FName SpawnID)
{
	DespawnActor(SpawnID);
	SpawnActorAtPoint(SpawnID);
}

void UMMOSpawnManagerSubsystem::TriggerEventSpawn(FName EventID, FName ZoneID)
{
	for (const auto& Pair : SpawnDefinitions)
	{
		if (Pair.Value.ZoneID == ZoneID && Pair.Value.GroupType == EMMOSpawnGroupType::Event)
		{
			SpawnActorAtPoint(Pair.Key);
		}
	}
}

void UMMOSpawnManagerSubsystem::ScaleSpawnsByPopulation(FName ZoneID, int32 CurrentPopulation)
{
	float ScaleFactor = FMath::Clamp(CurrentPopulation / 50.0f, 0.5f, 2.0f);

	for (const auto& Pair : SpawnDefinitions)
	{
		if (Pair.Value.ZoneID == ZoneID)
		{
			FMMOSpawnPointDefinition SpawnDef = Pair.Value;
			int32 AdjustedMax = FMath::CeilToInt(SpawnDef.MaxActiveCopies * ScaleFactor);
			SpawnDef.MaxActiveCopies = FMath::Max(1, AdjustedMax);
			SpawnDefinitions.Add(Pair.Key, SpawnDef);
		}
	}
}

TArray<AActor*> UMMOSpawnManagerSubsystem::GetActiveSpawnsForZone(FName ZoneID) const
{
	TArray<AActor*> Result;
	for (const auto& Pair : ActiveSpawns)
	{
		const FMMOSpawnPointDefinition* SpawnDef = SpawnDefinitions.Find(Pair.Key);
		if (SpawnDef && SpawnDef->ZoneID == ZoneID && Pair.Value.SpawnedActor)
		{
			Result.Add(Pair.Value.SpawnedActor);
		}
	}
	return Result;
}

int32 UMMOSpawnManagerSubsystem::GetActiveSpawnCount(FName ActorClassID) const
{
	int32 Count = 0;
	for (const auto& Pair : ActiveSpawns)
	{
		const FMMOSpawnPointDefinition* SpawnDef = SpawnDefinitions.Find(Pair.Key);
		if (SpawnDef && SpawnDef->ActorClassID == ActorClassID)
		{
			Count++;
		}
	}
	return Count;
}

bool UMMOSpawnManagerSubsystem::CanSpawn(FName ActorClassID, int32 MaxAllowed) const
{
	return GetActiveSpawnCount(ActorClassID) < MaxAllowed;
}

void UMMOSpawnManagerSubsystem::RegisterSpawnPoint(const FMMOSpawnPointDefinition& SpawnDef)
{
	SpawnDefinitions.Add(SpawnDef.SpawnID, SpawnDef);

	if (!SpawnDef.SpawnGroupID.IsNone())
	{
		SpawnGroups.FindOrAdd(SpawnDef.SpawnGroupID).AddUnique(SpawnDef.SpawnID);
	}

	FMMOSpawnPoolEntry& Pool = SpawnPools.FindOrAdd(SpawnDef.ActorClassID);
	Pool.ActorClassID = SpawnDef.ActorClassID;
}

void UMMOSpawnManagerSubsystem::UnregisterSpawnPoint(FName SpawnID)
{
	DespawnActor(SpawnID);
	SpawnDefinitions.Remove(SpawnID);
}

void UMMOSpawnManagerSubsystem::ProcessRespawnTimers(float DeltaTime)
{
	TArray<FName> ToRespawn;

	for (auto& Pair : ActiveSpawns)
	{
		if (!Pair.Value.SpawnedActor || !IsValid(Pair.Value.SpawnedActor))
		{
			FMMOSpawnPointDefinition* SpawnDef = SpawnDefinitions.Find(Pair.Key);
			if (SpawnDef)
			{
				Pair.Value.RespawnTimeRemaining -= DeltaTime;
				if (Pair.Value.RespawnTimeRemaining <= 0.0f)
				{
					ToRespawn.Add(Pair.Key);
				}
			}
		}
	}

	for (FName SpawnID : ToRespawn)
	{
		DespawnActor(SpawnID);
		SpawnActorAtPoint(SpawnID);
	}
}

AActor* UMMOSpawnManagerSubsystem::GetOrCreatePooledActor(FName ActorClassID)
{
	FMMOSpawnPoolEntry* Pool = SpawnPools.Find(ActorClassID);
	if (!Pool)
	{
		return nullptr;
	}

	for (AActor* Actor : Pool->PooledActors)
	{
		if (Actor && !Actor->IsActorBeingDestroyed() && !Actor->IsHidden())
		{
			continue;
		}
		if (Actor && Actor->IsHidden())
		{
			Actor->SetActorHiddenInGame(false);
			Actor->SetActorEnableCollision(true);
			Pool->ActiveCount++;
			return Actor;
		}
	}

	if (Pool->PooledActors.Num() < Pool->PoolSize)
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (NewActor)
		{
			Pool->PooledActors.Add(NewActor);
			Pool->ActiveCount++;
			return NewActor;
		}
	}

	return nullptr;
}

void UMMOSpawnManagerSubsystem::ReturnActorToPool(AActor* Actor, FName ActorClassID)
{
	if (!Actor)
	{
		return;
	}

	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);

	if (FMMOSpawnPoolEntry* Pool = SpawnPools.Find(ActorClassID))
	{
		Pool->ActiveCount = FMath::Max(0, Pool->ActiveCount - 1);
	}
}

bool UMMOSpawnManagerSubsystem::IsSpawnDenied(FName SpawnID) const
{
	const FMMOSpawnPointDefinition* SpawnDef = SpawnDefinitions.Find(SpawnID);
	if (!SpawnDef)
	{
		return true;
	}

	int32 ActiveOfClass = 0;
	for (const auto& Pair : ActiveSpawns)
	{
		const FMMOSpawnPointDefinition* Def = SpawnDefinitions.Find(Pair.Key);
		if (Def && Def->ActorClassID == SpawnDef->ActorClassID)
		{
			ActiveOfClass++;
		}
	}

	return ActiveOfClass >= SpawnDef->MaxActiveCopies;
}

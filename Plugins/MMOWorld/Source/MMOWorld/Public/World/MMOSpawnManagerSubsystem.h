#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Containers/Ticker.h"
#include "Engine/DataTable.h"
#include "MMOSpawnManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMMOSpawnGroupType : uint8
{
	Passive		UMETA(DisplayName = "Passive"),
	Aggressive	UMETA(DisplayName = "Aggressive"),
	Boss		UMETA(DisplayName = "Boss"),
	Event		UMETA(DisplayName = "Event"),
	Quest		UMETA(DisplayName = "Quest")
};

USTRUCT(BlueprintType)
struct FMMOSpawnPointDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName SpawnID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName ActorClassID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName ZoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName SpawnGroupID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float RespawnTimeSeconds = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 MaxActiveCopies = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	EMMOSpawnGroupType GroupType = EMMOSpawnGroupType::Passive;
};

USTRUCT(BlueprintType)
struct FMMOSpawnPoolEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FName ActorClassID;

	UPROPERTY()
	int32 ActiveCount = 0;

	UPROPERTY()
	int32 PoolSize = 10;

	UPROPERTY()
	TArray<AActor*> PooledActors;
};

USTRUCT(BlueprintType)
struct FMMOActiveSpawn
{
	GENERATED_BODY()

	UPROPERTY()
	FName SpawnID;

	UPROPERTY()
	AActor* SpawnedActor;

	UPROPERTY()
	float SpawnTime;

	UPROPERTY()
	float RespawnTimeRemaining;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMMOOnActorSpawned, FName, SpawnID, AActor*, SpawnedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorDespawned, FName, SpawnID);

UCLASS()
class MMOWORLD_API UMMOSpawnManagerSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableInEditor() const override { return false; }

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void LoadSpawnDefinitions();

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnActorAtPoint(FName SpawnID);

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void DespawnActor(FName SpawnID);

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void RespawnActor(FName SpawnID);

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void TriggerEventSpawn(FName EventID, FName ZoneID);

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void ScaleSpawnsByPopulation(FName ZoneID, int32 CurrentPopulation);

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	TArray<AActor*> GetActiveSpawnsForZone(FName ZoneID) const;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	int32 GetActiveSpawnCount(FName ActorClassID) const;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	bool CanSpawn(FName ActorClassID, int32 MaxAllowed) const;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void RegisterSpawnPoint(const FMMOSpawnPointDefinition& SpawnDef);

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void UnregisterSpawnPoint(FName SpawnID);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FMMOOnActorSpawned OnActorSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnActorDespawned OnActorDespawned;

protected:
	UPROPERTY()
	TMap<FName, FMMOSpawnPointDefinition> SpawnDefinitions;

	UPROPERTY()
	TMap<FName, FMMOActiveSpawn> ActiveSpawns;

	UPROPERTY()
	TMap<FName, FMMOSpawnPoolEntry> SpawnPools;

	TMap<FName, TArray<FName>> SpawnGroups;

	UPROPERTY()
	UDataTable* SpawnDataTable;

	void ProcessRespawnTimers(float DeltaTime);
	AActor* GetOrCreatePooledActor(FName ActorClassID);
	void ReturnActorToPool(AActor* Actor, FName ActorClassID);
	bool IsSpawnDenied(FName SpawnID) const;
};

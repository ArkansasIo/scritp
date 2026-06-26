#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MMOZoneData.h"
#include "MMOInstanceManager.h"
#include "MMOWeatherSystem.h"
#include "MMOTimeSystem.h"
#include "MMOSpawnManagerSubsystem.h"
#include "MMOWorldSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneTransferRequest, APlayerController*, Player, FName, TargetZoneID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMMOOnZonePopulationChanged, FName, ZoneID, int32, NewPopulation, int32, Capacity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneAvailabilityChanged, FName, ZoneID, bool, bAvailable);

USTRUCT(BlueprintType)
struct FMMOActiveZoneInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FName ZoneID;

	UPROPERTY()
	UMMOZoneData* ZoneData;

	UPROPERTY()
	int32 CurrentPopulation = 0;

	UPROPERTY()
	bool bAvailable = true;

	UPROPERTY()
	bool bMaintenanceMode = false;

	UPROPERTY()
	bool bLocked = false;

	UPROPERTY()
	TArray<FName> LoadedSubLevels;
};

UCLASS()
class MMOWORLD_API UMMOWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Zone")
	void LoadZoneDefinitions();

	UFUNCTION(BlueprintCallable, Category = "Zone")
	UMMOZoneData* GetZoneByID(FName ZoneID) const;

	UFUNCTION(BlueprintCallable, Category = "Zone")
	FName GetCurrentZoneForPlayer(APlayerController* Player) const;

	UFUNCTION(BlueprintCallable, Category = "Zone")
	bool IsZoneAvailable(FName ZoneID) const;

	UFUNCTION(BlueprintCallable, Category = "Zone")
	void SetZoneAvailability(FName ZoneID, bool bAvailable);

	UFUNCTION(BlueprintCallable, Category = "Zone")
	void SetZoneMaintenance(FName ZoneID, bool bMaintenance);

	UFUNCTION(BlueprintCallable, Category = "Zone")
	void ServerTravelToZone(APlayerController* Player, FName TargetZoneID);

	UFUNCTION(BlueprintCallable, Category = "Zone")
	void RequestZoneTransfer(APlayerController* Player, FName TargetZoneID);

	UFUNCTION(BlueprintCallable, Category = "Population")
	int32 GetZonePopulation(FName ZoneID) const;

	UFUNCTION(BlueprintCallable, Category = "Population")
	void AddZonePopulation(FName ZoneID, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Population")
	void RemoveZonePopulation(FName ZoneID, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Population")
	TArray<FName> GetActiveZoneIDs() const;

	UFUNCTION(BlueprintCallable, Category = "Instances")
	UMMOInstanceManager* GetInstanceManager() const { return InstanceManager; }

	UFUNCTION(BlueprintCallable, Category = "Weather")
	UMMOWeatherSystem* GetWeatherSystem() const { return WeatherSystem; }

	UFUNCTION(BlueprintCallable, Category = "Time")
	UMMOTimeSystem* GetTimeSystem() const { return TimeSystem; }

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	UMMOSpawnManagerSubsystem* GetSpawnManager() const { return SpawnManager; }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnZoneTransferRequest OnZoneTransferRequest;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FMMOOnZonePopulationChanged OnZonePopulationChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnZoneAvailabilityChanged OnZoneAvailabilityChanged;

protected:
	UPROPERTY()
	TMap<FName, FMMOActiveZoneInfo> ActiveZones;

	UPROPERTY()
	TMap<FName, UMMOZoneData*> ZoneDefinitions;

	UPROPERTY()
	UMMOInstanceManager* InstanceManager;

	UPROPERTY()
	UMMOWeatherSystem* WeatherSystem;

	UPROPERTY()
	UMMOTimeSystem* TimeSystem;

	UPROPERTY()
	UMMOSpawnManagerSubsystem* SpawnManager;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TArray<TSoftObjectPtr<UMMOZoneData>> ZoneDataAssets;
};

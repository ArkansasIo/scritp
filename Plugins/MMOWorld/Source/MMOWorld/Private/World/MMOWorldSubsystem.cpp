#include "World/MMOWorldSubsystem.h"
#include "MMOWorld.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UMMOWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InstanceManager = NewObject<UMMOInstanceManager>(this);
	InstanceManager->Initialize(Collection);

	WeatherSystem = NewObject<UMMOWeatherSystem>(this);
	WeatherSystem->Initialize(Collection);

	TimeSystem = NewObject<UMMOTimeSystem>(this);
	TimeSystem->Initialize(Collection);

	SpawnManager = NewObject<UMMOSpawnManagerSubsystem>(this);
	SpawnManager->Initialize(Collection);

	LoadZoneDefinitions();

	UE_LOG(LogMMOWorld, Log, TEXT("MMOWorldSubsystem initialized."));
}

void UMMOWorldSubsystem::Deinitialize()
{
	ActiveZones.Empty();
	ZoneDefinitions.Empty();

	if (InstanceManager)
	{
		InstanceManager->Deinitialize();
		InstanceManager = nullptr;
	}

	if (WeatherSystem)
	{
		WeatherSystem->Deinitialize();
		WeatherSystem = nullptr;
	}

	if (TimeSystem)
	{
		TimeSystem->Deinitialize();
		TimeSystem = nullptr;
	}

	if (SpawnManager)
	{
		SpawnManager->Deinitialize();
		SpawnManager = nullptr;
	}

	Super::Deinitialize();
}

TStatId UMMOWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMMOWorldSubsystem, STATGROUP_Tickables);
}

void UMMOWorldSubsystem::LoadZoneDefinitions()
{
	ZoneDefinitions.Empty();
	ActiveZones.Empty();

	for (const TSoftObjectPtr<UMMOZoneData>& AssetPtr : ZoneDataAssets)
	{
		if (UMMOZoneData* ZoneData = AssetPtr.LoadSynchronous())
		{
			FName ZoneID = ZoneData->ZoneID;
			ZoneDefinitions.Add(ZoneID, ZoneData);

			FMMOActiveZoneInfo ZoneInfo;
			ZoneInfo.ZoneID = ZoneID;
			ZoneInfo.ZoneData = ZoneData;
			ZoneInfo.CurrentPopulation = 0;
			ZoneInfo.bAvailable = true;
			ZoneInfo.bMaintenanceMode = false;
			ZoneInfo.bLocked = false;

			ActiveZones.Add(ZoneID, ZoneInfo);

			UE_LOG(LogMMOWorld, Verbose, TEXT("Loaded zone: %s"), *ZoneID.ToString());
		}
	}

	UE_LOG(LogMMOWorld, Log, TEXT("Loaded %d zone definitions."), ZoneDefinitions.Num());
}

UMMOZoneData* UMMOWorldSubsystem::GetZoneByID(FName ZoneID) const
{
	if (const UMMOZoneData* const* Found = ZoneDefinitions.Find(ZoneID))
	{
		return *Found;
	}
	return nullptr;
}

FName UMMOWorldSubsystem::GetCurrentZoneForPlayer(APlayerController* Player) const
{
	if (!Player || !Player->GetPawn())
	{
		return NAME_None;
	}

	FVector PlayerLocation = Player->GetPawn()->GetActorLocation();

	for (const auto& Pair : ActiveZones)
	{
		const FMMOActiveZoneInfo& ZoneInfo = Pair.Value;
		if (ZoneInfo.ZoneData && ZoneInfo.ZoneData->ZoneMap.IsValid())
		{
			UWorld* ZoneWorld = ZoneInfo.ZoneData->ZoneMap.Get();
			if (ZoneWorld && ZoneWorld == Player->GetWorld())
			{
				return Pair.Key;
			}
		}
	}

	return NAME_None;
}

bool UMMOWorldSubsystem::IsZoneAvailable(FName ZoneID) const
{
	if (const FMMOActiveZoneInfo* ZoneInfo = ActiveZones.Find(ZoneID))
	{
		return ZoneInfo->bAvailable && !ZoneInfo->bMaintenanceMode && !ZoneInfo->bLocked;
	}
	return false;
}

void UMMOWorldSubsystem::SetZoneAvailability(FName ZoneID, bool bAvailable)
{
	if (FMMOActiveZoneInfo* ZoneInfo = ActiveZones.Find(ZoneID))
	{
		ZoneInfo->bAvailable = bAvailable;
		OnZoneAvailabilityChanged.Broadcast(ZoneID, bAvailable);
	}
}

void UMMOWorldSubsystem::SetZoneMaintenance(FName ZoneID, bool bMaintenance)
{
	if (FMMOActiveZoneInfo* ZoneInfo = ActiveZones.Find(ZoneID))
	{
		ZoneInfo->bMaintenanceMode = bMaintenance;
		if (bMaintenance)
		{
			ZoneInfo->bAvailable = false;
		}
		OnZoneAvailabilityChanged.Broadcast(ZoneID, !bMaintenance);
	}
}

void UMMOWorldSubsystem::ServerTravelToZone(APlayerController* Player, FName TargetZoneID)
{
	if (!Player || !Player->HasAuthority())
	{
		return;
	}

	if (!IsZoneAvailable(TargetZoneID))
	{
		UE_LOG(LogMMOWorld, Warning, TEXT("Zone %s is not available for travel."), *TargetZoneID.ToString());
		return;
	}

	UMMOZoneData* ZoneData = GetZoneByID(TargetZoneID);
	if (!ZoneData || !ZoneData->ZoneMap.IsValid())
	{
		UE_LOG(LogMMOWorld, Error, TEXT("Zone %s has no valid map reference."), *TargetZoneID.ToString());
		return;
	}

	FString TravelURL = ZoneData->ZoneMap.ToSoftObjectPath().ToString();
	Player->GetWorld()->ServerTravel(TravelURL);
}

void UMMOWorldSubsystem::RequestZoneTransfer(APlayerController* Player, FName TargetZoneID)
{
	if (!Player)
	{
		return;
	}

	OnZoneTransferRequest.Broadcast(Player, TargetZoneID);
}

int32 UMMOWorldSubsystem::GetZonePopulation(FName ZoneID) const
{
	if (const FMMOActiveZoneInfo* ZoneInfo = ActiveZones.Find(ZoneID))
	{
		return ZoneInfo->CurrentPopulation;
	}
	return 0;
}

void UMMOWorldSubsystem::AddZonePopulation(FName ZoneID, int32 Amount)
{
	if (FMMOActiveZoneInfo* ZoneInfo = ActiveZones.Find(ZoneID))
	{
		ZoneInfo->CurrentPopulation = FMath::Max(0, ZoneInfo->CurrentPopulation + Amount);
		int32 Capacity = ZoneInfo->ZoneData ? ZoneInfo->ZoneData->MaxPlayerCapacity : 0;
		OnZonePopulationChanged.Broadcast(ZoneID, ZoneInfo->CurrentPopulation, Capacity);
	}
}

void UMMOWorldSubsystem::RemoveZonePopulation(FName ZoneID, int32 Amount)
{
	AddZonePopulation(ZoneID, -Amount);
}

TArray<FName> UMMOWorldSubsystem::GetActiveZoneIDs() const
{
	TArray<FName> ZoneIDs;
	ActiveZones.GetKeys(ZoneIDs);
	return ZoneIDs;
}

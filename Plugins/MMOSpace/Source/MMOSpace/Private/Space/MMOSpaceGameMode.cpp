#include "Space/MMOSpaceGameMode.h"
#include "MMOSpace.h"
#include "Database/MMOSQLDatabaseSubsystem.h"
#include "Universe/MMOGalaxySubsystem.h"
#include "Empire/MMOEmpireSubsystem.h"
#include "Fleet/MMOFleetSubsystem.h"
#include "Combat/MMOCombatEngine.h"
#include "Research/MMOResearchSubsystem.h"
#include "Economy/MMOEconomySubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

AMMOSpaceGameMode::AMMOSpaceGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	UniverseConfig.GalaxyCount = 9;
	UniverseConfig.SystemsPerGalaxy = 499;
	UniverseConfig.PlanetsPerSystem = 15;
	UniverseConfig.SpeedFactor = 1.0;
	UniverseConfig.EconomySpeed = 1.0;
	UniverseConfig.ResearchSpeed = 1.0;
	UniverseConfig.FleetSpeed = 1.0;
}

void AMMOSpaceGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (GetGameInstance())
	{
		DatabaseSub = GetGameInstance()->GetSubsystem<UMMOSQLDatabaseSubsystem>();
		GalaxySub = GetGameInstance()->GetSubsystem<UMMOGalaxySubsystem>();
		EmpireSub = GetGameInstance()->GetSubsystem<UMMOEmpireSubsystem>();
		FleetSub = GetGameInstance()->GetSubsystem<UMMOFleetSubsystem>();
		CombatSub = GetGameInstance()->GetSubsystem<UMMOCombatEngine>();
		ResearchSub = GetGameInstance()->GetSubsystem<UMMOResearchSubsystem>();
		EconomySub = GetGameInstance()->GetSubsystem<UMMOEconomySubsystem>();
	}

	if (bAutoGenerateUniverse)
	{
		InitializeGameWorld();
	}
}

void AMMOSpaceGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bWorldInitialized) return;

	ResourceTimer += DeltaTime;
	FleetTimer += DeltaTime;
	ResearchTimer += DeltaTime;

	if (ResourceTimer >= ResourceTickInterval && EmpireSub)
	{
		EmpireSub->TickResourceGeneration(ResourceTimer);
		ResourceTimer = 0.0f;
	}

	if (FleetTimer >= FleetUpdateInterval && FleetSub)
	{
		FleetSub->UpdateFleetPositions(FleetTimer);
		FleetTimer = 0.0f;
	}

	if (ResearchTimer >= ResearchUpdateInterval && ResearchSub)
	{
		ResearchSub->UpdateResearchProgress(ResearchTimer);
		ResearchTimer = 0.0f;
	}
}

void AMMOSpaceGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	UE_LOG(LogMMOSpace, Log, TEXT("Space Game Mode initialized on map: %s"), *MapName);
}

void AMMOSpaceGameMode::InitializeGameWorld()
{
	if (GalaxySub)
	{
		GalaxySub->GenerateUniverse(UniverseConfig);
	}

	if (DatabaseSub)
	{
		DatabaseSub->Connect(SQLConnectionString.IsEmpty()
			? TEXT("file://local/savegame")
			: SQLConnectionString);
	}

	bWorldInitialized = true;
	UE_LOG(LogMMOSpace, Log, TEXT("Game world initialized: %d galaxies, ~%d systems"),
		UniverseConfig.GalaxyCount, UniverseConfig.GalaxyCount * UniverseConfig.SystemsPerGalaxy);
}

void AMMOSpaceGameMode::StartNewEmpire(const FString& PlayerId, const FString& EmpireName, const FString& HomePlanetName)
{
	if (!bWorldInitialized)
	{
		InitializeGameWorld();
	}

	if (EmpireSub)
	{
		EmpireSub->InitializeEmpire(PlayerId, HomePlanetName);
	}

	if (GalaxySub)
	{
		FGalaxyCoordinate HomeWorld;
		HomeWorld.Galaxy = 1;
		HomeWorld.SolarSystem = FMath::RandRange(100, 400);
		HomeWorld.Position = FMath::RandRange(4, 9);
		HomeWorld.PlanetIndex = 0;

		FString PlanetName = GalaxySub->GetPlanetName(HomeWorld.Galaxy, HomeWorld.SolarSystem, HomeWorld.Position);

		UE_LOG(LogMMOSpace, Log, TEXT("New empire '%s' started at %s (planet: %s)"),
			*EmpireName, *HomeWorld.ToString(), *PlanetName);
	}

	if (EconomySub)
	{
		TArray<FBuildingLevel> StarterBuildings;
		FBuildingLevel MetalMine; MetalMine.Type = EBuildingType::MetalMine; MetalMine.Level = 1; StarterBuildings.Add(MetalMine);
		FBuildingLevel CrystalMine; CrystalMine.Type = EBuildingType::CrystalMine; CrystalMine.Level = 1; StarterBuildings.Add(CrystalMine);
		FBuildingLevel SolarPlant; SolarPlant.Type = EBuildingType::SolarPlant; SolarPlant.Level = 1; StarterBuildings.Add(SolarPlant);

		FResourceSet Prod = EconomySub->GetTotalProduction(StarterBuildings);
		if (EmpireSub)
			EmpireSub->UpdateResourceProduction(StarterBuildings);

		UE_LOG(LogMMOSpace, Log, TEXT("Starting production: M=%.0f/h C=%.0f/h E=%.0f/h"),
			Prod.Metal, Prod.Crystal, Prod.Energy);
	}

	if (DatabaseSub)
	{
		FPlayerSaveData SaveData;
		SaveData.PlayerId = PlayerId;
		SaveData.Username = EmpireName;
		SaveData.PlanetName = HomePlanetName;
		SaveData.EmpireLevel = 1;
		if (EmpireSub)
			SaveData.Resources = EmpireSub->GetResources();
		DatabaseSub->SavePlayerData(SaveData);
	}
}

void AMMOSpaceGameMode::SetUniverseConfig(const FUniverseConfig& NewConfig)
{
	UniverseConfig = NewConfig;
	if (bWorldInitialized)
	{
		InitializeGameWorld();
	}
}

void AMMOSpaceGameMode::ShutdownGameWorld()
{
	bWorldInitialized = false;
	if (DatabaseSub)
	{
		DatabaseSub->Disconnect();
	}
	UE_LOG(LogMMOSpace, Log, TEXT("Game world shutdown"));
}

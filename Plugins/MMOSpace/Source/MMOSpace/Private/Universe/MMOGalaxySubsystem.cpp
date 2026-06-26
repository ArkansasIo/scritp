#include "Universe/MMOGalaxySubsystem.h"
#include "MMOSpace.h"
#include "Misc/DateTime.h"

static const TCHAR* PlanetNamePrefixes[] = {
	TEXT("Al"), TEXT("Bet"), TEXT("Cal"), TEXT("Del"), TEXT("Eps"), TEXT("Fel"),
	TEXT("Gal"), TEXT("Hal"), TEXT("Ion"), TEXT("Jan"), TEXT("Kel"), TEXT("Lyr"),
	TEXT("Max"), TEXT("Neb"), TEXT("Ori"), TEXT("Peg"), TEXT("Qua"), TEXT("Rig"),
	TEXT("Sir"), TEXT("Tau"), TEXT("Veg"), TEXT("Xen"), TEXT("Zet"), TEXT("Amp")
};

static const TCHAR* PlanetNameSuffixes[] = {
	TEXT("aris"), TEXT("on"), TEXT("ia"), TEXT("or"), TEXT("an"), TEXT("is"),
	TEXT("ax"), TEXT("ius"), TEXT("on"), TEXT("ar"), TEXT("os"), TEXT("is"),
	TEXT("or"), TEXT("ian"), TEXT("ar"), TEXT("on"), TEXT("ius"), TEXT("ax"),
	TEXT("oris"), TEXT("on"), TEXT("ia"), TEXT("os"), TEXT("an"), TEXT("is")
};

void UMMOGalaxySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GalaxyRNG.Initialize(FDateTime::Now().GetTicks());
	UE_LOG(LogMMOSpace, Log, TEXT("Galaxy Subsystem initialized"));
}

void UMMOGalaxySubsystem::GenerateUniverse(const FUniverseConfig& Config)
{
	UniverseConfig = Config;
	Systems.Empty();
	OccupiedSectors.Empty();

	for (int32 G = 1; G <= Config.GalaxyCount; G++)
	{
		for (int32 S = 1; S <= Config.SystemsPerGalaxy; S++)
		{
			FStarSystemData SystemData;
			SystemData.GalaxyID = G;
			SystemData.SystemID = S;
			GenerateSystemName(G, S, SystemData.SystemName);
			SystemData.StarType = GalaxyRNG.RandRange(0, 7);
			SystemData.ResourceMultiplier = 0.5 + GalaxyRNG.FRand() * 1.5;

			int32 PlanetCount = FMath::Min(Config.PlanetsPerSystem, GalaxyRNG.RandRange(6, Config.PlanetsPerSystem));
			for (int32 P = 1; P <= PlanetCount; P++)
			{
				SystemData.PlanetNames.Add(GeneratePlanetName());
				SystemData.PlanetTypes.Add(GeneratePlanetType(P));
			}

			FString Key = FString::Printf(TEXT("%d-%d"), G, S);
			Systems.Add(Key, SystemData);
		}
	}

	UE_LOG(LogMMOSpace, Log, TEXT("Generated universe: %d galaxies, ~%d systems"),
		Config.GalaxyCount, Config.GalaxyCount * Config.SystemsPerGalaxy);
}

FStarSystemData UMMOGalaxySubsystem::GetSystemData(int32 Galaxy, int32 System) const
{
	FString Key = FString::Printf(TEXT("%d-%d"), Galaxy, System);
	if (const FStarSystemData* Data = Systems.Find(Key))
		return *Data;

	FStarSystemData Empty;
	Empty.GalaxyID = Galaxy;
	Empty.SystemID = System;
	Empty.SystemName = TEXT("Unknown System");
	return Empty;
}

TArray<FStarSystemData> UMMOGalaxySubsystem::GetSystemsInRange(int32 Galaxy, int32 StartSystem, int32 Range) const
{
	TArray<FStarSystemData> Results;
	for (int32 S = FMath::Max(1, StartSystem - Range); S <= FMath::Min(UniverseConfig.SystemsPerGalaxy, StartSystem + Range); S++)
	{
		if (S != StartSystem)
			Results.Add(GetSystemData(Galaxy, S));
	}
	return Results;
}

FString UMMOGalaxySubsystem::GetPlanetName(int32 Galaxy, int32 System, int32 Position) const
{
	FStarSystemData Data = GetSystemData(Galaxy, System);
	if (Position >= 0 && Position < Data.PlanetNames.Num())
		return Data.PlanetNames[Position];
	return TEXT("Unknown Planet");
}

EPlanetType UMMOGalaxySubsystem::GetPlanetType(int32 Galaxy, int32 System, int32 Position) const
{
	FStarSystemData Data = GetSystemData(Galaxy, System);
	if (Position >= 0 && Position < Data.PlanetTypes.Num())
		return Data.PlanetTypes[Position];
	return EPlanetType::Terran;
}

int32 UMMOGalaxySubsystem::CalculateDistance(const FGalaxyCoordinate& A, const FGalaxyCoordinate& B) const
{
	int32 Dist = 0;
	if (A.Galaxy != B.Galaxy)
	{
		Dist += FMath::Abs(A.Galaxy - B.Galaxy) * 20000;
	}
	Dist += FMath::Abs(A.SolarSystem - B.SolarSystem) * 95;
	Dist += FMath::Abs(A.Position - B.Position) * 5;
	return Dist;
}

TArray<FGalaxyCoordinate> UMMOGalaxySubsystem::GetColonizablePlanets(const FUniverseConfig& Config) const
{
	TArray<FGalaxyCoordinate> Available;
	for (int32 G = 1; G <= Config.GalaxyCount; G++)
	{
		for (int32 S = 1; S <= Config.SystemsPerGalaxy; S++)
		{
			FStarSystemData Data = GetSystemData(G, S);
			for (int32 P = 2; P <= Data.PlanetTypes.Num() && P <= 4; P++)
			{
				FGalaxyCoordinate Coord;
				Coord.Galaxy = G;
				Coord.SolarSystem = S;
				Coord.Position = P;
				Coord.PlanetIndex = P - 1;
				Available.Add(Coord);
			}
		}
	}
	return Available;
}

void UMMOGalaxySubsystem::ScanSector(const FGalaxyCoordinate& Coord)
{
	FString Key = FString::Printf(TEXT("%d-%d"), Coord.Galaxy, Coord.SolarSystem);
	FStarSystemData Data = GetSystemData(Coord.Galaxy, Coord.SolarSystem);
	bool bHasActivity = OccupiedSectors.Contains(Coord);
	OnSectorScanned.Broadcast(Coord, bHasActivity, bHasActivity ? TEXT("Occupied") : TEXT("Unoccupied"));
}

TArray<FGalaxyCoordinate> UMMOGalaxySubsystem::GetOccupiedSectors() const
{
	return OccupiedSectors;
}

void UMMOGalaxySubsystem::GenerateSystemName(int32 Galaxy, int32 System, FString& OutName)
{
	FRandomStream NameRNG(Galaxy * 1000000 + System);
	int32 PrefixIdx = NameRNG.RandRange(0, UE_ARRAY_COUNT(PlanetNamePrefixes) - 1);
	int32 SuffixIdx = NameRNG.RandRange(0, UE_ARRAY_COUNT(PlanetNameSuffixes) - 1);

	switch (NameRNG.RandRange(0, 3))
	{
	case 0:
		OutName = FString::Printf(TEXT("%s-%s %d"), PlanetNamePrefixes[PrefixIdx], PlanetNameSuffixes[SuffixIdx], System);
		break;
	case 1:
		OutName = FString::Printf(TEXT("%s %d"), PlanetNamePrefixes[PrefixIdx], System);
		break;
	case 2:
		OutName = FString::Printf(TEXT("%s%s-%d"), PlanetNamePrefixes[PrefixIdx], PlanetNameSuffixes[SuffixIdx], Galaxy);
		break;
	default:
		OutName = FString::Printf(TEXT("System %d-%d"), Galaxy, System);
		break;
	}
}

FString UMMOGalaxySubsystem::GeneratePlanetName() const
{
	int32 PrefixIdx = GalaxyRNG.RandRange(0, UE_ARRAY_COUNT(PlanetNamePrefixes) - 1);
	int32 SuffixIdx = GalaxyRNG.RandRange(0, UE_ARRAY_COUNT(PlanetNameSuffixes) - 1);
	return FString::Printf(TEXT("%s%s"), PlanetNamePrefixes[PrefixIdx], PlanetNameSuffixes[SuffixIdx]);
}

EPlanetType UMMOGalaxySubsystem::GeneratePlanetType(int32 Position) const
{
	if (Position <= 3) return EPlanetType::Terran;
	if (Position <= 5) return EPlanetType::Oceanic;
	if (Position <= 7) return EPlanetType::Arid;
	if (Position <= 9) return EPlanetType::DesertWorld;
	if (Position <= 11) return EPlanetType::IceWorld;
	return EPlanetType::Volcanic;
}

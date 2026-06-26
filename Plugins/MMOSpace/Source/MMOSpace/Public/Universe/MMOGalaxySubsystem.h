#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Space/MMOSpaceTypes.h"
#include "MMOGalaxySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSectorScanned, FGalaxyCoordinate, SectorCoord, bool, bHasActivity, const FString&, OwnerName);

USTRUCT(BlueprintType)
struct FStarSystemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
	int32 GalaxyID = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
	int32 SystemID = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
	FString SystemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
	TArray<FString> PlanetNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
	TArray<EPlanetType> PlanetTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
	int32 StarType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
	double ResourceMultiplier = 1.0;
};

USTRUCT(BlueprintType)
struct FUniverseConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
	int32 GalaxyCount = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
	int32 SystemsPerGalaxy = 499;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
	int32 PlanetsPerSystem = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
	int32 MoonChance = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
	int32 DebrisFieldChance = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
	int32 MinColonies = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
	int32 MaxColonies = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
	double SpeedFactor = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
	double EconomySpeed = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
	double ResearchSpeed = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
	double FleetSpeed = 1.0;
};

UCLASS()
class MMOSPACE_API UMMOGalaxySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Galaxy")
	FOnSectorScanned OnSectorScanned;

	UFUNCTION(BlueprintCallable, Category = "Galaxy")
	void GenerateUniverse(const FUniverseConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "Galaxy")
	FStarSystemData GetSystemData(int32 Galaxy, int32 System) const;

	UFUNCTION(BlueprintCallable, Category = "Galaxy")
	TArray<FStarSystemData> GetSystemsInRange(int32 Galaxy, int32 StartSystem, int32 Range) const;

	UFUNCTION(BlueprintCallable, Category = "Galaxy")
	FString GetPlanetName(int32 Galaxy, int32 System, int32 Position) const;

	UFUNCTION(BlueprintCallable, Category = "Galaxy")
	EPlanetType GetPlanetType(int32 Galaxy, int32 System, int32 Position) const;

	UFUNCTION(BlueprintCallable, Category = "Galaxy")
	int32 CalculateDistance(const FGalaxyCoordinate& A, const FGalaxyCoordinate& B) const;

	UFUNCTION(BlueprintCallable, Category = "Galaxy")
	TArray<FGalaxyCoordinate> GetColonizablePlanets(const FUniverseConfig& Config) const;

	UFUNCTION(BlueprintPure, Category = "Galaxy")
	int32 GetTotalSystems() const { return Systems.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Galaxy")
	void ScanSector(const FGalaxyCoordinate& Coord);

	UFUNCTION(BlueprintCallable, Category = "Galaxy")
	TArray<FGalaxyCoordinate> GetOccupiedSectors() const;

	UFUNCTION(BlueprintCallable, Category = "Galaxy")
	void GenerateSystemName(int32 Galaxy, int32 System, FString& OutName);

protected:
	FString GeneratePlanetName() const;
	EPlanetType GeneratePlanetType(int32 Position) const;

private:
	FUniverseConfig UniverseConfig;
	TMap<FString, FStarSystemData> Systems;
	TArray<FGalaxyCoordinate> OccupiedSectors;
	FRandomStream GalaxyRNG;
};

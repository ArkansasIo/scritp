#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MMOSpaceTypes.generated.h"

UENUM(BlueprintType)
enum class EResourceType : uint8
{
	Metal UMETA(DisplayName = "Metal"),
	Crystal UMETA(DisplayName = "Crystal"),
	Deuterium UMETA(DisplayName = "Deuterium"),
	Energy UMETA(DisplayName = "Energy"),
	Food UMETA(DisplayName = "Food"),
	Water UMETA(DisplayName = "Water"),
	Credits UMETA(DisplayName = "Credits"),
	DarkMatter UMETA(DisplayName = "Dark Matter"),
	Antimatter UMETA(DisplayName = "Antimatter"),
	VoidMatter UMETA(DisplayName = "Void Matter"),
	CelestialEssence UMETA(DisplayName = "Celestial Essence")
};

UENUM(BlueprintType)
enum class EShipClass : uint8
{
	LightFighter UMETA(DisplayName = "Light Fighter"),
	HeavyFighter UMETA(DisplayName = "Heavy Fighter"),
	Corvette UMETA(DisplayName = "Corvette"),
	Frigate UMETA(DisplayName = "Frigate"),
	Destroyer UMETA(DisplayName = "Destroyer"),
	Battleship UMETA(DisplayName = "Battleship"),
	Carrier UMETA(DisplayName = "Carrier"),
	Dreadnought UMETA(DisplayName = "Dreadnought"),
	Titan UMETA(DisplayName = "Titan"),
	AscendedWarship UMETA(DisplayName = "Ascended Warship")
};

UENUM(BlueprintType)
enum class EFleetFormation : uint8
{
	LineAbreast UMETA(DisplayName = "Line Abreast"),
	Wedge UMETA(DisplayName = "Wedge"),
	Vanguard UMETA(DisplayName = "Vanguard"),
	ShieldWall UMETA(DisplayName = "Shield Wall"),
	Pincer UMETA(DisplayName = "Pincer")
};

UENUM(BlueprintType)
enum class EGovernmentType : uint8
{
	Anarchy UMETA(DisplayName = "Anarchy"),
	Autocracy UMETA(DisplayName = "Autocracy"),
	Democracy UMETA(DisplayName = "Democracy"),
	Oligarchy UMETA(DisplayName = "Oligarchy"),
	Technocracy UMETA(DisplayName = "Technocracy"),
	Theocracy UMETA(DisplayName = "Theocracy"),
	Corporate UMETA(DisplayName = "Corporate"),
	HiveMind UMETA(DisplayName = "Hive Mind"),
	MilitaryJunta UMETA(DisplayName = "Military Junta"),
	GalacticEmpire UMETA(DisplayName = "Galactic Empire")
};

UENUM(BlueprintType)
enum class EBuildingType : uint8
{
	MetalMine UMETA(DisplayName = "Metal Mine"),
	CrystalMine UMETA(DisplayName = "Crystal Mine"),
	DeuteriumSynthesizer UMETA(DisplayName = "Deuterium Synthesizer"),
	SolarPlant UMETA(DisplayName = "Solar Plant"),
	FusionReactor UMETA(DisplayName = "Fusion Reactor"),
	RoboticsFactory UMETA(DisplayName = "Robotics Factory"),
	Shipyard UMETA(DisplayName = "Shipyard"),
	ResearchLab UMETA(DisplayName = "Research Lab"),
	NaniteFactory UMETA(DisplayName = "Nanite Factory"),
	Terraformer UMETA(DisplayName = "Terraformer"),
	SpaceDock UMETA(DisplayName = "Space Dock"),
	MissileSilo UMETA(DisplayName = "Missile Silo"),
	OrbitalDefense UMETA(DisplayName = "Orbital Defense"),
	ShieldDome UMETA(DisplayName = "Shield Dome")
};

UENUM(BlueprintType)
enum class EResearchCategory : uint8
{
	Weapons UMETA(DisplayName = "Weapons"),
	Shielding UMETA(DisplayName = "Shielding"),
	Armor UMETA(DisplayName = "Armor"),
	Energy UMETA(DisplayName = "Energy"),
	Propulsion UMETA(DisplayName = "Propulsion"),
	Lasers UMETA(DisplayName = "Lasers"),
	Ion UMETA(DisplayName = "Ion"),
	Plasma UMETA(DisplayName = "Plasma"),
	Hyperspace UMETA(DisplayName = "Hyperspace"),
	Combustion UMETA(DisplayName = "Combustion"),
	Impulse UMETA(DisplayName = "Impulse"),
	Warfare UMETA(DisplayName = "Warfare"),
	Colonization UMETA(DisplayName = "Colonization"),
	Economics UMETA(DisplayName = "Economics"),
	Espionage UMETA(DisplayName = "Espionage")
};

UENUM(BlueprintType)
enum class ECommanderRarity : uint8
{
	Common UMETA(DisplayName = "Common"),
	Uncommon UMETA(DisplayName = "Uncommon"),
	Rare UMETA(DisplayName = "Rare"),
	Epic UMETA(DisplayName = "Epic"),
	Legendary UMETA(DisplayName = "Legendary"),
	Mythic UMETA(DisplayName = "Mythic")
};

UENUM(BlueprintType)
enum class EPlanetType : uint8
{
	Terran UMETA(DisplayName = "Terran"),
	Arid UMETA(DisplayName = "Arid"),
	Oceanic UMETA(DisplayName = "Oceanic"),
	Volcanic UMETA(DisplayName = "Volcanic"),
	GasGiant UMETA(DisplayName = "Gas Giant"),
	IceWorld UMETA(DisplayName = "Ice World"),
	DesertWorld UMETA(DisplayName = "Desert World"),
	JungleWorld UMETA(DisplayName = "Jungle World"),
	Barren UMETA(DisplayName = "Barren"),
	Lava UMETA(DisplayName = "Lava"),
	Toxic UMETA(DisplayName = "Toxic"),
	Gaia UMETA(DisplayName = "Gaia"),
	Artificial UMETA(DisplayName = "Artificial")
};

USTRUCT(BlueprintType)
struct FResourceSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	double Metal = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	double Crystal = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	double Deuterium = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	double Energy = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	double Food = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	double Water = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	double Credits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	double DarkMatter = 0;
};

USTRUCT(BlueprintType)
struct FGalaxyCoordinate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
	int32 Galaxy = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
	int32 SolarSystem = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
	int32 Position = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
	int32 PlanetIndex = 1;

	FString ToString() const
	{
		return FString::Printf(TEXT("%d:%d:%d:%d"), Galaxy, SolarSystem, Position, PlanetIndex);
	}

	static FGalaxyCoordinate FromString(const FString& CoordString)
	{
		FGalaxyCoordinate Coord;
		TArray<FString> Parts;
		CoordString.ParseIntoArray(Parts, TEXT(":"));
		if (Parts.Num() >= 1) Coord.Galaxy = FCString::Atoi(*Parts[0]);
		if (Parts.Num() >= 2) Coord.SolarSystem = FCString::Atoi(*Parts[1]);
		if (Parts.Num() >= 3) Coord.Position = FCString::Atoi(*Parts[2]);
		if (Parts.Num() >= 4) Coord.PlanetIndex = FCString::Atoi(*Parts[3]);
		return Coord;
	}
};

USTRUCT(BlueprintType)
struct FShipStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	double StructuralIntegrity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	double ShieldPower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	double WeaponPower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	double Speed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	double CargoCapacity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	double FuelConsumption = 0;
};

USTRUCT(BlueprintType)
struct FBuildingLevel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	EBuildingType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 Level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	bool bIsUpgrading = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	double UpgradeEndTime = 0;
};

USTRUCT(BlueprintType)
struct FResearchProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	EResearchCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	int32 CurrentLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	bool bIsResearching = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
	double ResearchEndTime = 0;
};

#include "Fleet/MMOFleetSubsystem.h"
#include "MMOSpace.h"
#include "Misc/DateTime.h"

void UMMOFleetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BuildCosts.Empty();

	TArray<EShipClass> AllClasses = {
		EShipClass::LightFighter, EShipClass::HeavyFighter, EShipClass::Corvette,
		EShipClass::Frigate, EShipClass::Destroyer, EShipClass::Battleship,
		EShipClass::Carrier, EShipClass::Dreadnought, EShipClass::Titan,
		EShipClass::AscendedWarship
	};

	for (EShipClass ShipClass : AllClasses)
	{
		FShipBuildCost Cost;
		Cost.ShipClass = ShipClass;
		Cost.Cost.Metal = 1000 * ((int32)ShipClass + 1) * 2;
		Cost.Cost.Crystal = 500 * ((int32)ShipClass + 1);
		Cost.Cost.Deuterium = 100 * ((int32)ShipClass + 1);
		Cost.BuildTime = 30.0 * ((int32)ShipClass + 1);
		Cost.RequiredShipyardLevel = FMath::Max(1, (int32)ShipClass / 2 + 1);
		BuildCosts.Add(ShipClass, Cost);
	}

	UE_LOG(LogMMOSpace, Log, TEXT("Fleet Subsystem initialized with %d ship classes"), AllClasses.Num());
}

FString UMMOFleetSubsystem::DispatchFleet(const TArray<FFleetUnit>& Units, const FGalaxyCoordinate& Destination, const FString& MissionType)
{
	FFleetData Fleet;
	Fleet.FleetId = GenerateFleetId();
	Fleet.Units = Units;
	Fleet.Destination = Destination;
	Fleet.MissionType = MissionType;
	Fleet.DepartureTime = FPlatformTime::Seconds();
	Fleet.bIsActive = true;
	Fleet.bIsReturning = false;
	Fleet.Speed = GetFleetSpeed(Units);

	int32 TravelTime = CalculateTravelTime(Fleet.Origin, Destination, Fleet.Speed);
	Fleet.ArrivalTime = Fleet.DepartureTime + TravelTime;
	Fleet.ReturnTime = 0;

	ActiveFleets.Add(Fleet);
	OnFleetDispatched.Broadcast(Fleet.FleetId, Destination);

	UE_LOG(LogMMOSpace, Log, TEXT("Fleet %s dispatched to %s"), *Fleet.FleetId, *Destination.ToString());
	return Fleet.FleetId;
}

void UMMOFleetSubsystem::RecallFleet(const FString& FleetId)
{
	for (auto& Fleet : ActiveFleets)
	{
		if (Fleet.FleetId == FleetId && Fleet.bIsActive && !Fleet.bIsReturning)
		{
			Fleet.bIsReturning = true;
			double TimeElapsed = FPlatformTime::Seconds() - Fleet.DepartureTime;
			double TravelTime = Fleet.ArrivalTime - Fleet.DepartureTime;
			double ReturnTravelTime = TravelTime - TimeElapsed;
			Fleet.ReturnTime = FPlatformTime::Seconds() + ReturnTravelTime;
			Fleet.Destination = Fleet.Origin;

			UE_LOG(LogMMOSpace, Log, TEXT("Fleet %s recalled"), *FleetId);
			return;
		}
	}
}

TArray<FFleetData> UMMOFleetSubsystem::GetActiveFleets() const
{
	TArray<FFleetData> Active;
	for (const auto& Fleet : ActiveFleets)
	{
		if (Fleet.bIsActive)
			Active.Add(Fleet);
	}
	return Active;
}

TArray<FFleetData> UMMOFleetSubsystem::GetFleetsAtPosition(const FGalaxyCoordinate& Position) const
{
	TArray<FFleetData> Result;
	for (const auto& Fleet : ActiveFleets)
	{
		if (Fleet.CurrentPosition.Galaxy == Position.Galaxy &&
			Fleet.CurrentPosition.SolarSystem == Position.SolarSystem)
		{
			Result.Add(Fleet);
		}
	}
	return Result;
}

FFleetData UMMOFleetSubsystem::GetFleetById(const FString& FleetId) const
{
	for (const auto& Fleet : ActiveFleets)
	{
		if (Fleet.FleetId == FleetId)
			return Fleet;
	}
	return FFleetData();
}

int32 UMMOFleetSubsystem::CalculateTravelTime(const FGalaxyCoordinate& Origin, const FGalaxyCoordinate& Dest, double Speed) const
{
	int32 BaseDistance = 0;
	if (Origin.Galaxy != Dest.Galaxy)
		BaseDistance += FMath::Abs(Origin.Galaxy - Dest.Galaxy) * 20000;
	BaseDistance += FMath::Abs(Origin.SolarSystem - Dest.SolarSystem) * 95;
	BaseDistance += FMath::Abs(Origin.Position - Dest.Position) * 5;

	double SpeedFactor = FMath::Max(0.1, Speed / 100.0);
	return FMath::Max(1, (int32)(BaseDistance / SpeedFactor));
}

FShipStats UMMOFleetSubsystem::CalculateShipStats(EShipClass ShipClass, int32 Level) const
{
	FShipStats Stats;
	double BaseMult = 1.0 + ((int32)ShipClass * 0.5);
	double LevelMult = 1.0 + (Level * 0.1);

	switch (ShipClass)
	{
	case EShipClass::LightFighter:
		Stats.StructuralIntegrity = 4000 * BaseMult * LevelMult;
		Stats.ShieldPower = 10 * BaseMult * LevelMult;
		Stats.WeaponPower = 50 * BaseMult * LevelMult;
		Stats.Speed = 12500 * LevelMult;
		Stats.CargoCapacity = 50 * BaseMult * LevelMult;
		break;
	case EShipClass::HeavyFighter:
		Stats.StructuralIntegrity = 10000 * BaseMult * LevelMult;
		Stats.ShieldPower = 25 * BaseMult * LevelMult;
		Stats.WeaponPower = 150 * BaseMult * LevelMult;
		Stats.Speed = 10000 * LevelMult;
		Stats.CargoCapacity = 100 * BaseMult * LevelMult;
		break;
	case EShipClass::Battleship:
		Stats.StructuralIntegrity = 60000 * BaseMult * LevelMult;
		Stats.ShieldPower = 200 * BaseMult * LevelMult;
		Stats.WeaponPower = 1000 * BaseMult * LevelMult;
		Stats.Speed = 5000 * LevelMult;
		Stats.CargoCapacity = 1500 * BaseMult * LevelMult;
		break;
	case EShipClass::Dreadnought:
		Stats.StructuralIntegrity = 300000 * BaseMult * LevelMult;
		Stats.ShieldPower = 1000 * BaseMult * LevelMult;
		Stats.WeaponPower = 5000 * BaseMult * LevelMult;
		Stats.Speed = 2500 * LevelMult;
		Stats.CargoCapacity = 10000 * BaseMult * LevelMult;
		break;
	case EShipClass::Titan:
		Stats.StructuralIntegrity = 1000000 * BaseMult * LevelMult;
		Stats.ShieldPower = 5000 * BaseMult * LevelMult;
		Stats.WeaponPower = 25000 * BaseMult * LevelMult;
		Stats.Speed = 1000 * LevelMult;
		Stats.CargoCapacity = 50000 * BaseMult * LevelMult;
		break;
	default:
		Stats.StructuralIntegrity = 20000 * BaseMult * LevelMult;
		Stats.ShieldPower = 50 * BaseMult * LevelMult;
		Stats.WeaponPower = 300 * BaseMult * LevelMult;
		Stats.Speed = 8000 * LevelMult;
		Stats.CargoCapacity = 500 * BaseMult * LevelMult;
		break;
	}

	return Stats;
}

TArray<FShipBuildCost> UMMOFleetSubsystem::GetShipBuildCosts() const
{
	TArray<FShipBuildCost> Costs;
	BuildCosts.GenerateValueArray(Costs);
	return Costs;
}

FShipBuildCost UMMOFleetSubsystem::GetShipBuildCost(EShipClass ShipClass) const
{
	if (const FShipBuildCost* Cost = BuildCosts.Find(ShipClass))
		return *Cost;
	return FShipBuildCost();
}

void UMMOFleetSubsystem::UpdateFleetPositions(float DeltaTime)
{
	double CurrentTime = FPlatformTime::Seconds();
	TArray<FString> CompletedFleets;

	for (auto& Fleet : ActiveFleets)
	{
		if (!Fleet.bIsActive) continue;

		if (!Fleet.bIsReturning && CurrentTime >= Fleet.ArrivalTime)
		{
			OnFleetArrival(Fleet);
			OnFleetArrived.Broadcast(Fleet.FleetId);
		}
		else if (Fleet.bIsReturning && CurrentTime >= Fleet.ReturnTime)
		{
			OnFleetReturn(Fleet);
			OnFleetReturned.Broadcast(Fleet.FleetId);
			CompletedFleets.Add(Fleet.FleetId);
		}
	}

	for (const FString& Id : CompletedFleets)
	{
		ActiveFleets.RemoveAll([&](const FFleetData& F) { return F.FleetId == Id; });
	}
}

int32 UMMOFleetSubsystem::GetTotalFleetCount(const FString& PlayerId) const
{
	int32 Count = 0;
	for (const auto& Fleet : ActiveFleets)
	{
		if (Fleet.OwnerId == PlayerId)
		{
			for (const auto& Unit : Fleet.Units)
				Count += Unit.Count;
		}
	}
	return Count;
}

double UMMOFleetSubsystem::GetFleetSpeed(const TArray<FFleetUnit>& Units) const
{
	double TotalSpeed = 0;
	int32 Count = 0;
	for (const auto& Unit : Units)
	{
		FShipStats Stats = CalculateShipStats(Unit.ShipClass, 1);
		TotalSpeed += Stats.Speed * Unit.Count;
		Count += Unit.Count;
	}
	return Count > 0 ? TotalSpeed / Count : 1.0;
}

void UMMOFleetSubsystem::OnFleetArrival(FFleetData& Fleet)
{
	Fleet.CurrentPosition = Fleet.Destination;
	Fleet.bIsActive = false;
	UE_LOG(LogMMOSpace, Log, TEXT("Fleet %s arrived at destination"), *Fleet.FleetId);
}

void UMMOFleetSubsystem::OnFleetReturn(FFleetData& Fleet)
{
	Fleet.CurrentPosition = Fleet.Origin;
	UE_LOG(LogMMOSpace, Log, TEXT("Fleet %s returned to origin"), *Fleet.FleetId);
}

FString UMMOFleetSubsystem::GenerateFleetId()
{
	return FString::Printf(TEXT("FLEET_%d_%lld"), FleetCounter++, FDateTime::Now().GetTicks());
}

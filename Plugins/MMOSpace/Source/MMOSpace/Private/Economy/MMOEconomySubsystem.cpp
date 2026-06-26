#include "Economy/MMOEconomySubsystem.h"
#include "MMOSpace.h"

void UMMOEconomySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ProductionDefaults = InitializeProductionDefaults();
	UE_LOG(LogMMOSpace, Log, TEXT("Economy Subsystem initialized"));
}

FBuildingProduction UMMOEconomySubsystem::GetBuildingProduction(EBuildingType Type, int32 Level) const
{
	FBuildingProduction Result;
	Result.BuildingType = Type;

	if (const FBuildingProduction* Default = ProductionDefaults.Find(Type))
	{
		double LevelFactor = FMath::Pow(1.1, Level);
		Result.ProductionPerHour.Metal = Default->ProductionPerHour.Metal * Level * LevelFactor;
		Result.ProductionPerHour.Crystal = Default->ProductionPerHour.Crystal * Level * LevelFactor;
		Result.ProductionPerHour.Deuterium = Default->ProductionPerHour.Deuterium * Level * LevelFactor;
		Result.ProductionPerHour.Energy = Default->ProductionPerHour.Energy * Level * LevelFactor;
		Result.EnergyConsumption.Energy = Default->EnergyConsumption.Energy * Level;
	}

	return Result;
}

FBuildingCost UMMOEconomySubsystem::GetBuildingCost(EBuildingType Type, int32 CurrentLevel) const
{
	FBuildingCost Cost;
	Cost.BuildingType = Type;
	Cost.CurrentLevel = CurrentLevel;

	double MetalCost = 0, CrystalCost = 0, DeuteriumCost = 0;

	switch (Type)
	{
	case EBuildingType::MetalMine:
		MetalCost = 60 * FMath::Pow(1.5, CurrentLevel);
		CrystalCost = 15 * FMath::Pow(1.5, CurrentLevel);
		Cost.EnergyCost = 10 * CurrentLevel;
		Cost.ConstructionTime = 30 * FMath::Pow(1.5, CurrentLevel);
		break;
	case EBuildingType::CrystalMine:
		MetalCost = 48 * FMath::Pow(1.6, CurrentLevel);
		CrystalCost = 24 * FMath::Pow(1.6, CurrentLevel);
		Cost.EnergyCost = 10 * CurrentLevel;
		Cost.ConstructionTime = 45 * FMath::Pow(1.6, CurrentLevel);
		break;
	case EBuildingType::DeuteriumSynthesizer:
		MetalCost = 225 * FMath::Pow(1.5, CurrentLevel);
		CrystalCost = 75 * FMath::Pow(1.5, CurrentLevel);
		Cost.EnergyCost = 20 * CurrentLevel;
		Cost.ConstructionTime = 60 * FMath::Pow(1.5, CurrentLevel);
		break;
	case EBuildingType::SolarPlant:
		MetalCost = 75 * FMath::Pow(1.5, CurrentLevel);
		CrystalCost = 30 * FMath::Pow(1.5, CurrentLevel);
		Cost.ConstructionTime = 20 * FMath::Pow(1.5, CurrentLevel);
		break;
	case EBuildingType::FusionReactor:
		MetalCost = 900 * FMath::Pow(1.8, CurrentLevel);
		CrystalCost = 360 * FMath::Pow(1.8, CurrentLevel);
		DeuteriumCost = 180 * FMath::Pow(1.8, CurrentLevel);
		Cost.ConstructionTime = 120 * FMath::Pow(1.8, CurrentLevel);
		break;
	case EBuildingType::RoboticsFactory:
		MetalCost = 400 * FMath::Pow(2.0, CurrentLevel);
		CrystalCost = 120 * FMath::Pow(2.0, CurrentLevel);
		DeuteriumCost = 200 * FMath::Pow(2.0, CurrentLevel);
		Cost.ConstructionTime = 60 * FMath::Pow(2.0, CurrentLevel);
		break;
	case EBuildingType::Shipyard:
		MetalCost = 400 * FMath::Pow(2.0, CurrentLevel);
		CrystalCost = 200 * FMath::Pow(2.0, CurrentLevel);
		DeuteriumCost = 100 * FMath::Pow(2.0, CurrentLevel);
		Cost.ConstructionTime = 80 * FMath::Pow(2.0, CurrentLevel);
		break;
	case EBuildingType::ResearchLab:
		MetalCost = 200 * FMath::Pow(2.0, CurrentLevel);
		CrystalCost = 400 * FMath::Pow(2.0, CurrentLevel);
		DeuteriumCost = 200 * FMath::Pow(2.0, CurrentLevel);
		Cost.ConstructionTime = 60 * FMath::Pow(2.0, CurrentLevel);
		break;
	default:
		MetalCost = 100 * FMath::Pow(2.0, CurrentLevel);
		CrystalCost = 50 * FMath::Pow(2.0, CurrentLevel);
		Cost.ConstructionTime = 30 * FMath::Pow(2.0, CurrentLevel);
		break;
	}

	Cost.Cost.Metal = MetalCost;
	Cost.Cost.Crystal = CrystalCost;
	Cost.Cost.Deuterium = DeuteriumCost;

	return Cost;
}

FResourceSet UMMOEconomySubsystem::GetTotalProduction(const TArray<FBuildingLevel>& Buildings) const
{
	FResourceSet Total;
	for (const auto& Building : Buildings)
	{
		FBuildingProduction Prod = GetBuildingProduction(Building.Type, Building.Level);
		Total.Metal += Prod.ProductionPerHour.Metal;
		Total.Crystal += Prod.ProductionPerHour.Crystal;
		Total.Deuterium += Prod.ProductionPerHour.Deuterium;
		Total.Energy += Prod.ProductionPerHour.Energy;
		Total.Energy -= Prod.EnergyConsumption.Energy;
	}
	return Total;
}

bool UMMOEconomySubsystem::CanAffordUpgrade(const FResourceSet& CurrentResources, const FBuildingCost& Cost) const
{
	return CurrentResources.Metal >= Cost.Cost.Metal &&
		CurrentResources.Crystal >= Cost.Cost.Crystal &&
		CurrentResources.Deuterium >= Cost.Cost.Deuterium;
}

double UMMOEconomySubsystem::CalculateBuildTime(EBuildingType Type, int32 TargetLevel,
	int32 RoboticsLevel, int32 NaniteLevel) const
{
	FBuildingCost Cost = GetBuildingCost(Type, TargetLevel);
	double Time = Cost.ConstructionTime;
	if (RoboticsLevel > 0)
		Time /= (1.0 + RoboticsLevel * 0.1);
	if (NaniteLevel > 0)
		Time /= FMath::Pow(2.0, NaniteLevel);
	return FMath::Max(1.0, Time);
}

void UMMOEconomySubsystem::ProcessTickProduction(FResourceSet& Resources,
	const TArray<FBuildingLevel>& Buildings, double DeltaHours)
{
	FResourceSet Production = GetTotalProduction(Buildings);

	Resources.Metal += Production.Metal * DeltaHours;
	Resources.Crystal += Production.Crystal * DeltaHours;
	Resources.Deuterium += Production.Deuterium * DeltaHours;
	Resources.Energy += Production.Energy * DeltaHours;
}

FString UMMOEconomySubsystem::CreateMarketOrder(const FString& PlayerId, bool bIsBuyOrder,
	EResourceType ResourceOffered, int64 AmountOffered,
	EResourceType ResourceRequested, int64 AmountRequested,
	double PricePerUnit)
{
	FMarketOrder Order;
	Order.OrderId = FString::Printf(TEXT("MKT_%d"), ++MarketOrderCounter);
	Order.PlayerId = PlayerId;
	Order.bIsBuyOrder = bIsBuyOrder;
	Order.ResourceOffered = ResourceOffered;
	Order.AmountOffered = AmountOffered;
	Order.ResourceRequested = ResourceRequested;
	Order.AmountRequested = AmountRequested;
	Order.PricePerUnit = PricePerUnit;
	Order.CreatedAt = FPlatformTime::Seconds();
	Order.bIsActive = true;
	ActiveMarketOrders.Add(Order);
	return Order.OrderId;
}

bool UMMOEconomySubsystem::FillMarketOrder(const FString& OrderId, int64 Amount)
{
	for (auto& Order : ActiveMarketOrders)
	{
		if (Order.OrderId == OrderId && Order.bIsActive)
		{
			if (Amount >= Order.AmountOffered)
			{
				Order.bIsActive = false;
				OnMarketOrderFilled.Broadcast(OrderId, Order.AmountOffered);
				return true;
			}
			Order.AmountOffered -= Amount;
			OnMarketOrderFilled.Broadcast(OrderId, Amount);
			return true;
		}
	}
	return false;
}

void UMMOEconomySubsystem::CancelMarketOrder(const FString& OrderId)
{
	for (auto& Order : ActiveMarketOrders)
	{
		if (Order.OrderId == OrderId)
		{
			Order.bIsActive = false;
			break;
		}
	}
}

TArray<FMarketOrder> UMMOEconomySubsystem::GetActiveOrders(EResourceType Resource) const
{
	TArray<FMarketOrder> Result;
	for (const auto& Order : ActiveMarketOrders)
	{
		if (Order.bIsActive && Order.ResourceOffered == Resource)
			Result.Add(Order);
	}
	return Result;
}

TArray<FMarketOrder> UMMOEconomySubsystem::GetPlayerOrders(const FString& PlayerId) const
{
	TArray<FMarketOrder> Result;
	for (const auto& Order : ActiveMarketOrders)
	{
		if (Order.PlayerId == PlayerId)
			Result.Add(Order);
	}
	return Result;
}

double UMMOEconomySubsystem::GetMarketPrice(EResourceType Resource) const
{
	double TotalPrice = 0;
	int32 Count = 0;
	for (const auto& Order : ActiveMarketOrders)
	{
		if (Order.bIsActive && Order.ResourceOffered == Resource)
		{
			TotalPrice += Order.PricePerUnit;
			Count++;
		}
	}
	return Count > 0 ? TotalPrice / Count : 1.0;
}

bool UMMOEconomySubsystem::CanBuild(EBuildingType Type, int32 CurrentLevel) const
{
	if (CurrentLevel >= 255) return false;
	FBuildingCost Cost = GetBuildingCost(Type, CurrentLevel);
	return Cost.Cost.Metal > 0;
}

double UMMOEconomySubsystem::CalculateEnergyBalance(const TArray<FBuildingLevel>& Buildings) const
{
	double TotalEnergy = 0;
	for (const auto& Building : Buildings)
	{
		FBuildingProduction Prod = GetBuildingProduction(Building.Type, Building.Level);
		TotalEnergy += Prod.ProductionPerHour.Energy - Prod.EnergyConsumption.Energy;
	}
	return TotalEnergy;
}

FString UMMOEconomySubsystem::GetEconomyReport(const TArray<FBuildingLevel>& Buildings) const
{
	FResourceSet Prod = GetTotalProduction(Buildings);
	return FString::Printf(TEXT("Production/h: M=%.0f C=%.0f D=%.0f E=%.0f"),
		Prod.Metal, Prod.Crystal, Prod.Deuterium, Prod.Energy);
}

TMap<EBuildingType, FBuildingProduction> UMMOEconomySubsystem::InitializeProductionDefaults()
{
	TMap<EBuildingType, FBuildingProduction> Defaults;

	FBuildingProduction MetalMine;
	MetalMine.BuildingType = EBuildingType::MetalMine;
	MetalMine.ProductionPerHour.Metal = 30;
	MetalMine.EnergyConsumption.Energy = 10;
	Defaults.Add(EBuildingType::MetalMine, MetalMine);

	FBuildingProduction CrystalMine;
	CrystalMine.BuildingType = EBuildingType::CrystalMine;
	CrystalMine.ProductionPerHour.Crystal = 20;
	CrystalMine.EnergyConsumption.Energy = 10;
	Defaults.Add(EBuildingType::CrystalMine, CrystalMine);

	FBuildingProduction DeutSynth;
	DeutSynth.BuildingType = EBuildingType::DeuteriumSynthesizer;
	DeutSynth.ProductionPerHour.Deuterium = 10;
	DeutSynth.EnergyConsumption.Energy = 20;
	Defaults.Add(EBuildingType::DeuteriumSynthesizer, DeutSynth);

	FBuildingProduction Solar;
	Solar.BuildingType = EBuildingType::SolarPlant;
	Solar.ProductionPerHour.Energy = 20;
	Defaults.Add(EBuildingType::SolarPlant, Solar);

	FBuildingProduction Fusion;
	Fusion.BuildingType = EBuildingType::FusionReactor;
	Fusion.ProductionPerHour.Energy = 50;
	Fusion.EnergyConsumption.Deuterium = 5;
	Defaults.Add(EBuildingType::FusionReactor, Fusion);

	return Defaults;
}

TMap<EBuildingType, double> UMMOEconomySubsystem::CalculateBaseCostMultiplier()
{
	TMap<EBuildingType, double> Multipliers;
	Multipliers.Add(EBuildingType::MetalMine, 1.5);
	Multipliers.Add(EBuildingType::CrystalMine, 1.6);
	Multipliers.Add(EBuildingType::DeuteriumSynthesizer, 1.5);
	Multipliers.Add(EBuildingType::SolarPlant, 1.5);
	Multipliers.Add(EBuildingType::FusionReactor, 1.8);
	Multipliers.Add(EBuildingType::RoboticsFactory, 2.0);
	Multipliers.Add(EBuildingType::Shipyard, 2.0);
	Multipliers.Add(EBuildingType::ResearchLab, 2.0);
	return Multipliers;
}

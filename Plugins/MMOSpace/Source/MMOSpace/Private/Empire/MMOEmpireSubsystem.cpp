#include "Empire/MMOEmpireSubsystem.h"
#include "MMOSpace.h"

void UMMOEmpireSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GovernmentBonuses = InitializeGovernmentBonuses();
	UE_LOG(LogMMOSpace, Log, TEXT("Empire Subsystem initialized"));
}

void UMMOEmpireSubsystem::InitializeEmpire(const FString& PlayerId, const FString& PlanetName)
{
	PlayerEmpireId = PlayerId;
	Progression = FEmpireProgression();
	Resources = FResourceSet();
	Resources.Metal = 50000;
	Resources.Crystal = 50000;
	Resources.Deuterium = 20000;
	Resources.Energy = 5000;
	Resources.Credits = 10000;
	Resources.DarkMatter = 0;
	Progression.Government = EGovernmentType::Democracy;

	UE_LOG(LogMMOSpace, Log, TEXT("Empire initialized for %s at %s"), *PlayerId, *PlanetName);
}

void UMMOEmpireSubsystem::AddExperience(int64 Amount)
{
	Progression.EmpireExperience += Amount;

	while (Progression.EmpireExperience >= GetEmpireLevelUpThreshold())
	{
		Progression.EmpireExperience -= GetEmpireLevelUpThreshold();
		Progression.EmpireLevel++;
		OnEmpireLevelUp.Broadcast(Progression.EmpireLevel);
		UE_LOG(LogMMOSpace, Log, TEXT("Empire leveled up to %d"), Progression.EmpireLevel);
	}
}

void UMMOEmpireSubsystem::SetGovernment(EGovernmentType NewGovernment)
{
	Progression.Government = NewGovernment;
	OnGovernmentChanged.Broadcast(NewGovernment);
	UE_LOG(LogMMOSpace, Log, TEXT("Government changed to %s"), *UEnum::GetValueAsString(NewGovernment));
}

FGovernmentBonus UMMOEmpireSubsystem::GetGovernmentBonus(EGovernmentType GovernmentType) const
{
	if (const FGovernmentBonus* Bonus = GovernmentBonuses.Find(GovernmentType))
		return *Bonus;

	FGovernmentBonus Default;
	Default.Type = GovernmentType;
	Default.BonusDescription = TEXT("Standard governance");
	return Default;
}

void UMMOEmpireSubsystem::AddTurns(int32 Amount)
{
	Progression.CurrentTurns = FMath::Min(Progression.CurrentTurns + Amount, 1000);
	Progression.TotalTurns += Amount;
}

bool UMMOEmpireSubsystem::SpendTurns(int32 Amount)
{
	if (Progression.CurrentTurns >= Amount)
	{
		Progression.CurrentTurns -= Amount;
		Progression.LastTurnUpdate = FPlatformTime::Seconds();
		return true;
	}
	return false;
}

void UMMOEmpireSubsystem::UpdateResourceProduction(const TArray<FBuildingLevel>& Buildings)
{
	ProductionPerHour = CalculateProduction(Buildings);
}

FResourceSet UMMOEmpireSubsystem::CalculateProduction(const TArray<FBuildingLevel>& Buildings) const
{
	FResourceSet Prod;
	for (const auto& Building : Buildings)
	{
		switch (Building.Type)
		{
		case EBuildingType::MetalMine:
			Prod.Metal += 30 * Building.Level * FMath::Pow(1.1, Building.Level);
			Prod.Energy -= 10 * Building.Level;
			break;
		case EBuildingType::CrystalMine:
			Prod.Crystal += 20 * Building.Level * FMath::Pow(1.1, Building.Level);
			Prod.Energy -= 10 * Building.Level;
			break;
		case EBuildingType::DeuteriumSynthesizer:
			Prod.Deuterium += 10 * Building.Level * FMath::Pow(1.1, Building.Level);
			Prod.Energy -= 20 * Building.Level;
			break;
		case EBuildingType::SolarPlant:
			Prod.Energy += 20 * Building.Level * FMath::Pow(1.1, Building.Level);
			break;
		case EBuildingType::FusionReactor:
			Prod.Energy += 50 * Building.Level * FMath::Pow(1.2, Building.Level);
			Prod.Deuterium -= 5 * Building.Level;
			break;
		default:
			break;
		}
	}

	FGovernmentBonus GovBonus = GetGovernmentBonus(Progression.Government);
	Prod.Metal *= GovBonus.ResourceMultiplier;
	Prod.Crystal *= GovBonus.ResourceMultiplier;
	Prod.Deuterium *= GovBonus.ResourceMultiplier;

	if (Prod.Energy < 0)
	{
		double Shortage = FMath::Abs(Prod.Energy);
		double ProductionPenalty = 1.0 - (Shortage / (Prod.Metal + Prod.Crystal + Prod.Deuterium + 1));
		Prod.Metal *= ProductionPenalty;
		Prod.Crystal *= ProductionPenalty;
		Prod.Deuterium *= ProductionPenalty;
	}

	return Prod;
}

void UMMOEmpireSubsystem::SetResources(const FResourceSet& NewResources)
{
	Resources = NewResources;
}

bool UMMOEmpireSubsystem::SpendResources(const FResourceSet& Cost)
{
	if (Resources.Metal >= Cost.Metal && Resources.Crystal >= Cost.Crystal &&
		Resources.Deuterium >= Cost.Deuterium && Resources.Energy >= Cost.Energy &&
		Resources.Credits >= Cost.Credits && Resources.DarkMatter >= Cost.DarkMatter)
	{
		Resources.Metal -= Cost.Metal;
		Resources.Crystal -= Cost.Crystal;
		Resources.Deuterium -= Cost.Deuterium;
		Resources.Energy -= Cost.Energy;
		Resources.Credits -= Cost.Credits;
		Resources.DarkMatter -= Cost.DarkMatter;
		return true;
	}
	return false;
}

FString UMMOEmpireSubsystem::GetEmpireSummary() const
{
	return FString::Printf(TEXT("Empire Level %d | Government: %s | Resources: M=%.0f C=%.0f D=%.0f E=%.0f DM=%.0f"),
		Progression.EmpireLevel, *UEnum::GetValueAsString(Progression.Government),
		Resources.Metal, Resources.Crystal, Resources.Deuterium, Resources.Energy, Resources.DarkMatter);
}

void UMMOEmpireSubsystem::TickResourceGeneration(float DeltaTime)
{
	ResourceTickAccumulator += DeltaTime;
	if (ResourceTickAccumulator >= 1.0f)
	{
		double HoursPassed = ResourceTickAccumulator / 3600.0;
		Resources.Metal += ProductionPerHour.Metal * HoursPassed;
		Resources.Crystal += ProductionPerHour.Crystal * HoursPassed;
		Resources.Deuterium += ProductionPerHour.Deuterium * HoursPassed;
		Resources.Energy += ProductionPerHour.Energy * HoursPassed;
		ResourceTickAccumulator = 0.0f;
		OnResourceTick.Broadcast(Resources);
	}
}

TMap<EGovernmentType, FGovernmentBonus> UMMOEmpireSubsystem::InitializeGovernmentBonuses()
{
	TMap<EGovernmentType, FGovernmentBonus> Bonuses;

	FGovernmentBonus Anarchy;
	Anarchy.Type = EGovernmentType::Anarchy; Anarchy.ResourceMultiplier = 0.5; Anarchy.CombatMultiplier = 1.5; Anarchy.BonusDescription = TEXT("No rules, combat bonus");
	Bonuses.Add(EGovernmentType::Anarchy, Anarchy);

	FGovernmentBonus Autocracy;
	Autocracy.Type = EGovernmentType::Autocracy; Autocracy.ResourceMultiplier = 1.2; Autocracy.ResearchMultiplier = 0.8; Autocracy.BonusDescription = TEXT("Strong resource production, weak research");
	Bonuses.Add(EGovernmentType::Autocracy, Autocracy);

	FGovernmentBonus Democracy;
	Democracy.Type = EGovernmentType::Democracy; Democracy.ResourceMultiplier = 1.0; Democracy.ResearchMultiplier = 1.3; Democracy.DiplomaticMultiplier = 1.5; Democracy.BonusDescription = TEXT("Balanced with research and diplomacy bonus");
	Bonuses.Add(EGovernmentType::Democracy, Democracy);

	FGovernmentBonus Technocracy;
	Technocracy.Type = EGovernmentType::Technocracy; Technocracy.ResearchMultiplier = 2.0; Technocracy.ResourceMultiplier = 0.9; Technocracy.BonusDescription = TEXT("Double research speed");
	Bonuses.Add(EGovernmentType::Technocracy, Technocracy);

	FGovernmentBonus GalacticEmpire;
	GalacticEmpire.Type = EGovernmentType::GalacticEmpire; GalacticEmpire.ResourceMultiplier = 1.5; GalacticEmpire.FleetMultiplier = 1.5; GalacticEmpire.CombatMultiplier = 1.3; GalacticEmpire.BonusDescription = TEXT("Strong all-round bonuses");
	Bonuses.Add(EGovernmentType::GalacticEmpire, GalacticEmpire);

	FGovernmentBonus HiveMind;
	HiveMind.Type = EGovernmentType::HiveMind; HiveMind.ResourceMultiplier = 1.3; HiveMind.FleetMultiplier = 1.8; HiveMind.DiplomaticMultiplier = 0.3; HiveMind.BonusDescription = TEXT("Efficient but poor diplomacy");
	Bonuses.Add(EGovernmentType::HiveMind, HiveMind);

	FGovernmentBonus Corporate;
	Corporate.Type = EGovernmentType::Corporate; Corporate.ResourceMultiplier = 1.8; Corporate.CombatMultiplier = 0.7; Corporate.BonusDescription = TEXT("Economic focus, weak military");
	Bonuses.Add(EGovernmentType::Corporate, Corporate);

	FGovernmentBonus MilitaryJunta;
	MilitaryJunta.Type = EGovernmentType::MilitaryJunta; MilitaryJunta.CombatMultiplier = 2.0; MilitaryJunta.FleetMultiplier = 2.0; MilitaryJunta.ResourceMultiplier = 0.8; MilitaryJunta.ResearchMultiplier = 0.7; MilitaryJunta.BonusDescription = TEXT("Strong military, weak economy");
	Bonuses.Add(EGovernmentType::MilitaryJunta, MilitaryJunta);

	FGovernmentBonus Oligarchy;
	Oligarchy.Type = EGovernmentType::Oligarchy; Oligarchy.ResourceMultiplier = 1.1; Oligarchy.FleetMultiplier = 1.2; Oligarchy.DiplomaticMultiplier = 1.2; Oligarchy.BonusDescription = TEXT("Slight bonuses across the board");
	Bonuses.Add(EGovernmentType::Oligarchy, Oligarchy);

	FGovernmentBonus Theocracy;
	Theocracy.Type = EGovernmentType::Theocracy; Theocracy.ResourceMultiplier = 0.9; Theocracy.ResearchMultiplier = 0.9; Theocracy.DiplomaticMultiplier = 2.0; Theocracy.BonusDescription = TEXT("Diplomatic focus");
	Bonuses.Add(EGovernmentType::Theocracy, Theocracy);

	return Bonuses;
}

int32 UMMOEmpireSubsystem::GetEmpireLevelUpThreshold()
{
	return 1000;
}

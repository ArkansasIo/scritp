#include "Research/MMOResearchSubsystem.h"
#include "MMOSpace.h"

void UMMOResearchSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeResearchTree();
	UE_LOG(LogMMOSpace, Log, TEXT("Research Subsystem initialized with %d techs"), TechTree.Num());
}

void UMMOResearchSubsystem::InitializeResearchTree()
{
	TechTree = GenerateDefaultTechTree();
	ResearchLevels.Empty();
	AccumulatedBonuses.Empty();

	for (EResearchCategory Cat : TEnumRange<EResearchCategory>())
	{
		ResearchLevels.Add(Cat, 0);
	}
}

bool UMMOResearchSubsystem::StartResearch(int32 TechId, const FResearchLabData& LabData)
{
	CurrentLab = LabData;

	for (const auto& QueueItem : ResearchQueue)
	{
		if (QueueItem.TechId == TechId)
			return false;
	}

	FTechDefinition* Tech = TechTree.FindByPredicate([TechId](const FTechDefinition& T) { return T.TechId == TechId; });
	if (!Tech || IsTechCompleted(TechId) || !CheckPrerequisites(*Tech))
		return false;

	if (CurrentLab.QueueSlots <= ResearchQueue.Num())
		return false;

	double ResearchTime = Tech->ResearchTime / (CurrentLab.ResearchSpeedMultiplier * CurrentLab.Level);

	FResearchQueueItem QueueItem;
	QueueItem.TechId = TechId;
	QueueItem.Category = Tech->Category;
	QueueItem.StartTime = FPlatformTime::Seconds();
	QueueItem.EndTime = QueueItem.StartTime + ResearchTime;

	ResearchQueue.Add(QueueItem);
	return true;
}

void UMMOResearchSubsystem::CancelResearch(int32 TechId)
{
	ResearchQueue.RemoveAll([TechId](const FResearchQueueItem& Item) { return Item.TechId == TechId; });
}

void UMMOResearchSubsystem::CompleteResearch(int32 TechId)
{
	CancelResearch(TechId);

	if (!CompletedTechs.Contains(TechId))
	{
		CompletedTechs.Add(TechId);
		FTechDefinition* Tech = TechTree.FindByPredicate([TechId](const FTechDefinition& T) { return T.TechId == TechId; });
		if (Tech)
		{
			int32& Level = ResearchLevels.FindOrAdd(Tech->Category);
			Level++;

			for (const auto& Bonus : Tech->Bonuses)
			{
				AccumulatedBonuses.FindOrAdd(Bonus.Key) += Bonus.Value;
			}
		}
		OnResearchCompleted.Broadcast(Tech->Category);
		OnTechUnlocked.Broadcast(TechId);
	}
}

TArray<FTechDefinition> UMMOResearchSubsystem::GetAvailableTechs() const
{
	TArray<FTechDefinition> Available;
	for (const auto& Tech : TechTree)
	{
		if (!CompletedTechs.Contains(Tech.TechId) && CheckPrerequisites(Tech))
			Available.Add(Tech);
	}
	return Available;
}

TArray<FTechDefinition> UMMOResearchSubsystem::GetCompletedTechs() const
{
	TArray<FTechDefinition> Completed;
	for (const auto& Tech : TechTree)
	{
		if (CompletedTechs.Contains(Tech.TechId))
			Completed.Add(Tech);
	}
	return Completed;
}

TArray<FTechDefinition> UMMOResearchSubsystem::GetTechsByCategory(EResearchCategory Category) const
{
	TArray<FTechDefinition> Result;
	for (const auto& Tech : TechTree)
	{
		if (Tech.Category == Category)
			Result.Add(Tech);
	}
	return Result;
}

bool UMMOResearchSubsystem::IsTechAvailable(int32 TechId) const
{
	if (CompletedTechs.Contains(TechId)) return false;
	const FTechDefinition* Tech = TechTree.FindByPredicate([TechId](const FTechDefinition& T) { return T.TechId == TechId; });
	return Tech && CheckPrerequisites(*Tech);
}

bool UMMOResearchSubsystem::IsTechCompleted(int32 TechId) const
{
	return CompletedTechs.Contains(TechId);
}

FTechDefinition UMMOResearchSubsystem::GetTechDefinition(int32 TechId) const
{
	for (const auto& Tech : TechTree)
	{
		if (Tech.TechId == TechId)
			return Tech;
	}
	return FTechDefinition();
}

double UMMOResearchSubsystem::GetResearchBonus(const FString& BonusKey) const
{
	if (const double* Bonus = AccumulatedBonuses.Find(BonusKey))
		return *Bonus;
	return 0.0;
}

int32 UMMOResearchSubsystem::GetResearchLevel(EResearchCategory Category) const
{
	if (const int32* Level = ResearchLevels.Find(Category))
		return *Level;
	return 0;
}

void UMMOResearchSubsystem::UpdateResearchProgress(float DeltaTime)
{
	double CurrentTime = FPlatformTime::Seconds();
	TArray<int32> CompletedItems;

	for (const auto& Item : ResearchQueue)
	{
		if (CurrentTime >= Item.EndTime)
			CompletedItems.Add(Item.TechId);
	}

	for (int32 TechId : CompletedItems)
	{
		CompleteResearch(TechId);
	}
}

void UMMOResearchSubsystem::SpeedUpResearch(int32 TechId, double GamerAccelerateCount)
{
	for (auto& Item : ResearchQueue)
	{
		if (Item.TechId == TechId)
		{
			Item.EndTime -= GamerAccelerateCount;
			break;
		}
	}
}

bool UMMOResearchSubsystem::CheckPrerequisites(const FTechDefinition& Tech) const
{
	for (int32 PrereqId : Tech.Prerequisites)
	{
		if (!CompletedTechs.Contains(PrereqId))
			return false;
	}
	return true;
}

TArray<FTechDefinition> UMMOResearchSubsystem::GenerateDefaultTechTree()
{
	TArray<FTechDefinition> Tree;
	int32 TechId = 1;

	auto AddTech = [&](const FString& Name, EResearchCategory Cat, int32 Tier,
		TArray<int32> Prereqs, double Metal, double Crystal, double Deuterium, double Time,
		TMap<FString, double> Bonuses)
	{
		FTechDefinition Tech;
		Tech.TechId = TechId++;
		Tech.TechName = Name;
		Tech.Category = Cat;
		Tech.Tier = Tier;
		Tech.Prerequisites = Prereqs;
		Tech.ResearchCost.Metal = Metal;
		Tech.ResearchCost.Crystal = Crystal;
		Tech.ResearchCost.Deuterium = Deuterium;
		Tech.ResearchTime = Time;
		Tech.Bonuses = Bonuses;
		Tree.Add(Tech);
	};

	// Tier 1 - Basic Weapons
	AddTech(TEXT("Laser Technology"), EResearchCategory::Lasers, 1, {},
		200, 100, 50, 60, { {TEXT("weapon_damage"), 1.1} });

	// Tier 1 - Basic Energy
	AddTech(TEXT("Energy Technology"), EResearchCategory::Energy, 1, {},
		0, 800, 400, 45, { {TEXT("energy_production"), 1.1} });

	// Tier 1 - Basic Propulsion
	AddTech(TEXT("Combustion Drive"), EResearchCategory::Combustion, 1, {},
		400, 200, 100, 30, { {TEXT("ship_speed"), 1.1} });

	// Tier 2 - Advanced Weapons
	AddTech(TEXT("Ion Technology"), EResearchCategory::Ion, 2, { 1 },
		1000, 500, 200, 120, { {TEXT("weapon_damage"), 1.15} });

	// Tier 2 - Shielding
	AddTech(TEXT("Shielding Technology"), EResearchCategory::Shielding, 2, { 2 },
		200, 600, 200, 90, { {TEXT("shield_power"), 1.1} });

	// Tier 2 - Armor
	AddTech(TEXT("Armor Technology"), EResearchCategory::Armor, 2, { 2 },
		500, 300, 100, 60, { {TEXT("armor_strength"), 1.1} });

	// Tier 2 - Espionage
	AddTech(TEXT("Espionage Technology"), EResearchCategory::Espionage, 2, { 2 },
		200, 1000, 200, 80, { {TEXT("espionage_range"), 1.0} });

	// Tier 3 - Plasma Weapons
	AddTech(TEXT("Plasma Technology"), EResearchCategory::Plasma, 3, { 3, 4 },
		2000, 1000, 500, 240, { {TEXT("weapon_damage"), 1.2} });

	// Tier 3 - Hyperspace
	AddTech(TEXT("Hyperspace Technology"), EResearchCategory::Hyperspace, 3, { 5, 6 },
		4000, 2000, 1000, 180, { {TEXT("ship_speed"), 1.3} });

	// Tier 3 - Impulse Drive
	AddTech(TEXT("Impulse Drive"), EResearchCategory::Impulse, 3, { 3 },
		2000, 4000, 600, 120, { {TEXT("ship_speed"), 1.2} });

	// Tier 4 - Weapons
	AddTech(TEXT("Weapons Technology"), EResearchCategory::Weapons, 4, { 7 },
		5000, 2500, 1000, 360, { {TEXT("weapon_damage"), 1.3} });

	// Tier 4 - Colonization
	AddTech(TEXT("Colonization Technology"), EResearchCategory::Colonization, 4, { 9 },
		10000, 5000, 2500, 480, { {TEXT("colony_slots"), 1.0} });

	// Tier 4 - Economics
	AddTech(TEXT("Economics"), EResearchCategory::Economics, 4, { 2 },
		3000, 1500, 500, 240, { {TEXT("resource_production"), 1.15} });

	// Tier 5 - Warfare
	AddTech(TEXT("Warfare Technology"), EResearchCategory::Warfare, 5, { 11, 8 },
		15000, 7500, 3000, 720, { {TEXT("combat_rating"), 1.25} });

	// Tier 5 - Propulsion
	AddTech(TEXT("Hyperspace Drive"), EResearchCategory::Propulsion, 5, { 9 },
		20000, 10000, 5000, 600, { {TEXT("ship_speed"), 1.5} });

	// Advanced Laser (continues Laser tree)
	AddTech(TEXT("Advanced Laser Technology"), EResearchCategory::Lasers, 2, { 1 },
		500, 250, 100, 90, { {TEXT("weapon_damage"), 1.15} });

	// High Energy
	AddTech(TEXT("High Energy Technology"), EResearchCategory::Energy, 2, { 2 },
		400, 1600, 800, 90, { {TEXT("energy_production"), 1.2} });

	// Combat Lasers
	AddTech(TEXT("Combat Lasers"), EResearchCategory::Lasers, 3, { 1, 16 },
		1500, 750, 300, 180, { {TEXT("weapon_damage"), 1.2} });

	return Tree;
}

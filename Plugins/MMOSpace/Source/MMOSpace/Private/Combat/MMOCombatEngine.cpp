#include "Combat/MMOCombatEngine.h"
#include "MMOSpace.h"
#include "Fleet/MMOFleetSubsystem.h"
#include "Misc/DateTime.h"
#include "Engine/GameInstance.h"

void UMMOCombatEngine::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogMMOSpace, Log, TEXT("Combat Engine initialized"));
}

FString UMMOCombatEngine::StartBattle(const TArray<FFleetUnit>& AttackerFleet, const TArray<FFleetUnit>& DefenderFleet,
	const FString& AttackerId, const FString& DefenderId,
	EFleetFormation AttackerFormation, EFleetFormation DefenderFormation)
{
	FString BattleId = GenerateBattleId();
	LastResult = FCombatResult();
	LastResult.BattleId = BattleId;
	LastResult.AttackerId = AttackerId;
	LastResult.DefenderId = DefenderId;

	TArray<FFleetData> Participants;
	FFleetData AtkData, DefData;
	AtkData.FleetId = TEXT("Attacker");
	AtkData.OwnerId = AttackerId;
	AtkData.Units = AttackerFleet;
	AtkData.Formation = AttackerFormation;
	DefData.FleetId = TEXT("Defender");
	DefData.OwnerId = DefenderId;
	DefData.Units = DefenderFleet;
	DefData.Formation = DefenderFormation;
	Participants.Add(AtkData);
	Participants.Add(DefData);

	OnBattleStarted.Broadcast(BattleId, Participants);
	bSimulating = true;
	SimulateCombat(LastResult);
	bSimulating = false;
	OnBattleEnded.Broadcast(BattleId);

	return BattleId;
}

void UMMOCombatEngine::SimulateCombat(FCombatResult& Result)
{
	UMMOFleetSubsystem* FleetSub = GetWorld()->GetGameInstance()->GetSubsystem<UMMOFleetSubsystem>();
	TArray<FFleetUnit> Attacker, Defender;
	EFleetFormation AttackForm = EFleetFormation::LineAbreast;
	EFleetFormation DefendForm = EFleetFormation::LineAbreast;

	if (FleetSub)
	{
		FFleetData AtkData = FleetSub->GetFleetById(Result.AttackerId);
		FFleetData DefData = FleetSub->GetFleetById(Result.DefenderId);
		Attacker = AtkData.Units;
		Defender = DefData.Units;
		AttackForm = AtkData.Formation;
		DefendForm = DefData.Formation;

		for (const auto& F : FleetSub->GetActiveFleets())
		{
			if (F.OwnerId == Result.AttackerId) AttackForm = F.Formation;
			if (F.OwnerId == Result.DefenderId) DefendForm = F.Formation;
		}
	}

	int32 MaxRounds = 6;
	for (int32 Round = 1; Round <= MaxRounds; Round++)
	{
		FCombatRoundResult RoundResult = SimulateRound(Attacker, Defender, AttackForm, DefendForm);
		Result.RoundResults.Add(RoundResult);
		Result.TotalRounds = Round;

		TArray<FString> DamageLog;
		DamageLog.Add(FString::Printf(TEXT("Round %d: Attacker %d ships, Defender %d ships"),
			Round, RoundResult.AttackerRemaining.Num(), RoundResult.DefenderRemaining.Num()));

		OnBattleRound.Broadcast(Round, DamageLog, RoundResult.AttackerRemaining);

		Attacker = RoundResult.AttackerRemaining;
		Defender = RoundResult.DefenderRemaining;

		int32 AttackerCount = 0, DefenderCount = 0;
		for (const auto& U : Attacker) AttackerCount += U.Count;
		for (const auto& U : Defender) DefenderCount += U.Count;

		if (AttackerCount == 0 || DefenderCount == 0)
		{
			Result.bAttackerWon = (DefenderCount == 0);
			break;
		}
	}

	Result.TotalLoot = Result.RoundResults.Num() > 0 ? Result.RoundResults.Last().LootCaptured : TArray<FResourceSet>();
	Result.WinningFormation = Result.bAttackerWon ? AttackForm : DefendForm;

	for (const auto& RR : Result.RoundResults)
	{
		for (const auto& Unit : RR.DefenderLosses)
		{
			Result.DebrisMetal += Unit.Stats.StructuralIntegrity * 0.3 * Unit.Count;
			Result.DebrisCrystal += Unit.Stats.StructuralIntegrity * 0.15 * Unit.Count;
		}
	}

	LastResult = Result;
}

FCombatRoundResult UMMOCombatEngine::SimulateRound(TArray<FFleetUnit>& Attacker, TArray<FFleetUnit>& Defender,
	EFleetFormation AttackForm, EFleetFormation DefendForm)
{
	FCombatRoundResult Result;
	double AttackerFormBonus = CalculateFormationBonus(AttackForm, true);
	double DefenderFormBonus = CalculateFormationBonus(DefendForm, false);

	for (auto& AttUnit : Attacker)
	{
		if (AttUnit.Count <= 0) continue;

		int32 TotalDamage = 0;
		for (int32 i = 0; i < AttUnit.Count; i++)
		{
			for (auto& DefUnit : Defender)
			{
				if (DefUnit.Count <= 0) continue;
				double HitChance = CalculateHitChance(AttUnit.ShipClass, DefUnit.ShipClass, 1.0);
				if (FMath::FRand() < HitChance)
				{
					int32 Dmg = CalculateDamage(AttUnit.Stats, DefUnit.Stats, AttackerFormBonus);
					TotalDamage += Dmg;
				}
			}
		}
	}

	for (auto& DefUnit : Defender)
	{
		if (DefUnit.Count <= 0) continue;

		int32 TotalDamage = 0;
		for (int32 i = 0; i < DefUnit.Count; i++)
		{
			for (auto& AttUnit : Attacker)
			{
				if (AttUnit.Count <= 0) continue;
				double HitChance = CalculateHitChance(DefUnit.ShipClass, AttUnit.ShipClass, 1.0);
				if (FMath::FRand() < HitChance)
				{
					int32 Dmg = CalculateDamage(DefUnit.Stats, AttUnit.Stats, DefenderFormBonus);
					TotalDamage += Dmg;
				}
			}
		}
	}

	double ExpRatio = CalculateExplosiveRatio(Defender);
	Result.DefenderLosses = CalculateLosses(Defender, ExpRatio);
	Result.AttackerLosses = CalculateLosses(Attacker, ExpRatio);
	Result.ExplosiveRatio = ExpRatio;

	for (auto& A : Attacker)
	{
		for (auto& Loss : Result.AttackerLosses)
		{
			if (A.ShipClass == Loss.ShipClass)
				A.Count = FMath::Max(0, A.Count - Loss.Count);
		}
		if (A.Count > 0) Result.AttackerRemaining.Add(A);
	}

	for (auto& D : Defender)
	{
		for (auto& Loss : Result.DefenderLosses)
		{
			if (D.ShipClass == Loss.ShipClass)
				D.Count = FMath::Max(0, D.Count - Loss.Count);
		}
		if (D.Count > 0) Result.DefenderRemaining.Add(D);
	}

	return Result;
}

double UMMOCombatEngine::CalculateFormationBonus(EFleetFormation Formation, bool bIsAttacker) const
{
	if (bIsAttacker)
		return GetFormationDamageBonus(Formation);
	return GetFormationDefenseBonus(Formation);
}

int32 UMMOCombatEngine::CalculateDamage(const FShipStats& Attacker, const FShipStats& Defender, double FormationBonus) const
{
	double RawDamage = Attacker.WeaponPower * FormationBonus;
	double EffectiveShield = Defender.ShieldPower * 0.7;
	double DamageAfterShield = FMath::Max(0.0, RawDamage - EffectiveShield);
	return FMath::Max(1, (int32)(DamageAfterShield / FMath::Max(1.0, Defender.StructuralIntegrity * 0.01)));
}

double UMMOCombatEngine::CalculateHitChance(EShipClass AttackerClass, EShipClass DefenderClass, double TechLevel) const
{
	double BaseChance = 0.7;
	int32 ClassDiff = (int32)DefenderClass - (int32)AttackerClass;
	double ClassModifier = 1.0 + (ClassDiff * 0.05);
	return FMath::Clamp(BaseChance * ClassModifier * TechLevel, 0.1, 0.99);
}

TArray<FFleetUnit> UMMOCombatEngine::CalculateLosses(const TArray<FFleetUnit>& Fleet, double ExplosiveRatio) const
{
	TArray<FFleetUnit> Losses;
	for (const auto& Unit : Fleet)
	{
		if (Unit.Count <= 0) continue;
		FFleetUnit Loss;
		Loss.ShipClass = Unit.ShipClass;
		Loss.Stats = Unit.Stats;
		Loss.Count = FMath::Max(1, (int32)(Unit.Count * ExplosiveRatio * FMath::FRand()));
		if (Loss.Count > 0)
			Losses.Add(Loss);
	}
	return Losses;
}

double UMMOCombatEngine::GetWeaponRating(EShipClass ShipClass)
{
	switch (ShipClass)
	{
	case EShipClass::LightFighter: return 1.0;
	case EShipClass::HeavyFighter: return 2.0;
	case EShipClass::Corvette: return 3.0;
	case EShipClass::Frigate: return 4.0;
	case EShipClass::Destroyer: return 6.0;
	case EShipClass::Battleship: return 8.0;
	case EShipClass::Carrier: return 10.0;
	case EShipClass::Dreadnought: return 15.0;
	case EShipClass::Titan: return 25.0;
	case EShipClass::AscendedWarship: return 50.0;
	default: return 1.0;
	}
}

double UMMOCombatEngine::GetArmorRating(EShipClass ShipClass)
{
	switch (ShipClass)
	{
	case EShipClass::LightFighter: return 1.0;
	case EShipClass::HeavyFighter: return 2.0;
	case EShipClass::Corvette: return 3.0;
	case EShipClass::Frigate: return 5.0;
	case EShipClass::Destroyer: return 7.0;
	case EShipClass::Battleship: return 10.0;
	case EShipClass::Carrier: return 12.0;
	case EShipClass::Dreadnought: return 20.0;
	case EShipClass::Titan: return 35.0;
	case EShipClass::AscendedWarship: return 60.0;
	default: return 1.0;
	}
}

double UMMOCombatEngine::GetShieldRating(EShipClass ShipClass)
{
	switch (ShipClass)
	{
	case EShipClass::LightFighter: return 0.5;
	case EShipClass::HeavyFighter: return 1.0;
	case EShipClass::Corvette: return 2.0;
	case EShipClass::Frigate: return 3.0;
	case EShipClass::Destroyer: return 5.0;
	case EShipClass::Battleship: return 8.0;
	case EShipClass::Carrier: return 10.0;
	case EShipClass::Dreadnought: return 15.0;
	case EShipClass::Titan: return 25.0;
	case EShipClass::AscendedWarship: return 50.0;
	default: return 0.5;
	}
}

double UMMOCombatEngine::GetFormationDamageBonus(EFleetFormation Formation)
{
	switch (Formation)
	{
	case EFleetFormation::Wedge: return 1.5;
	case EFleetFormation::Vanguard: return 1.3;
	case EFleetFormation::Pincer: return 1.4;
	case EFleetFormation::LineAbreast: return 1.2;
	case EFleetFormation::ShieldWall: return 0.8;
	default: return 1.0;
	}
}

double UMMOCombatEngine::GetFormationDefenseBonus(EFleetFormation Formation)
{
	switch (Formation)
	{
	case EFleetFormation::ShieldWall: return 2.0;
	case EFleetFormation::LineAbreast: return 1.3;
	case EFleetFormation::Vanguard: return 1.1;
	case EFleetFormation::Wedge: return 0.8;
	case EFleetFormation::Pincer: return 0.9;
	default: return 1.0;
	}
}

double UMMOCombatEngine::GetFormationSpeedBonus(EFleetFormation Formation)
{
	switch (Formation)
	{
	case EFleetFormation::Vanguard: return 1.5;
	case EFleetFormation::Wedge: return 1.3;
	case EFleetFormation::Pincer: return 1.2;
	case EFleetFormation::LineAbreast: return 1.0;
	case EFleetFormation::ShieldWall: return 0.7;
	default: return 1.0;
	}
}

FString UMMOCombatEngine::GenerateBattleId()
{
	return FString::Printf(TEXT("BATTLE_%d_%lld"), BattleCounter++, FDateTime::Now().GetTicks());
}

double UMMOCombatEngine::CalculateExplosiveRatio(const TArray<FFleetUnit>& Fleet) const
{
	int32 TotalShips = 0;
	for (const auto& Unit : Fleet) TotalShips += Unit.Count;
	double Ratio = 0.3 + (FMath::FRand() * 0.4);
	return FMath::Clamp(Ratio, 0.0, 1.0);
}

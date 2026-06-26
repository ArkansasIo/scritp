#include "Abilities/MMODamageSystem.h"
#include "Characters/MMOCharacterBase.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

FMMODamageResult UMMODamageSystem::CalculateDamage(
	AMMOCharacterBase* Attacker,
	AMMOCharacterBase* Defender,
	float BaseDamage,
	EMMODamageType DamageType,
	const FGameplayTagContainer& DamageTags,
	bool bCanCrit,
	bool bCanBeMitigated)
{
	FMMODamageResult Result;
	Result.RawDamage = BaseDamage;
	Result.DamageType = DamageType;
	Result.DamageTags = DamageTags;
	Result.Instigator = Attacker;
	Result.Causer = Attacker;

	if (!Attacker || !Defender)
	{
		Result.FinalDamage = BaseDamage;
		return Result;
	}

	FMMOAttackAttributes AttackStats = GetAttackAttributes(Attacker);
	FMMODefenseAttributes DefenseStats = GetDefenseAttributes(Defender);

	// Determine hit result
	Result.HitResult = DetermineHitResult(AttackStats, DefenseStats);

	// Handle miss / dodge / parry / immune
	switch (Result.HitResult)
	{
	case EMMOHitResult::Missed:
		Result.FinalDamage = 0.0f;
		Result.ThreatGenerated = CalculateThreatFromDamage(0.0f, Result.HitResult);
		return Result;
	case EMMOHitResult::Dodged:
	case EMMOHitResult::Parried:
	case EMMOHitResult::Immune:
		Result.FinalDamage = 0.0f;
		Result.ThreatGenerated = CalculateThreatFromDamage(0.0f, Result.HitResult);
		return Result;
	case EMMOHitResult::Absorbed:
		Result.FinalDamage = 0.0f;
		Result.AppliedMitigations.Add(EMMODamageMitigation::Absorption);
		Result.ThreatGenerated = CalculateThreatFromDamage(BaseDamage, Result.HitResult);
		return Result;
	default:
		break;
	}

	// Mitigation
	float MitigatedDamage = BaseDamage;
	if (bCanBeMitigated)
	{
		if (DamageType == EMMODamageType::Physical)
		{
			const float ArmorMitigation = CalculateArmorMitigation(DefenseStats.Armor, Attacker->GetCharacterLevel(), Defender->GetCharacterLevel());
			const float ArmorPenMultiplier = 1.0f - (AttackStats.ArmorPenetration / 100.0f);
			const float EffectiveArmorMitigation = FMath::Max(0.0f, ArmorMitigation * ArmorPenMultiplier);
			MitigatedDamage *= (1.0f - EffectiveArmorMitigation / 100.0f);
			if (EffectiveArmorMitigation > 0.0f)
			{
				Result.AppliedMitigations.Add(EMMODamageMitigation::Armor);
			}
		}
		else if (DamageType != EMMODamageType::TrueDamage)
		{
			const float ResMitigation = CalculateResistanceMitigation(DefenseStats.MagicResistance, Attacker->GetCharacterLevel());
			const float MagicPenMultiplier = 1.0f - (AttackStats.MagicPenetration / 100.0f);
			const float EffectiveResMitigation = FMath::Max(0.0f, ResMitigation * MagicPenMultiplier);
			MitigatedDamage *= (1.0f - EffectiveResMitigation / 100.0f);
			if (EffectiveResMitigation > 0.0f)
			{
				Result.AppliedMitigations.Add(EMMODamageMitigation::MagicResistance);
			}
		}

		// Absorption shield
		if (DefenseStats.AbsorptionShield > 0.0f)
		{
			const float Absorbed = FMath::Min(MitigatedDamage, DefenseStats.AbsorptionShield);
			MitigatedDamage -= Absorbed;
			if (Absorbed > 0.0f)
			{
				Result.AppliedMitigations.Add(EMMODamageMitigation::Absorption);
			}
		}
	}
	Result.MitigatedDamage = BaseDamage - MitigatedDamage;

	// Critical hit
	float FinalDamage = MitigatedDamage;
	if (bCanCrit && Result.HitResult == EMMOHitResult::Critical)
	{
		const float CritMult = CalculateCritMultiplier(AttackStats.CritMultiplier, DefenseStats.Resilience);
		FinalDamage *= CritMult;
	}
	else if (Result.HitResult == EMMOHitResult::Glancing)
	{
		FinalDamage *= 0.75f;
	}
	else if (Result.HitResult == EMMOHitResult::Crushing)
	{
		FinalDamage *= 1.3f;
	}
	else if (Result.HitResult == EMMOHitResult::Blocked)
	{
		FinalDamage = FMath::Max(0.0f, FinalDamage - DefenseStats.BlockValue);
	}

	Result.FinalDamage = ClampDamage(FinalDamage);
	Result.OverkillDamage = FMath::Max(0.0f, Result.FinalDamage - Defender->GetHealth());
	Result.ThreatGenerated = CalculateThreatFromDamage(Result.FinalDamage, Result.HitResult);

	return Result;
}

FMMODamageResult UMMODamageSystem::CalculateHealing(
	AMMOCharacterBase* Healer,
	AMMOCharacterBase* Target,
	float BaseHealing,
	const FGameplayTagContainer& HealingTags,
	bool bCanCrit)
{
	FMMODamageResult Result;
	Result.RawHealing = BaseHealing;
	Result.DamageTags = HealingTags;
	Result.Instigator = Healer;
	Result.Causer = Healer;

	if (!Healer || !Target)
	{
		Result.FinalHealing = BaseHealing;
		return Result;
	}

	FMMOAttackAttributes AttackStats = GetAttackAttributes(Healer);

	float FinalHealing = BaseHealing;

	if (bCanCrit && RollCrit(AttackStats.CritChance, 0.0f))
	{
		Result.HitResult = EMMOHitResult::Critical;
		FinalHealing *= AttackStats.CritMultiplier;
	}
	else
	{
		Result.HitResult = EMMOHitResult::Normal;
	}

	const float TargetMaxHealth = Target->GetMaxHealth();
	const float TargetCurrentHealth = Target->GetHealth();
	const float MaxPossibleHeal = TargetMaxHealth - TargetCurrentHealth;
	const float ActualHeal = FMath::Min(FinalHealing, MaxPossibleHeal);

	Result.FinalHealing = ActualHeal;
	Result.Overheal = FMath::Max(0.0f, FinalHealing - ActualHeal);
	Result.ThreatGenerated = CalculateThreatFromHealing(ActualHeal);

	return Result;
}

EMMOHitResult UMMODamageSystem::DetermineHitResult(
	const FMMOAttackAttributes& AttackerStats,
	const FMMODefenseAttributes& DefenderStats)
{
	const float EffectiveHitChance = FMath::Clamp(AttackerStats.HitChance - DefenderStats.DefenseRating * 0.01f, 0.0f, 100.0f);
	const float Roll = FMath::FRandRange(0.0f, 100.0f);

	// Check miss
	if (Roll > EffectiveHitChance)
	{
		return EMMOHitResult::Missed;
	}

	// Check dodge
	if (Roll < DefenderStats.DodgeChance)
	{
		return EMMOHitResult::Dodged;
	}

	// Check parry (only for melee/phys from front)
	if (Roll < DefenderStats.DodgeChance + DefenderStats.ParryChance)
	{
		return EMMOHitResult::Parried;
	}

	// Check block
	if (Roll < DefenderStats.DodgeChance + DefenderStats.ParryChance + DefenderStats.BlockChance)
	{
		return EMMOHitResult::Blocked;
	}

	// Check glancing (when defender level > attacker level)
	if (Roll < EffectiveHitChance - 10.0f)
	{
		return EMMOHitResult::Glancing;
	}

	// Check crushing (when attacker level > defender level)
	if (Roll > EffectiveHitChance - 5.0f && Roll <= EffectiveHitChance)
	{
		return EMMOHitResult::Crushing;
	}

	// Check critical
	if (Roll < AttackerStats.CritChance - DefenderStats.CritAvoidance)
	{
		return EMMOHitResult::Critical;
	}

	return EMMOHitResult::Normal;
}

float UMMODamageSystem::CalculateArmorMitigation(float Armor, float AttackerLevel, float DefenderLevel)
{
	if (Armor <= 0.0f)
	{
		return 0.0f;
	}
	// Standard formula: Damage reduction = Armor / (Armor + 400 + 85 * AttackerLevel)
	const float Denom = Armor + 400.0f + 85.0f * AttackerLevel;
	return (Armor / Denom) * 100.0f;
}

float UMMODamageSystem::CalculateResistanceMitigation(float Resistance, float AttackerLevel)
{
	if (Resistance <= 0.0f)
	{
		return 0.0f;
	}
	const float Denom = Resistance + 400.0f + 85.0f * AttackerLevel;
	return (Resistance / Denom) * 100.0f;
}

float UMMODamageSystem::CalculateCritMultiplier(float BaseCritMultiplier, float Resilience)
{
	const float ResilienceReduction = Resilience * 0.01f;
	return FMath::Max(1.0f, BaseCritMultiplier - ResilienceReduction);
}

float UMMODamageSystem::CalculateThreatFromDamage(float Damage, EMMOHitResult HitResult, float ThreatMultiplier)
{
	float BaseThreat = Damage * ThreatMultiplier;
	switch (HitResult)
	{
	case EMMOHitResult::Critical:
		BaseThreat *= 1.5f;
		break;
	case EMMOHitResult::Crushing:
		BaseThreat *= 1.25f;
		break;
	case EMMOHitResult::Glancing:
		BaseThreat *= 0.75f;
		break;
	default:
		break;
	}
	return BaseThreat;
}

float UMMODamageSystem::CalculateThreatFromHealing(float Healing, float ThreatMultiplier)
{
	return Healing * ThreatMultiplier;
}

void UMMODamageSystem::ApplyPeriodicDamage(
	AMMOCharacterBase* Target,
	const FMMODoTData& DoTData,
	AActor* Instigator)
{
	if (!Target)
	{
		return;
	}

	UWorld* World = Target->GetWorld();
	if (!World)
	{
		return;
	}

	FTimerDelegate TickDelegate;
	FTimerHandle DotTimerHandle;
	int32 TicksRemaining = DoTData.MaxTicks;

	TickDelegate.BindLambda([Target, DoTData, Instigator, &TicksRemaining]()
	{
		if (!Target || TicksRemaining <= 0)
		{
			return;
		}

		float TickDamage = DoTData.BaseDamagePerTick;
		if (DoTData.bScalesWithAttackPower)
		{
			if (AMMOCharacterBase* CharInstigator = Cast<AMMOCharacterBase>(Instigator))
			{
				TickDamage += CharInstigator->GetAttackPower() * DoTData.ScalingFactor;
			}
		}

		FMMODamageResult DotResult = CalculateDamage(
			Cast<AMMOCharacterBase>(Instigator),
			Target,
			TickDamage,
			DoTData.DamageType,
			DoTData.DoTTags,
			false,
			true
		);
		DotResult.bIsPeriodic = true;

		Target->ReceiveDamage(DotResult);
		TicksRemaining--;
	});

	World->GetTimerManager().SetTimer(DotTimerHandle, TickDelegate, DoTData.TickInterval, true, 0.0f);

	// Auto-clear the timer when DoT ends
	FTimerHandle CleanupHandle;
	World->GetTimerManager().SetTimer(CleanupHandle, FTimerDelegate::CreateLambda([World, DotTimerHandle]()
	{
		World->GetTimerManager().ClearTimer(DotTimerHandle);
	}), DoTData.TotalDuration, false);
}

bool UMMODamageSystem::RollCrit(float CritChance, float CritAvoidance)
{
	const float EffectiveCrit = FMath::Max(0.0f, CritChance - CritAvoidance);
	return FMath::FRandRange(0.0f, 100.0f) < EffectiveCrit;
}

float UMMODamageSystem::ClampDamage(float Damage)
{
	return FMath::Max(1.0f, Damage);
}

FMMOAttackAttributes UMMODamageSystem::GetAttackAttributes(AMMOCharacterBase* Character)
{
	FMMOAttackAttributes Attrs;
	if (!Character)
	{
		return Attrs;
	}

	Attrs.AttackPower = Character->GetAttackPower();
	Attrs.SpellPower = Character->GetSpellPower();
	Attrs.CritChance = Character->GetCritChance();
	Attrs.CritMultiplier = Character->GetCritMultiplier();
	Attrs.HitChance = Character->GetHitChance();
	Attrs.Expertise = Character->GetExpertise();
	Attrs.ArmorPenetration = Character->GetArmorPenetration();
	Attrs.MagicPenetration = Character->GetMagicPenetration();
	Attrs.HastePercent = Character->GetHastePercent();

	return Attrs;
}

FMMODefenseAttributes UMMODamageSystem::GetDefenseAttributes(AMMOCharacterBase* Character)
{
	FMMODefenseAttributes Attrs;
	if (!Character)
	{
		return Attrs;
	}

	Attrs.Armor = Character->GetArmor();
	Attrs.MagicResistance = Character->GetMagicResistance();
	Attrs.DodgeChance = Character->GetDodgeChance();
	Attrs.ParryChance = Character->GetParryChance();
	Attrs.BlockChance = Character->GetBlockChance();
	Attrs.BlockValue = Character->GetBlockValue();
	Attrs.CritAvoidance = Character->GetCritAvoidance();
	Attrs.Resilience = Character->GetResilience();
	Attrs.DefenseRating = Character->GetDefenseRating();
	Attrs.AbsorptionShield = Character->GetAbsorptionShield();
	Attrs.ImmunityTags = Character->GetImmunityTags();

	return Attrs;
}

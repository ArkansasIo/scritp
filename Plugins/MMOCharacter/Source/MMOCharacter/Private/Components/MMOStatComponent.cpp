#include "Components/MMOStatComponent.h"
#include "Characters/MMOCharacterBase.h"
#include "Data/MMOCharacterData.h"
#include "Data/MMOLevelData.h"
#include "Net/UnrealNetwork.h"
#include "Engine/CurveTable.h"

UMMOStatComponent::UMMOStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UMMOStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMMOStatComponent, BasePrimaryStats, COND_Always);
	DOREPLIFETIME_CONDITION(UMMOStatComponent, BonusPrimaryStats, COND_OwnerOnly);
}

void UMMOStatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CalculateSecondaryStats();
	}
}

void UMMOStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

int32 UMMOStatComponent::GetEffectivePrimaryStat(EMMOPrimaryStat StatType) const
{
	switch (StatType)
	{
	case EMMOPrimaryStat::Strength:
		return BasePrimaryStats.Strength + BonusPrimaryStats.Strength;
	case EMMOPrimaryStat::Dexterity:
		return BasePrimaryStats.Dexterity + BonusPrimaryStats.Dexterity;
	case EMMOPrimaryStat::Intelligence:
		return BasePrimaryStats.Intelligence + BonusPrimaryStats.Intelligence;
	case EMMOPrimaryStat::Vitality:
		return BasePrimaryStats.Vitality + BonusPrimaryStats.Vitality;
	case EMMOPrimaryStat::Wisdom:
		return BasePrimaryStats.Wisdom + BonusPrimaryStats.Wisdom;
	case EMMOPrimaryStat::Luck:
		return BasePrimaryStats.Luck + BonusPrimaryStats.Luck;
	default:
		return 0;
	}
}

void UMMOStatComponent::ModifyBasePrimaryStat(EMMOPrimaryStat StatType, int32 Delta)
{
	if (!HasAuthority())
	{
		return;
	}

	int32 OldValue = GetEffectivePrimaryStat(StatType);

	switch (StatType)
	{
	case EMMOPrimaryStat::Strength:
		BasePrimaryStats.Strength += Delta;
		break;
	case EMMOPrimaryStat::Dexterity:
		BasePrimaryStats.Dexterity += Delta;
		break;
	case EMMOPrimaryStat::Intelligence:
		BasePrimaryStats.Intelligence += Delta;
		break;
	case EMMOPrimaryStat::Vitality:
		BasePrimaryStats.Vitality += Delta;
		break;
	case EMMOPrimaryStat::Wisdom:
		BasePrimaryStats.Wisdom += Delta;
		break;
	case EMMOPrimaryStat::Luck:
		BasePrimaryStats.Luck += Delta;
		break;
	}

	OnRep_PrimaryStats();
	OnPrimaryStatChanged.Broadcast(StatType, OldValue, GetEffectivePrimaryStat(StatType));
}

void UMMOStatComponent::SetBasePrimaryStat(EMMOPrimaryStat StatType, int32 NewValue)
{
	if (!HasAuthority())
	{
		return;
	}

	int32 OldValue = GetEffectivePrimaryStat(StatType);

	switch (StatType)
	{
	case EMMOPrimaryStat::Strength:
		BasePrimaryStats.Strength = NewValue;
		break;
	case EMMOPrimaryStat::Dexterity:
		BasePrimaryStats.Dexterity = NewValue;
		break;
	case EMMOPrimaryStat::Intelligence:
		BasePrimaryStats.Intelligence = NewValue;
		break;
	case EMMOPrimaryStat::Vitality:
		BasePrimaryStats.Vitality = NewValue;
		break;
	case EMMOPrimaryStat::Wisdom:
		BasePrimaryStats.Wisdom = NewValue;
		break;
	case EMMOPrimaryStat::Luck:
		BasePrimaryStats.Luck = NewValue;
		break;
	}

	OnRep_PrimaryStats();
	OnPrimaryStatChanged.Broadcast(StatType, OldValue, GetEffectivePrimaryStat(StatType));
}

void UMMOStatComponent::AddBonusPrimaryStat(EMMOPrimaryStat StatType, int32 Amount)
{
	switch (StatType)
	{
	case EMMOPrimaryStat::Strength:
		BonusPrimaryStats.Strength += Amount;
		break;
	case EMMOPrimaryStat::Dexterity:
		BonusPrimaryStats.Dexterity += Amount;
		break;
	case EMMOPrimaryStat::Intelligence:
		BonusPrimaryStats.Intelligence += Amount;
		break;
	case EMMOPrimaryStat::Vitality:
		BonusPrimaryStats.Vitality += Amount;
		break;
	case EMMOPrimaryStat::Wisdom:
		BonusPrimaryStats.Wisdom += Amount;
		break;
	case EMMOPrimaryStat::Luck:
		BonusPrimaryStats.Luck += Amount;
		break;
	}

	CalculateSecondaryStats();
}

void UMMOStatComponent::RemoveBonusPrimaryStat(EMMOPrimaryStat StatType, int32 Amount)
{
	AddBonusPrimaryStat(StatType, -Amount);
}

void UMMOStatComponent::ClearBonusStats()
{
	BonusPrimaryStats = FMMOPrimaryStats();
	CalculateSecondaryStats();
}

float UMMOStatComponent::GetSecondaryStat(EMMOSecondaryStat StatType) const
{
	switch (StatType)
	{
	case EMMOSecondaryStat::AttackPower:
		return SecondaryStats.AttackPower;
	case EMMOSecondaryStat::SpellPower:
		return SecondaryStats.SpellPower;
	case EMMOSecondaryStat::Defense:
		return SecondaryStats.Defense;
	case EMMOSecondaryStat::MagicDefense:
		return SecondaryStats.MagicDefense;
	case EMMOSecondaryStat::CriticalHitChance:
		return SecondaryStats.CriticalHitChance;
	case EMMOSecondaryStat::CriticalHitDamage:
		return SecondaryStats.CriticalHitDamage;
	case EMMOSecondaryStat::DodgeChance:
		return SecondaryStats.DodgeChance;
	case EMMOSecondaryStat::ParryChance:
		return SecondaryStats.ParryChance;
	case EMMOSecondaryStat::BlockChance:
		return SecondaryStats.BlockChance;
	case EMMOSecondaryStat::HitChance:
		return SecondaryStats.HitChance;
	case EMMOSecondaryStat::Haste:
		return SecondaryStats.Haste;
	case EMMOSecondaryStat::MovementSpeedModifier:
		return SecondaryStats.MovementSpeedModifier;
	default:
		return 0.0f;
	}
}

void UMMOStatComponent::CalculateSecondaryStats()
{
	FMMOSecondaryStats OldStats = SecondaryStats;

	int32 Str = GetEffectivePrimaryStat(EMMOPrimaryStat::Strength);
	int32 Dex = GetEffectivePrimaryStat(EMMOPrimaryStat::Dexterity);
	int32 Int = GetEffectivePrimaryStat(EMMOPrimaryStat::Intelligence);
	int32 Vit = GetEffectivePrimaryStat(EMMOPrimaryStat::Vitality);
	int32 Wis = GetEffectivePrimaryStat(EMMOPrimaryStat::Wisdom);
	int32 Luk = GetEffectivePrimaryStat(EMMOPrimaryStat::Luck);

	AMMOCharacterBase* OwnerChar = Cast<AMMOCharacterBase>(GetOwner());
	int32 CharLevel = OwnerChar ? OwnerChar->Level : 1;

	SecondaryStats.AttackPower = Str * 2.0f + Dex * 0.5f + CharLevel * 0.5f;
	SecondaryStats.SpellPower = Int * 2.5f + Wis * 0.5f + CharLevel * 0.5f;
	SecondaryStats.Defense = Vit * 1.5f + Str * 0.5f + CharLevel;
	SecondaryStats.MagicDefense = Wis * 1.5f + Int * 0.5f + CharLevel;
	SecondaryStats.CriticalHitChance = FMath::Min(Dex * 0.1f + Luk * 0.05f, 75.0f);
	SecondaryStats.CriticalHitDamage = 150.0f + Str * 0.5f;
	SecondaryStats.DodgeChance = FMath::Min(Dex * 0.15f + Luk * 0.05f, 50.0f);
	SecondaryStats.ParryChance = FMath::Min(Dex * 0.1f + Str * 0.05f, 40.0f);
	SecondaryStats.BlockChance = FMath::Min(Str * 0.1f, 30.0f);
	SecondaryStats.HitChance = 100.0f + Dex * 0.05f;
	SecondaryStats.Haste = FMath::Max(Dex * 0.2f, 0.0f);
	SecondaryStats.MovementSpeedModifier = 1.0f;

	if (StatCurveTable)
	{
		static const FName ContextStr(TEXT("StatCurveLookup"));
		FString ContextString(ContextStr.ToString());

		FRichCurve* AttackCurve = StatCurveTable->FindRichCurve(FName(TEXT("AttackPower")), ContextString);
		if (AttackCurve)
		{
			SecondaryStats.AttackPower = AttackCurve->Eval(CharLevel);
		}

		FRichCurve* DefenseCurve = StatCurveTable->FindRichCurve(FName(TEXT("Defense")), ContextString);
		if (DefenseCurve)
		{
			SecondaryStats.Defense = DefenseCurve->Eval(CharLevel);
		}
	}

	UpdateDerivedStats();
}

void UMMOStatComponent::UpdateDerivedStats()
{
	AMMOCharacterBase* OwnerChar = Cast<AMMOCharacterBase>(GetOwner());
	if (!OwnerChar || !HasAuthority())
	{
		return;
	}

	int32 Vit = GetEffectivePrimaryStat(EMMOPrimaryStat::Vitality);
	int32 Wis = GetEffectivePrimaryStat(EMMOPrimaryStat::Wisdom);

	OwnerChar->MaxHP = 100.0f + (OwnerChar->Level - 1) * 25.0f + Vit * 10.0f;
	OwnerChar->MaxMP = 50.0f + (OwnerChar->Level - 1) * 12.0f + Wis * 8.0f;
	OwnerChar->MaxStamina = 100.0f + (OwnerChar->Level - 1) * 5.0f + Vit * 2.0f;
}

float UMMOStatComponent::LookupStatCurve(FName CurveRowName, float InputValue) const
{
	if (!StatCurveTable)
	{
		return InputValue;
	}

	static const FName ContextStr(TEXT("StatCurveLookup"));
	FString ContextString(ContextStr.ToString());

	FRichCurve* Curve = StatCurveTable->FindRichCurve(CurveRowName, ContextString);
	if (Curve)
	{
		return Curve->Eval(InputValue);
	}

	return InputValue;
}

void UMMOStatComponent::OnRep_PrimaryStats()
{
	CalculateSecondaryStats();
}

void UMMOStatComponent::RecalculateAll()
{
	CalculateSecondaryStats();
}

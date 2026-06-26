#include "Data/MMOLevelData.h"
#include "Engine/CurveTable.h"

UMMOLevelDataAsset::UMMOLevelDataAsset()
{
	MaxLevel = 60;
	BaseExperienceRequirement = 100;
	ExperienceExponent = 1.5f;
	HPMultiplierPerLevel = 25.0f;
	MPMultiplierPerLevel = 12.0f;
}

int64 UMMOLevelDataAsset::GetExperienceRequiredForLevel(int32 TargetLevel) const
{
	if (TargetLevel <= 1)
	{
		return 0;
	}

	if (ExperienceRequirements.Contains(TargetLevel))
	{
		return ExperienceRequirements[TargetLevel];
	}

	if (ExperienceCurve)
	{
		return static_cast<int64>(ExperienceCurve->GetFloatValue(TargetLevel));
	}

	if (ExperienceCurveTable && ExperienceCurveRow != NAME_None)
	{
		static const FName ContextStr(TEXT("LevelCurveLookup"));
		FString ContextString(ContextStr.ToString());

		FRichCurve* Curve = ExperienceCurveTable->FindRichCurve(ExperienceCurveRow, ContextString);
		if (Curve)
		{
			return static_cast<int64>(Curve->Eval(TargetLevel));
		}
	}

	return static_cast<int64>(BaseExperienceRequirement * FMath::Pow(TargetLevel, ExperienceExponent));
}

int32 UMMOLevelDataAsset::GetLevelFromExperience(int64 TotalExperience) const
{
	int64 AccumulatedExp = 0;
	for (int32 Level = 1; Level <= MaxLevel; Level++)
	{
		int64 RequiredExp = GetExperienceRequiredForLevel(Level + 1);
		AccumulatedExp += RequiredExp;

		if (TotalExperience < AccumulatedExp)
		{
			return Level;
		}
	}

	return MaxLevel;
}

FMMOLevelStatGain UMMOLevelDataAsset::GetStatGainForLevel(int32 Level) const
{
	if (StatGainsPerLevel.IsValidIndex(Level - 1))
	{
		return StatGainsPerLevel[Level - 1];
	}

	FMMOLevelStatGain DefaultGain;
	DefaultGain.Strength = 1;
	DefaultGain.Dexterity = 1;
	DefaultGain.Intelligence = 1;
	DefaultGain.Vitality = 1;
	DefaultGain.Wisdom = 1;
	DefaultGain.Luck = 0;
	DefaultGain.MaxHPGain = HPMultiplierPerLevel;
	DefaultGain.MaxMPGain = MPMultiplierPerLevel;

	return DefaultGain;
}

TArray<FMMOAbilityUnlock> UMMOLevelDataAsset::GetAbilitiesForLevel(int32 Level) const
{
	TArray<FMMOAbilityUnlock> Abilities;
	for (const FMMOAbilityUnlock& Unlock : AbilityUnlocks)
	{
		if (Unlock.UnlockLevel == Level)
		{
			Abilities.Add(Unlock);
		}
	}
	return Abilities;
}

bool UMMOLevelDataAsset::HasMaxLevelBeenReached(int32 CurrentLevel) const
{
	return CurrentLevel >= MaxLevel;
}

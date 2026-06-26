#include "Abilities/MMOAbilityBase.h"
#include "Abilities/MMOAbilityComponent.h"
#include "GameFramework/Character.h"

UMMOAbilityBase::UMMOAbilityBase()
{
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Default"));
}

bool UMMOAbilityBase::CanCast(const AMMOCharacterBase* Caster, const AMMOCharacterBase* Target) const
{
	if (!Caster)
	{
		return false;
	}

	if (bIsOnCooldown)
	{
		return false;
	}

	if (AbilityType == EMMOAbilityType::Passive)
	{
		return false;
	}

	if (RequiredLevel > 0 && CurrentRank < 1)
	{
		return false;
	}

	if (RequiredStance != EMMOStance::None)
	{
		return false;
	}

	if (RequiredWeaponType != EMMOWeaponType::None)
	{
		return false;
	}

	if (TargetType == EMMOTargetType::SingleEnemy || TargetType == EMMOTargetType::SingleAlly)
	{
		if (!Target)
		{
			return false;
		}
		const float RangeSqr = Range * Range;
		if (FVector::DistSquared(Caster->GetActorLocation(), Target->GetActorLocation()) > RangeSqr)
		{
			return false;
		}
	}

	if (TargetType == EMMOTargetType::Self)
	{
		return true;
	}

	return true;
}

void UMMOAbilityBase::OnCastStart_Implementation(AMMOCharacterBase* Caster, const FVector& TargetLocation, AMMOCharacterBase* Target)
{
	bIsCasting = true;
	if (OwningComponent.IsValid())
	{
		OwningComponent->OnCastStarted.Broadcast(this, Target);
	}

	if (!CastMontage.IsNull())
	{
		if (ACharacter* Char = Cast<ACharacter>(Caster))
		{
			UAnimInstance* AnimInst = Char->GetMesh()->GetAnimInstance();
			if (AnimInst)
			{
				AnimInst->Montage_Play(CastMontage.LoadSynchronous());
			}
		}
	}
}

void UMMOAbilityBase::OnCastTick_Implementation(AMMOCharacterBase* Caster, float DeltaTime)
{
	CurrentCastProgress += DeltaTime;
}

void UMMOAbilityBase::OnCastCompleted_Implementation(AMMOCharacterBase* Caster, const FVector& TargetLocation, AMMOCharacterBase* Target)
{
	bIsCasting = false;
	CurrentCastProgress = 0.0f;

	if (OwningComponent.IsValid())
	{
		OwningComponent->OnCastCompleted.Broadcast(this, Target);
	}
}

void UMMOAbilityBase::OnCastCancelled_Implementation(AMMOCharacterBase* Caster)
{
	bIsCasting = false;
	CurrentCastProgress = 0.0f;

	if (OwningComponent.IsValid())
	{
		OwningComponent->OnCastCancelled.Broadcast(this);
	}

	if (ACharacter* Char = Cast<ACharacter>(Caster))
	{
		UAnimInstance* AnimInst = Char->GetMesh()->GetAnimInstance();
		if (AnimInst && AnimInst->Montage_IsPlaying(CastMontage.LoadSynchronous()))
		{
			AnimInst->Montage_Stop(0.25f);
		}
	}
}

float UMMOAbilityBase::GetCooldownForRank(int32 Rank) const
{
	if (RankData.IsValidIndex(Rank - 1))
	{
		return RankData[Rank - 1].Cooldown;
	}
	return BaseCooldown;
}

float UMMOAbilityBase::GetCastTimeForRank(int32 Rank) const
{
	if (RankData.IsValidIndex(Rank - 1))
	{
		return RankData[Rank - 1].CastTime;
	}
	return BaseCastTime;
}

float UMMOAbilityBase::GetRangeForRank(int32 Rank) const
{
	if (RankData.IsValidIndex(Rank - 1))
	{
		return RankData[Rank - 1].Range;
	}
	return Range;
}

FMMOAbilityResourceCost UMMOAbilityBase::GetResourceCostForRank(int32 Rank) const
{
	if (RankData.IsValidIndex(Rank - 1))
	{
		return RankData[Rank - 1].ResourceCost;
	}
	return ResourceCost;
}

float UMMOAbilityBase::GetDamageForRank(int32 Rank, float CasterLevel) const
{
	float Damage = BaseDamage;
	if (RankData.IsValidIndex(Rank - 1))
	{
		Damage *= RankData[Rank - 1].DamageMultiplier;
	}
	Damage += DamagePerLevel * FMath::Max(0.0f, CasterLevel - 1.0f);
	return Damage;
}

float UMMOAbilityBase::GetHealingForRank(int32 Rank, float CasterLevel) const
{
	float Healing = BaseHealing;
	if (RankData.IsValidIndex(Rank - 1))
	{
		Healing *= RankData[Rank - 1].HealingMultiplier;
	}
	Healing += HealingPerLevel * FMath::Max(0.0f, CasterLevel - 1.0f);
	return Healing;
}

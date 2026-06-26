#include "Abilities/MMOBuff.h"
#include "Abilities/MMOBuffSystem.h"
#include "GameFramework/Character.h"
#include "Characters/MMOCharacterBase.h"

UMMOBuff::UMMOBuff()
{
	BuffTags.AddTag(FGameplayTag::RequestGameplayTag("Buff.Default"));
}

void UMMOBuff::OnApplied_Implementation(AActor* Source, AActor* Target)
{
	TimeSinceApplied = 0.0f;
	TimeSinceLastTick = 0.0f;
	RemainingTime = bInfiniteDuration ? -1.0f : Duration;
	SourceActor = Source;
	TargetActor = Target;
}

void UMMOBuff::OnTick_Implementation(AActor* Target, float DeltaTime)
{
	TimeSinceApplied += DeltaTime;
	RemainingTime = bInfiniteDuration ? -1.0f : FMath::Max(0.0f, RemainingTime - DeltaTime);

	// Handle periodic effects
	if (PeriodicEffect.TickInterval > 0.0f)
	{
		TimeSinceLastTick += DeltaTime;
		if (TimeSinceLastTick >= PeriodicEffect.TickInterval)
		{
			TimeSinceLastTick = 0.0f;

			if (OwningSystem.IsValid())
			{
				if (PeriodicEffect.BaseDamage > 0.0f && Target)
				{
					AMMOCharacterBase* CharTarget = Cast<AMMOCharacterBase>(Target);
					if (CharTarget)
					{
						FMMODamageResult DotResult;
						DotResult.RawDamage = PeriodicEffect.BaseDamage;
						DotResult.FinalDamage = PeriodicEffect.BaseDamage;
						DotResult.DamageType = PeriodicEffect.DamageType;
						DotResult.DamageTags = PeriodicEffect.EffectTags;
						DotResult.bIsPeriodic = true;
						DotResult.Instigator = SourceActor.Get();
						DotResult.Causer = SourceActor.Get();
						CharTarget->ReceiveDamage(DotResult);
					}
				}

				if (PeriodicEffect.BaseHealing > 0.0f && Target)
				{
					AMMOCharacterBase* CharTarget = Cast<AMMOCharacterBase>(Target);
					if (CharTarget)
					{
						CharTarget->ReceiveHealing(PeriodicEffect.BaseHealing, SourceActor.Get());
					}
				}
			}
		}
	}
}

void UMMOBuff::OnExpired_Implementation(AActor* Target)
{
	RemainingTime = 0.0f;
}

void UMMOBuff::OnRemoved_Implementation(AActor* Target, bool bDispelled)
{
	// Clean up any child buffs if needed
	if (OwningSystem.IsValid())
	{
		OwningSystem->RemoveBuffByID(BuffID, bDispelled);
	}
}

void UMMOBuff::OnStackChanged_Implementation(int32 OldStacks, int32 NewStacks)
{
	CurrentStacks = NewStacks;
}

bool UMMOBuff::IsExpired() const
{
	if (bInfiniteDuration)
	{
		return false;
	}
	return RemainingTime <= 0.0f;
}

void UMMOBuff::RefreshDuration()
{
	RemainingTime = Duration;
	TimeSinceApplied = 0.0f;
}

void UMMOBuff::AddStack(int32 Amount)
{
	const int32 OldStacks = CurrentStacks;
	CurrentStacks = FMath::Min(CurrentStacks + Amount, MaxStacks);
	if (OldStacks != CurrentStacks)
	{
		OnStackChanged(OldStacks, CurrentStacks);
	}
}

void UMMOBuff::RemoveStack(int32 Amount)
{
	const int32 OldStacks = CurrentStacks;
	CurrentStacks = FMath::Max(0, CurrentStacks - Amount);
	if (OldStacks != CurrentStacks)
	{
		OnStackChanged(OldStacks, CurrentStacks);
	}
}

float UMMOBuff::GetRemainingTimePercent() const
{
	if (Duration <= 0.0f || bInfiniteDuration)
	{
		return 1.0f;
	}
	return FMath::Clamp(RemainingTime / Duration, 0.0f, 1.0f);
}

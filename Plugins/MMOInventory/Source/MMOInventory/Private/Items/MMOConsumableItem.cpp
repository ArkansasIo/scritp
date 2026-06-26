#include "Items/MMOConsumableItem.h"

UMMOConsumableItem::UMMOConsumableItem()
	: EffectType(EMMOConsumableEffect::Heal)
	, EffectDuration(0.0f)
	, CooldownDuration(0.0f)
	, bIsInstant(true)
	, StacksConsumedPerUse(1)
	, TargetType(EMMOConsumableTarget::Self)
{
	ItemType = EMMOItemType::Consumable;
}

bool UMMOConsumableItem::CanUse_Implementation(APlayerController* User)
{
	if (!User)
	{
		return false;
	}
	return true;
}

bool UMMOConsumableItem::OnUse_Implementation(APlayerController* User)
{
	if (!CanUse(User))
	{
		return false;
	}

	AActor* TargetActor = User->GetPawn();
	if (!TargetActor)
	{
		return false;
	}

	return ApplyEffect(TargetActor);
}

bool UMMOConsumableItem::ApplyEffect(AActor* Target)
{
	if (!Target)
	{
		return false;
	}

	switch (EffectType)
	{
	case EMMOConsumableEffect::Heal:
	{
		float HealAmount = GetTotalEffectValue();
		break;
	}
	case EMMOConsumableEffect::Mana:
	{
		float ManaAmount = GetTotalEffectValue();
		break;
	}
	case EMMOConsumableEffect::Buff:
		break;
	case EMMOConsumableEffect::RemoveDebuff:
		break;
	case EMMOConsumableEffect::TemporaryStatBoost:
		break;
	case EMMOConsumableEffect::Teleport:
		break;
	}

	return true;
}

float UMMOConsumableItem::GetTotalEffectValue() const
{
	float Total = 0.0f;
	for (float Value : EffectValues)
	{
		Total += Value;
	}
	return Total;
}

bool UMMOConsumableItem::HasCooldown() const
{
	return CooldownDuration > 0.0f;
}

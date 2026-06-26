#include "UI/MMOCombatTextManager.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"

UMMOCombatTextManager::UMMOCombatTextManager()
{
	PrimaryComponentTick.bCanEverTick = true;

	PhysicalColor = FLinearColor::White;
	MagicColor = FLinearColor::Blue;
	FireColor = FLinearColor::Red;
	FrostColor = FLinearColor(0.0f, 1.0f, 1.0f);
	NatureColor = FLinearColor::Green;
	ShadowColor = FLinearColor(0.5f, 0.0f, 0.5f);
	HolyColor = FLinearColor::Yellow;
	ArcaneColor = FLinearColor(1.0f, 0.0f, 1.0f);
}

void UMMOCombatTextManager::BeginPlay()
{
	Super::BeginPlay();
	InitializePool();
}

void UMMOCombatTextManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CleanupExpiredTexts();
}

void UMMOCombatTextManager::InitializePool()
{
	for (int32 i = 0; i < PoolSize; i++)
	{
		AMMOFloatingTextActor* TextActor = CreateFloatingTextActor();
		if (TextActor)
		{
			FloatingTextPool.Add(TextActor);
			TextActor->SetActorHiddenInGame(true);
			TextActor->SetActorEnableCollision(false);
		}
	}
}

AMMOFloatingTextActor* UMMOCombatTextManager::CreateFloatingTextActor()
{
	if (!FloatingTextActorClass)
		return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return GetWorld()->SpawnActor<AMMOFloatingTextActor>(FloatingTextActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
}

AMMOFloatingTextActor* UMMOCombatTextManager::AcquireFloatingText()
{
	if (FloatingTextPool.Num() > 0)
	{
		AMMOFloatingTextActor* TextActor = FloatingTextPool.Pop();
		ActiveFloatingTexts.Add(TextActor);
		TextActor->SetActorHiddenInGame(false);
		return TextActor;
	}

	AMMOFloatingTextActor* TextActor = CreateFloatingTextActor();
	if (TextActor)
	{
		ActiveFloatingTexts.Add(TextActor);
	}
	return TextActor;
}

void UMMOCombatTextManager::ReleaseFloatingText(AMMOFloatingTextActor* TextActor)
{
	if (TextActor)
	{
		ActiveFloatingTexts.Remove(TextActor);
		TextActor->SetActorHiddenInGame(true);
		TextActor->SetActorEnableCollision(false);
		FloatingTextPool.Add(TextActor);
	}
}

void UMMOCombatTextManager::SpawnDamageText(float Damage, EMMOHitResult HitResult, EMMODamageType DamageType, AActor* Target, const FVector& Location)
{
	FMMOFloatingTextEntry Entry;
	Entry.Value = Damage;
	Entry.HitResult = HitResult;
	Entry.DamageType = DamageType;
	Entry.bIsHealing = false;
	Entry.WorldLocation = Location;
	Entry.bIsBigHit = (HitResult == EMMOHitResult::Critical) && (Damage >= 100.0f);
	Entry.TargetActor = Target;

	SpawnTextIndicator(Entry);
}

void UMMOCombatTextManager::SpawnHealingText(float Healing, EMMOHitResult HitResult, AActor* Target, const FVector& Location)
{
	FMMOFloatingTextEntry Entry;
	Entry.Value = Healing;
	Entry.HitResult = HitResult;
	Entry.DamageType = EMMODamageType::Holy;
	Entry.bIsHealing = true;
	Entry.WorldLocation = Location;
	Entry.bIsBigHit = (HitResult == EMMOHitResult::Critical) && (Healing >= 100.0f);
	Entry.TargetActor = Target;

	SpawnTextIndicator(Entry);
}

void UMMOCombatTextManager::SpawnMissText(AActor* Target, const FVector& Location)
{
	FMMOFloatingTextEntry Entry;
	Entry.Value = 0;
	Entry.HitResult = EMMOHitResult::Miss;
	Entry.DamageType = EMMODamageType::Physical;
	Entry.bIsHealing = false;
	Entry.WorldLocation = Location;
	Entry.TargetActor = Target;

	SpawnTextIndicator(Entry);
}

void UMMOCombatTextManager::SpawnDodgeText(AActor* Target, const FVector& Location)
{
	FMMOFloatingTextEntry Entry;
	Entry.Value = 0;
	Entry.HitResult = EMMOHitResult::Dodge;
	Entry.DamageType = EMMODamageType::Physical;
	Entry.bIsHealing = false;
	Entry.WorldLocation = Location;
	Entry.TargetActor = Target;

	SpawnTextIndicator(Entry);
}

void UMMOCombatTextManager::SpawnParryText(AActor* Target, const FVector& Location)
{
	FMMOFloatingTextEntry Entry;
	Entry.Value = 0;
	Entry.HitResult = EMMOHitResult::Parry;
	Entry.DamageType = EMMODamageType::Physical;
	Entry.bIsHealing = false;
	Entry.WorldLocation = Location;
	Entry.TargetActor = Target;

	SpawnTextIndicator(Entry);
}

void UMMOCombatTextManager::SpawnBlockText(AActor* Target, const FVector& Location)
{
	FMMOFloatingTextEntry Entry;
	Entry.Value = 0;
	Entry.HitResult = EMMOHitResult::Block;
	Entry.DamageType = EMMODamageType::Physical;
	Entry.bIsHealing = false;
	Entry.WorldLocation = Location;
	Entry.TargetActor = Target;

	SpawnTextIndicator(Entry);
}

void UMMOCombatTextManager::SpawnGlanceText(AActor* Target, const FVector& Location)
{
	FMMOFloatingTextEntry Entry;
	Entry.Value = 0;
	Entry.HitResult = EMMOHitResult::Glancing;
	Entry.DamageType = EMMODamageType::Physical;
	Entry.bIsHealing = false;
	Entry.WorldLocation = Location;
	Entry.TargetActor = Target;

	SpawnTextIndicator(Entry);
}

FLinearColor UMMOCombatTextManager::GetDamageColor(EMMODamageType DamageType) const
{
	switch (DamageType)
	{
	case EMMODamageType::Physical: return PhysicalColor;
	case EMMODamageType::Magic:    return MagicColor;
	case EMMODamageType::Fire:     return FireColor;
	case EMMODamageType::Frost:    return FrostColor;
	case EMMODamageType::Nature:   return NatureColor;
	case EMMODamageType::Shadow:   return ShadowColor;
	case EMMODamageType::Holy:     return HolyColor;
	case EMMODamageType::Arcane:   return ArcaneColor;
	default: return FLinearColor::White;
	}
}

void UMMOCombatTextManager::PlayScreenShake(float Intensity)
{
	if (BigHitShake && GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(BigHitShake, Intensity);
	}
}

void UMMOCombatTextManager::CleanupExpiredTexts()
{
	for (int32 i = ActiveFloatingTexts.Num() - 1; i >= 0; i--)
	{
		if (ActiveFloatingTexts[i] && !ActiveFloatingTexts[i]->bIsFloating)
		{
			ReleaseFloatingText(ActiveFloatingTexts[i]);
		}
	}
}

void UMMOCombatTextManager::SpawnTextIndicator(const FMMOFloatingTextEntry& Entry)
{
	AMMOFloatingTextActor* TextActor = AcquireFloatingText();
	if (TextActor)
	{
		TextActor->SetActorLocation(Entry.WorldLocation);
		TextActor->SetTextEntry(Entry);
		TextActor->StartFloating();
		OnCombatTextSpawned.Broadcast(Entry, TextActor);

		if (Entry.bIsBigHit)
		{
			PlayScreenShake(Entry.Value / 100.0f);
		}
	}
}

AMMOFloatingTextActor::AMMOFloatingTextActor()
{
	PrimaryActorTick.bCanEverTick = true;
	ElapsedTime = 0.0f;
}

void AMMOFloatingTextActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMMOFloatingTextActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFloating)
	{
		ElapsedTime += DeltaTime;

		FVector NewLocation = GetActorLocation();
		NewLocation.Z += FloatSpeed * DeltaTime;
		SetActorLocation(NewLocation);

		if (ElapsedTime >= FloatDuration)
		{
			bIsFloating = false;
			ReturnToPool();
		}
	}
}

void AMMOFloatingTextActor::SetTextEntry(const FMMOFloatingTextEntry& Entry)
{
	TextEntry = Entry;
	OnSpawn(Entry);
}

void AMMOFloatingTextActor::StartFloating()
{
	bIsFloating = true;
	ElapsedTime = 0.0f;
	SetActorHiddenInGame(false);
}

void AMMOFloatingTextActor::ReturnToPool()
{
	bIsFloating = false;
	SetActorHiddenInGame(true);
}

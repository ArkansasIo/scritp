#include "Characters/MMOEnemyCharacter.h"
#include "Components/MMOCombatStateComponent.h"
#include "Components/MMOStatComponent.h"
#include "AIController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

AMMOEnemyCharacter::AMMOEnemyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CharacterType = EMMOCharacterType::Enemy;
	bReplicates = true;
	bAutoRespawn = true;

	AggroRange = 1500.0f;
	LeashRange = 3000.0f;
	LeashReturnSpeed = 600.0f;
	DeAggroTime = 10.0f;

	DifficultyTier = EMMOEnemyDifficultyTier::Normal;

	ExperienceReward = 50;
	RespawnTime = 30.0f;

	CurrentPatrolIndex = 0;
	PatrolWaitTime = 5.0f;
	bLoopPatrol = true;

	SpawnEffect = nullptr;
	DeathEffect = nullptr;
	CurrentTarget = nullptr;

	SpawnOrigin = FVector::ZeroVector;
}

void AMMOEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMMOEnemyCharacter, SpawnGroupID, COND_Always);
	DOREPLIFETIME_CONDITION(AMMOEnemyCharacter, DifficultyTier, COND_Always);
	DOREPLIFETIME_CONDITION(AMMOEnemyCharacter, LootTableID, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMOEnemyCharacter, bAutoRespawn, COND_Always);
}

void AMMOEnemyCharacter::BeginPlay()
{
	SpawnOrigin = GetActorLocation();

	Super::BeginPlay();

	InitializeEnemyData();

	if (AIControllerClass && !GetController())
	{
		AIController = GetWorld()->SpawnActor<AAIController>(AIControllerClass, GetActorLocation(), GetActorRotation());
		if (AIController)
		{
			AIController->Possess(this);
		}
	}

	if (HasAuthority())
	{
		MulticastSpawnEffect();
	}
}

void AMMOEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority() || CharacterState == EMMOCharacterState::Dead)
	{
		return;
	}

	if (CurrentTarget && CombatStateComponent)
	{
		if (!IsInLeashRange())
		{
			CurrentTarget = nullptr;
			CombatStateComponent->ExitCombat();
		}
	}
}

void AMMOEnemyCharacter::InitializeEnemyData()
{
	CurrentHP = MaxHP;
	CurrentMP = MaxMP;
	Stamina = MaxStamina;
}

float AMMOEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (HasAuthority() && CharacterState == EMMOCharacterState::Alive)
	{
		ModifyHealth(-ActualDamage);

		if (CombatStateComponent)
		{
			CombatStateComponent->EnterCombat();
			CombatStateComponent->RecordDamage(DamageCauser, ActualDamage);
		}

		if (DamageCauser)
		{
			SetCurrentTarget(DamageCauser);
		}
	}

	return ActualDamage;
}

void AMMOEnemyCharacter::SetCurrentTarget(AActor* Target)
{
	CurrentTarget = Target;
}

bool AMMOEnemyCharacter::IsInAggroRange(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return false;
	}

	return FVector::DistSquared(GetActorLocation(), TargetActor->GetActorLocation()) <= AggroRange * AggroRange;
}

bool AMMOEnemyCharacter::IsInLeashRange() const
{
	return FVector::DistSquared(GetActorLocation(), SpawnOrigin) <= LeashRange * LeashRange;
}

void AMMOEnemyCharacter::Die()
{
	if (!HasAuthority() || CharacterState == EMMOCharacterState::Dead)
	{
		return;
	}

	Super::Die();

	CombatStateComponent->SetCombatState(EMMOCombatState::OutOfCombat);

	MulticastDeathEffect();

	if (CurrentTarget)
	{
		AMMOCharacterBase* TargetChar = Cast<AMMOCharacterBase>(CurrentTarget);
		if (TargetChar)
		{
			TargetChar->AddExperience(ExperienceReward);
		}
	}

	if (bAutoRespawn)
	{
		StartRespawn();
	}
}

void AMMOEnemyCharacter::OnDeath()
{
	Super::OnDeath();
}

void AMMOEnemyCharacter::StartRespawn()
{
	if (!HasAuthority())
	{
		return;
	}

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(RespawnTimerHandle, this, &AMMOEnemyCharacter::OnRespawnComplete, RespawnTime, false);
}

void AMMOEnemyCharacter::OnRespawnComplete()
{
	if (!HasAuthority())
	{
		return;
	}

	CharacterState = EMMOCharacterState::Alive;
	CurrentHP = MaxHP;
	CurrentMP = MaxMP;
	Stamina = MaxStamina;

	CurrentTarget = nullptr;

	SetActorLocation(SpawnOrigin);
	SetActorEnableCollision(true);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (GetController())
	{
		GetController()->SetIgnoreMoveInput(false);
	}

	MulticastSpawnEffect();
	OnCharacterRespawn.Broadcast();
}

FVector AMMOEnemyCharacter::GetNextPatrolPoint()
{
	if (PatrolRoutePoints.Num() == 0)
	{
		return SpawnOrigin;
	}

	return PatrolRoutePoints[CurrentPatrolIndex];
}

void AMMOEnemyCharacter::AdvancePatrol()
{
	if (PatrolRoutePoints.Num() == 0)
	{
		return;
	}

	CurrentPatrolIndex++;
	if (CurrentPatrolIndex >= PatrolRoutePoints.Num())
	{
		if (bLoopPatrol)
		{
			CurrentPatrolIndex = 0;
		}
		else
		{
			CurrentPatrolIndex = PatrolRoutePoints.Num() - 1;
		}
	}
}

void AMMOEnemyCharacter::MulticastSpawnEffect_Implementation()
{
	if (SpawnEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SpawnEffect, GetActorLocation(), GetActorRotation(), true);
	}
}

void AMMOEnemyCharacter::MulticastDeathEffect_Implementation()
{
	if (DeathEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeathEffect, GetActorLocation(), GetActorRotation(), true);
	}
}

void AMMOEnemyCharacter::SpawnVisualEffects()
{
}

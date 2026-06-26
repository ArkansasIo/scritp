#include "Components/MMOCharacterMovementComponent.h"
#include "Characters/MMOCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

UMMOCharacterMovementComponent::UMMOCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WalkSpeed = 150.0f;
	RunSpeed = 300.0f;
	SprintSpeed = 600.0f;
	SneakSpeed = 100.0f;
	SwimSpeed = 200.0f;
	MountedSpeed = 800.0f;
	FlySpeed = 600.0f;

	MaxAllowedSpeed = 1000.0f;
	SpeedHackTolerance = 1.5f;
	SpeedHackViolationLimit = 3;
	SpeedHackViolations = 0;

	SprintStaminaDrainRate = 15.0f;
	MinStaminaForSprint = 10.0f;
	StaminaRegenDelay = 1.0f;

	MMOMovementMode = EMMOMovementMode::Running;
	RegenDelayTimer = 0.0f;
	bStaminaExhausted = false;

	MaxWalkSpeed = RunSpeed;
	RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	bOrientRotationToMovement = true;
	AirControl = 0.8f;
	NavAgentProps.bCanCrouch = true;
}

void UMMOCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UMMOCharacterMovementComponent, MMOMovementMode, COND_SkipOwner);
}

void UMMOCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !PawnOwner->IsLocallyControlled())
	{
		return;
	}

	HandleSprintStamina(DeltaTime);
	UpdateMMOSpeed();
}

void UMMOCharacterMovementComponent::SetMMOMovementMode(EMMOMovementMode NewMode)
{
	EMMOMovementMode OldMode = MMOMovementMode;
	MMOMovementMode = NewMode;

	ApplyMovementSpeed();
	OnRep_MMOMovementMode(OldMode);
	OnMovementModeChanged.Broadcast(NewMode);
}

EMMOMovementMode UMMOCharacterMovementComponent::GetMMOMovementMode() const
{
	return MMOMovementMode;
}

bool UMMOCharacterMovementComponent::CanSprint() const
{
	AMMOCharacterBase* OwnerChar = Cast<AMMOCharacterBase>(PawnOwner);
	if (!OwnerChar)
	{
		return false;
	}

	return OwnerChar->Stamina > MinStaminaForSprint && !bStaminaExhausted && !OwnerChar->GetCharacterMovement()->IsCrouching();
}

bool UMMOCharacterMovementComponent::IsSprinting() const
{
	return MMOMovementMode == EMMOMovementMode::Sprinting;
}

void UMMOCharacterMovementComponent::ApplyMovementSpeed()
{
	switch (MMOMovementMode)
	{
	case EMMOMovementMode::Walking:
		MaxWalkSpeed = WalkSpeed;
		break;
	case EMMOMovementMode::Running:
		MaxWalkSpeed = RunSpeed;
		break;
	case EMMOMovementMode::Sprinting:
		MaxWalkSpeed = SprintSpeed;
		break;
	case EMMOMovementMode::Sneaking:
		MaxWalkSpeed = SneakSpeed;
		break;
	case EMMOMovementMode::Swimming:
		MaxWalkSpeed = SwimSpeed;
		MaxSwimSpeed = SwimSpeed;
		break;
	case EMMOMovementMode::Mounted:
		MaxWalkSpeed = MountedSpeed;
		break;
	case EMMOMovementMode::Flying:
		MaxWalkSpeed = FlySpeed;
		MaxFlySpeed = FlySpeed;
		break;
	default:
		MaxWalkSpeed = RunSpeed;
		break;
	}
}

void UMMOCharacterMovementComponent::HandleSprintStamina(float DeltaTime)
{
	AMMOCharacterBase* OwnerChar = Cast<AMMOCharacterBase>(PawnOwner);
	if (!OwnerChar)
	{
		return;
	}

	if (MMOMovementMode == EMMOMovementMode::Sprinting)
	{
		if (!CanSprint())
		{
			SetMMOMovementMode(EMMOMovementMode::Running);
			bStaminaExhausted = true;
			OnStaminaExhausted.Broadcast(true);
			return;
		}

		OwnerChar->ModifyStamina(-SprintStaminaDrainRate * DeltaTime);
		RegenDelayTimer = StaminaRegenDelay;

		if (OwnerChar->Stamina <= 0.0f)
		{
			SetMMOMovementMode(EMMOMovementMode::Running);
			bStaminaExhausted = true;
			OnStaminaExhausted.Broadcast(true);
		}
	}
	else
	{
		if (RegenDelayTimer > 0.0f)
		{
			RegenDelayTimer -= DeltaTime;
		}
		else if (bStaminaExhausted && OwnerChar->Stamina > MinStaminaForSprint * 2.0f)
		{
			bStaminaExhausted = false;
			OnStaminaExhausted.Broadcast(false);
		}
	}
}

void UMMOCharacterMovementComponent::UpdateMMOSpeed()
{
}

bool UMMOCharacterMovementComponent::CheckSpeedHack(FVector ClientVelocity)
{
	float ClientSpeed = ClientVelocity.Size();
	return ClientSpeed > MaxAllowedSpeed * SpeedHackTolerance;
}

void UMMOCharacterMovementComponent::ServerValidateMovement_Implementation(FVector_NetQuantize10 ClientLocation, FVector_NetQuantize10 ClientVelocity, float ClientTimestamp)
{
	if (CheckSpeedHack(ClientVelocity))
	{
		ReportSpeedHackViolation();
	}
}

bool UMMOCharacterMovementComponent::ServerValidateMovement_Validate(FVector_NetQuantize10 ClientLocation, FVector_NetQuantize10 ClientVelocity, float ClientTimestamp)
{
	return true;
}

bool UMMOCharacterMovementComponent::ValidateClientMovement(FVector ClientLocation, FVector ClientVelocity)
{
	return !CheckSpeedHack(ClientVelocity);
}

void UMMOCharacterMovementComponent::ReportSpeedHackViolation()
{
	SpeedHackViolations++;

	if (SpeedHackViolations >= SpeedHackViolationLimit)
	{
		UE_LOG(LogMMOCharacter, Warning, TEXT("Player %s kicked for speed hacking. Violations: %d"), *GetNameSafe(PawnOwner), SpeedHackViolations);

		APlayerController* PC = Cast<APlayerController>(PawnOwner->GetInstigatorController());
		if (PC)
		{
			PC->Destroy();
		}
	}
}

void UMMOCharacterMovementComponent::OnRep_MMOMovementMode(EMMOMovementMode OldMode)
{
	ApplyMovementSpeed();
	OnMovementModeChanged.Broadcast(MMOMovementMode);
}

#include "Components/MMOInteractionComponent.h"
#include "Characters/MMOCharacterBase.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

UMMOInteractionComponent::UMMOInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(true);

	InteractionRange = 300.0f;
	InteractionAngle = 60.0f;
	bCanInteract = true;

	CurrentFocusTarget = nullptr;
	CurrentInteractionTarget = nullptr;

	InteractionRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRangeSphere"));
	InteractionRangeSphere->SetSphereRadius(InteractionRange);
	InteractionRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionRangeSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionRangeSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionRangeSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	InteractionRangeSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	InteractionRangeSphere->SetupAttachment(this);
}

void UMMOInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMMOInteractionComponent, CurrentInteractionTarget, COND_OwnerOnly);
}

void UMMOInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMMOInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->IsLocallyControlled())
	{
		return;
	}

	UpdateInteractablesInRange();
	UpdateFocus();
}

void UMMOInteractionComponent::UpdateInteractablesInRange()
{
	InteractablesInRange.Empty();

	TArray<AActor*> OverlappingActors;
	InteractionRangeSphere->GetOverlappingActors(OverlappingActors, AMMOCharacterBase::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor != GetOwner())
		{
			InteractablesInRange.Add(Actor);
		}
	}
}

void UMMOInteractionComponent::UpdateFocus()
{
	AMMOCharacterBase* Closest = GetClosestInteractable();

	if (Closest != CurrentFocusTarget)
	{
		if (CurrentFocusTarget)
		{
			CurrentFocusTarget->OnEndFocus();
			OnEndFocus.Broadcast(CurrentFocusTarget);
		}

		CurrentFocusTarget = Closest;

		if (CurrentFocusTarget)
		{
			CurrentFocusTarget->OnBeginFocus();
			OnBeginFocus.Broadcast(CurrentFocusTarget);
			UpdateInteractionPrompt();
		}
	}
}

void UMMOInteractionComponent::GetInteractablesInRange(TArray<AActor*>& OutActors) const
{
	OutActors = InteractablesInRange;
}

AMMOCharacterBase* UMMOInteractionComponent::GetClosestInteractable() const
{
	if (InteractablesInRange.Num() == 0)
	{
		return nullptr;
	}

	AMMOCharacterBase* Closest = nullptr;
	float MinDist = FLT_MAX;
	FVector OwnerLocation = GetOwner()->GetActorLocation();

	for (AActor* Actor : InteractablesInRange)
	{
		AMMOCharacterBase* Char = Cast<AMMOCharacterBase>(Actor);
		if (!Char)
		{
			continue;
		}

		FVector DirToTarget = Char->GetActorLocation() - OwnerLocation;
		float Dist = DirToTarget.Size();

		if (Dist < MinDist)
		{
			FVector Forward = GetOwner()->GetActorForwardVector();
			DirToTarget.Normalize();
			float Dot = FVector::DotProduct(Forward, DirToTarget);
			float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

			if (Angle <= InteractionAngle)
			{
				MinDist = Dist;
				Closest = Char;
			}
		}
	}

	return Closest;
}

void UMMOInteractionComponent::Interact()
{
	AMMOCharacterBase* Target = GetClosestInteractable();
	if (Target)
	{
		InteractWithTarget(Target);
	}
}

void UMMOInteractionComponent::InteractWithTarget(AMMOCharacterBase* Target)
{
	if (!Target || !bCanInteract)
	{
		return;
	}

	CurrentInteractionTarget = Target;
	OnInteract.Broadcast(Cast<AMMOCharacterBase>(GetOwner()), Target);

	Target->OnInteract(Cast<AMMOCharacterBase>(GetOwner()));

	if (GetOwner()->HasAuthority())
	{
		ServerInteractWith(Target);
	}
	else
	{
		ServerInteractWith(Target);
	}
}

void UMMOInteractionComponent::ServerInteractWith_Implementation(AMMOCharacterBase* Target)
{
	if (Target && bCanInteract)
	{
		CurrentInteractionTarget = Target;
		Target->OnInteract(Cast<AMMOCharacterBase>(GetOwner()));
		OnInteract.Broadcast(Cast<AMMOCharacterBase>(GetOwner()), Target);
	}
}

bool UMMOInteractionComponent::ServerInteractWith_Validate(AMMOCharacterBase* Target)
{
	return Target != nullptr;
}

void UMMOInteractionComponent::ServerInteract(AMMOCharacterBase* Target)
{
	ServerInteractWith_Implementation(Target);
}

void UMMOInteractionComponent::FocusTarget(AMMOCharacterBase* Target)
{
	if (Target)
	{
		Target->OnBeginFocus();
		OnBeginFocus.Broadcast(Target);
	}
}

void UMMOInteractionComponent::UnfocusTarget()
{
	if (CurrentFocusTarget)
	{
		CurrentFocusTarget->OnEndFocus();
		OnEndFocus.Broadcast(CurrentFocusTarget);
		CurrentFocusTarget = nullptr;
	}
}

void UMMOInteractionComponent::UpdateInteractionPrompt()
{
	if (CurrentFocusTarget)
	{
		CurrentPromptData = DefaultInteractionPrompt;
		CurrentPromptData.InteractionText = FText::Format(
			FText::FromString(TEXT("Interact with {0}")),
			CurrentFocusTarget->DisplayName
		);
	}
}

bool UMMOInteractionComponent::TraceForInteraction(AMMOCharacterBase*& OutHitCharacter) const
{
	APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
	if (!PC)
	{
		OutHitCharacter = nullptr;
		return false;
	}

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FVector End = CamLoc + CamRot.Vector() * InteractionRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, End, ECC_Visibility, Params))
	{
		OutHitCharacter = Cast<AMMOCharacterBase>(Hit.GetActor());
		return OutHitCharacter != nullptr;
	}

	OutHitCharacter = nullptr;
	return false;
}

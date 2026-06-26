#include "Characters/MMOCharacterBase.h"
#include "Components/MMOCharacterMovementComponent.h"
#include "Components/MMOStatComponent.h"
#include "Components/MMOCombatStateComponent.h"
#include "Components/MMOInteractionComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AMMOCharacterBase::AMMOCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMMOCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bReplicates = true;
	bReplicateMovement = true;
	NetUpdateFrequency = 100.0f;
	MinNetUpdateFrequency = 33.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Level = 1;
	Experience = 0;
	CurrentHP = 100.0f;
	CurrentMP = 50.0f;
	MaxHP = 100.0f;
	MaxMP = 50.0f;
	Stamina = 100.0f;
	MaxStamina = 100.0f;

	CharacterState = EMMOCharacterState::Alive;
	CharacterType = EMMOCharacterType::NonPlayer;

	HealthRegenRate = 2.0f;
	ManaRegenRate = 1.5f;
	StaminaRegenRate = 5.0f;
	RegenInterval = 2.0f;
	OutOfCombatRegenMultiplier = 3.0f;

	bHighlightable = true;
	CustomDepthStencilValue = 1;
	bIsInFocus = false;
	bIsRespawnTimerActive = false;

	StatComponent = CreateDefaultSubobject<UMMOStatComponent>(TEXT("StatComponent"));
	CombatStateComponent = CreateDefaultSubobject<UMMOCombatStateComponent>(TEXT("CombatStateComponent"));
	InteractionComponent = CreateDefaultSubobject<UMMOInteractionComponent>(TEXT("InteractionComponent"));

	StatComponent->SetIsReplicated(true);
	CombatStateComponent->SetIsReplicated(true);
	InteractionComponent->SetIsReplicated(true);
}

void AMMOCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMMOCharacterBase, DisplayName, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMOCharacterBase, Title, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMOCharacterBase, GuildTag, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMOCharacterBase, CharacterType, COND_Always);

	DOREPLIFETIME_CONDITION(AMMOCharacterBase, Level, COND_Always);
	DOREPLIFETIME_CONDITION(AMMOCharacterBase, Experience, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMOCharacterBase, CurrentHP, COND_Always);
	DOREPLIFETIME_CONDITION(AMMOCharacterBase, CurrentMP, COND_Always);
	DOREPLIFETIME_CONDITION(AMMOCharacterBase, MaxHP, COND_Always);
	DOREPLIFETIME_CONDITION(AMMOCharacterBase, MaxMP, COND_Always);
	DOREPLIFETIME_CONDITION(AMMOCharacterBase, Stamina, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMOCharacterBase, MaxStamina, COND_OwnerOnly);

	DOREPLIFETIME_CONDITION(AMMOCharacterBase, CharacterState, COND_Always);
}

void AMMOCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	InitializeComponents();
}

void AMMOCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		StartRegeneration();
	}
}

void AMMOCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickCharacter(DeltaTime);
}

void AMMOCharacterBase::TickCharacter(float DeltaTime)
{
}

void AMMOCharacterBase::InitializeComponents()
{
	if (StatComponent)
	{
		StatComponent->InitializeComponent();
	}
	if (CombatStateComponent)
	{
		CombatStateComponent->InitializeComponent();
	}
	if (InteractionComponent)
	{
		InteractionComponent->InitializeComponent();
	}
}

void AMMOCharacterBase::StartRegeneration()
{
	if (!HasAuthority())
	{
		return;
	}

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(RegenTimerHandle, this, &AMMOCharacterBase::ApplyRegeneration, RegenInterval, true, 0.0f);
}

void AMMOCharacterBase::StopRegeneration()
{
	if (!HasAuthority())
	{
		return;
	}

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.ClearTimer(RegenTimerHandle);
}

void AMMOCharacterBase::ApplyRegeneration()
{
	if (!HasAuthority() || CharacterState == EMMOCharacterState::Dead)
	{
		return;
	}

	float RegenMultiplier = 1.0f;
	if (CombatStateComponent && CombatStateComponent->CombatState == EMMOCombatState::OutOfCombat)
	{
		RegenMultiplier = OutOfCombatRegenMultiplier;
	}

	if (CurrentHP < MaxHP)
	{
		float HPRegen = HealthRegenRate * RegenMultiplier * RegenInterval;
		CurrentHP = FMath::Min(CurrentHP + HPRegen, MaxHP);
		OnRep_CurrentHP(CurrentHP - HPRegen);
	}

	if (CurrentMP < MaxMP)
	{
		float MPRegen = ManaRegenRate * RegenMultiplier * RegenInterval;
		CurrentMP = FMath::Min(CurrentMP + MPRegen, MaxMP);
		OnRep_CurrentMP(CurrentMP - MPRegen);
	}

	if (Stamina < MaxStamina)
	{
		Stamina = FMath::Min(Stamina + StaminaRegenRate * RegenInterval, MaxStamina);
	}
}

void AMMOCharacterBase::AddExperience(int64 Amount)
{
	if (!HasAuthority())
	{
		return;
	}

	Experience += Amount;
	OnRep_Experience(Experience - Amount);

	UMMOLevelDataAsset* LevelData = nullptr;
	if (StatComponent && StatComponent->CharacterDataAsset)
	{
		// Level check handled here
	}

	int32 RequiredExp = 100 * Level;
	if (Experience >= RequiredExp)
	{
		SetLevel(Level + 1);
	}
}

void AMMOCharacterBase::SetLevel(int32 NewLevel)
{
	if (!HasAuthority())
	{
		return;
	}

	int32 OldLevel = Level;
	Level = FMath::Max(1, NewLevel);

	MaxHP = 100.0f + (Level - 1) * 25.0f;
	MaxMP = 50.0f + (Level - 1) * 12.0f;

	OnRep_Level(OldLevel);

	if (OldLevel < Level)
	{
		MulticastLevelUpEffect();
		OnLevelUp.Broadcast(Level);
	}
}

void AMMOCharacterBase::ModifyHealth(float Delta)
{
	if (!HasAuthority() || CharacterState == EMMOCharacterState::Dead)
	{
		return;
	}

	float OldHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP + Delta, 0.0f, MaxHP);
	OnRep_CurrentHP(OldHP);

	if (CurrentHP <= 0.0f)
	{
		Die();
	}
}

void AMMOCharacterBase::ModifyMana(float Delta)
{
	if (!HasAuthority())
	{
		return;
	}

	float OldMP = CurrentMP;
	CurrentMP = FMath::Clamp(CurrentMP + Delta, 0.0f, MaxMP);
	OnRep_CurrentMP(OldMP);
}

void AMMOCharacterBase::ModifyStamina(float Delta)
{
	if (!HasAuthority())
	{
		return;
	}

	Stamina = FMath::Clamp(Stamina + Delta, 0.0f, MaxStamina);
}

void AMMOCharacterBase::Die()
{
	if (!HasAuthority() || CharacterState == EMMOCharacterState::Dead)
	{
		return;
	}

	CharacterState = EMMOCharacterState::Dead;
	OnDeath();
	MulticastDeath();
	OnCharacterDeath.Broadcast();

	StopRegeneration();
}

void AMMOCharacterBase::OnDeath()
{
	if (GetController())
	{
		GetController()->SetIgnoreMoveInput(true);
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	SetActorEnableCollision(false);

	SetLifeSpan(10.0f);
}

void AMMOCharacterBase::MulticastDeath_Implementation()
{
	OnDeath();
}

void AMMOCharacterBase::Respawn()
{
	if (!HasAuthority() || CharacterState != EMMOCharacterState::Dead)
	{
		return;
	}

	CharacterState = EMMOCharacterState::Respawning;
	OnRespawn();
	MulticastRespawn();
}

void AMMOCharacterBase::OnRespawn()
{
	CharacterState = EMMOCharacterState::Spawned;

	CurrentHP = MaxHP;
	CurrentMP = MaxMP;
	Stamina = MaxStamina;

	OnRep_CurrentHP(0.0f);
	OnRep_CurrentMP(0.0f);

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	SetActorEnableCollision(true);

	if (GetController())
	{
		GetController()->SetIgnoreMoveInput(false);
	}

	if (HasAuthority())
	{
		CharacterState = EMMOCharacterState::Alive;
		StartRegeneration();
		OnCharacterRespawn.Broadcast();
	}
}

void AMMOCharacterBase::MulticastRespawn_Implementation()
{
	OnRespawn();
}

void AMMOCharacterBase::StartRespawnTimer(float RespawnDelay)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsRespawnTimerActive = true;
	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(RespawnTimerHandle, this, &AMMOCharacterBase::OnRespawnTimerComplete, RespawnDelay, false);
}

void AMMOCharacterBase::OnRespawnTimerComplete()
{
	bIsRespawnTimerActive = false;
	Respawn();
}

void AMMOCharacterBase::OnBeginFocus()
{
	bIsInFocus = true;
	SetHighlight(true);
}

void AMMOCharacterBase::OnEndFocus()
{
	bIsInFocus = false;
	SetHighlight(false);
}

void AMMOCharacterBase::OnInteract(AMMOCharacterBase* InstigatorCharacter)
{
	UE_LOG(LogMMOCharacter, Verbose, TEXT("%s interacted with %s"), *GetNameSafe(InstigatorCharacter), *GetNameSafe(this));
}

void AMMOCharacterBase::MulticastLevelUpEffect_Implementation()
{
	BP_LevelUpEffect();
}

void AMMOCharacterBase::SetAnimationBlueprintClass(TSubclassOf<UAnimInstance> AnimClass)
{
	if (GetMesh())
	{
		GetMesh()->SetAnimInstanceClass(AnimClass);
	}
}

void AMMOCharacterBase::SetAnimationBlueprintClassByPath(const FString& AnimBlueprintPath)
{
	UClass* AnimClass = StaticLoadClass(UAnimInstance::StaticClass(), nullptr, *AnimBlueprintPath);
	if (AnimClass)
	{
		SetAnimationBlueprintClass(AnimClass);
	}
}

void AMMOCharacterBase::SetHighlight(bool bEnabled)
{
	if (!GetMesh() || !bHighlightable)
	{
		return;
	}

	GetMesh()->SetRenderCustomDepth(bEnabled);
	if (bEnabled)
	{
		GetMesh()->SetCustomDepthStencilValue(CustomDepthStencilValue);
	}
}

void AMMOCharacterBase::OnRep_Level(int32 OldLevel)
{
	if (OldLevel < Level)
	{
		OnLevelUp.Broadcast(Level);
	}
}

void AMMOCharacterBase::OnRep_Experience(int64 OldExperience)
{
}

void AMMOCharacterBase::OnRep_CurrentHP(float OldHP)
{
	OnHPChanged.Broadcast(OldHP, CurrentHP);
}

void AMMOCharacterBase::OnRep_CurrentMP(float OldMP)
{
	OnMPChanged.Broadcast(OldMP, CurrentMP);
}

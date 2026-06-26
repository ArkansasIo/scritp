#include "Characters/MMONonPlayerCharacter.h"
#include "Components/MMOInteractionComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AMMONonPlayerCharacter::AMMONonPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CharacterType = EMMOCharacterType::NonPlayer;
	bReplicates = true;

	NPCType = EMMONPCType::Generic;
	QuestIndicator = EMMOQuestIndicator::None;
	bIsStaticSpawn = true;

	GreetingDistance = 400.0f;
	BuyPriceMultiplier = 1.0f;
	SellPriceMultiplier = 0.5f;
	SpawnRadius = 0.0f;

	DialogueTriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DialogueTriggerSphere"));
	DialogueTriggerSphere->SetupAttachment(RootComponent);
	DialogueTriggerSphere->SetSphereRadius(GreetingDistance);
	DialogueTriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DialogueTriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DialogueTriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	NameplateWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameplateWidget"));
	NameplateWidget->SetupAttachment(RootComponent);
	NameplateWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	NameplateWidget->SetWidgetSpace(EWidgetSpace::Screen);
	NameplateWidget->SetDrawSize(FVector2D(300.0f, 60.0f));
}

void AMMONonPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMMONonPlayerCharacter, NPCType, COND_Always);
	DOREPLIFETIME_CONDITION(AMMONonPlayerCharacter, CurrentGreeting, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMONonPlayerCharacter, QuestIndicator, COND_Always);
	DOREPLIFETIME_CONDITION(AMMONonPlayerCharacter, bIsStaticSpawn, COND_Always);
}

void AMMONonPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeNPCComponents();

	if (DialogueTriggerSphere)
	{
		DialogueTriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AMMONonPlayerCharacter::OnDialogueTriggerOverlap);
		DialogueTriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AMMONonPlayerCharacter::OnDialogueTriggerEndOverlap);
	}
}

void AMMONonPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMMONonPlayerCharacter::InitializeNPCComponents()
{
	if (GreetingText.IsEmpty() && GossipTextLines.Num() > 0)
	{
		GreetingText = GossipTextLines[0];
	}

	CurrentGreeting = GreetingText;
}

void AMMONonPlayerCharacter::OnInteract(AMMOCharacterBase* InstigatorCharacter)
{
	Super::OnInteract(InstigatorCharacter);

	if (NPCType == EMMONPCType::Vendor)
	{
		UE_LOG(LogMMOCharacter, Log, TEXT("%s opened vendor panel for %s"), *GetNameSafe(InstigatorCharacter), *GetNameSafe(this));
	}
	else if (NPCType == EMMONPCType::QuestGiver)
	{
		UE_LOG(LogMMOCharacter, Log, TEXT("%s opened quest dialogue for %s"), *GetNameSafe(InstigatorCharacter), *GetNameSafe(this));
	}
	else if (NPCType == EMMONPCType::Trainer)
	{
		UE_LOG(LogMMOCharacter, Log, TEXT("%s opened training panel for %s"), *GetNameSafe(InstigatorCharacter), *GetNameSafe(this));
	}
	else if (NPCType == EMMONPCType::Banker)
	{
		UE_LOG(LogMMOCharacter, Log, TEXT("%s opened bank for %s"), *GetNameSafe(InstigatorCharacter), *GetNameSafe(this));
	}
	else if (NPCType == EMMONPCType::Guard)
	{
		UE_LOG(LogMMOCharacter, Log, TEXT("%s greeted by guard %s"), *GetNameSafe(InstigatorCharacter), *GetNameSafe(this));
	}
}

void AMMONonPlayerCharacter::OnDialogueTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMMOCharacterBase* Character = Cast<AMMOCharacterBase>(OtherActor);
	if (Character)
	{
		CurrentGreeting = GreetingText;
		UE_LOG(LogMMOCharacter, Verbose, TEXT("%s entered dialogue range of %s"), *GetNameSafe(Character), *GetNameSafe(this));
	}
}

void AMMONonPlayerCharacter::OnDialogueTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMMOCharacterBase* Character = Cast<AMMOCharacterBase>(OtherActor);
	if (Character)
	{
		CurrentGreeting = FText();
		UE_LOG(LogMMOCharacter, Verbose, TEXT("%s left dialogue range of %s"), *GetNameSafe(Character), *GetNameSafe(this));
	}
}

void AMMONonPlayerCharacter::OnRep_QuestIndicator(EMMOQuestIndicator OldIndicator)
{
	BP_UpdateQuestIndicator(QuestIndicator);
}

void AMMONonPlayerCharacter::SetQuestIndicator(EMMOQuestIndicator NewIndicator)
{
	if (HasAuthority())
	{
		EMMOQuestIndicator OldIndicator = QuestIndicator;
		QuestIndicator = NewIndicator;
		OnRep_QuestIndicator(OldIndicator);
	}
}

void AMMONonPlayerCharacter::OnRep_Level(int32 OldLevel)
{
	Super::OnRep_Level(OldLevel);
}

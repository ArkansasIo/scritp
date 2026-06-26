#include "Characters/MMOPlayerCharacter.h"
#include "Components/MMOCharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

AMMOPlayerCharacter::AMMOPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CharacterType = EMMOCharacterType::Player;
	bReplicates = true;

	RestState = EMMOPlayerRestState::Standing;
	PvPFlag = EMMOPvPFlag::Peaceful;

	NumEquipmentSlots = 12;

	PlayerLevel = 1;
	TotalPlayTimeSeconds = 0;
}

void AMMOPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMMOPlayerCharacter, AppearanceData, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMOPlayerCharacter, RestState, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMMOPlayerCharacter, PvPFlag, COND_Always);
	DOREPLIFETIME_CONDITION(AMMOPlayerCharacter, PlayerLevel, COND_Always);
	DOREPLIFETIME_CONDITION(AMMOPlayerCharacter, TotalPlayTimeSeconds, COND_OwnerOnly);
}

void AMMOPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		EquipmentSlots.SetNum(NumEquipmentSlots);
	}
}

void AMMOPlayerCharacter::OnRep_AppearanceData(const FMMOAppearanceData& OldAppearanceData)
{
	BP_ApplyAppearance(AppearanceData);
}

void AMMOPlayerCharacter::UpdateAppearance(const FMMOAppearanceData& NewAppearance)
{
	AppearanceData = NewAppearance;

	if (HasAuthority())
	{
		OnRep_AppearanceData(NewAppearance);
	}
}

void AMMOPlayerCharacter::PlayEmote(EMMOEmoteType Emote, UAnimMontage* CustomMontage)
{
	if (HasAuthority())
	{
		MulticastPlayEmote(Emote, CustomMontage);
	}
	else
	{
		MulticastPlayEmote(Emote, CustomMontage);
	}
}

void AMMOPlayerCharacter::MulticastPlayEmote_Implementation(EMMOEmoteType Emote, UAnimMontage* CustomMontage)
{
	BP_PlayEmoteMontage(Emote, CustomMontage);
}

void AMMOPlayerCharacter::SetRestState(EMMOPlayerRestState NewRestState)
{
	if (HasAuthority())
	{
		RestState = NewRestState;
		MulticastSetRestState(NewRestState);
	}
	else
	{
		RestState = NewRestState;
	}
}

void AMMOPlayerCharacter::MulticastSetRestState_Implementation(EMMOPlayerRestState NewRestState)
{
	RestState = NewRestState;

	if (RestState == EMMOPlayerRestState::Sitting || RestState == EMMOPlayerRestState::Resting)
	{
		GetCharacterMovement()->DisableMovement();
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void AMMOPlayerCharacter::OnRep_PvPFlag(EMMOPvPFlag OldFlag)
{
}

void AMMOPlayerCharacter::SetPvPFlag(EMMOPvPFlag NewFlag)
{
	if (HasAuthority())
	{
		EMMOPvPFlag OldFlag = PvPFlag;
		PvPFlag = NewFlag;
		OnRep_PvPFlag(OldFlag);
	}
	else
	{
		ServerSetPvPFlag(NewFlag);
	}
}

void AMMOPlayerCharacter::ServerSetPvPFlag_Implementation(EMMOPvPFlag NewFlag)
{
	SetPvPFlag(NewFlag);
}

void AMMOPlayerCharacter::EquipItem(class UMMOEquipItem* Item)
{
	if (!Item || !HasAuthority())
	{
		return;
	}

	int32 SlotIndex = 0;
	for (int32 i = 0; i < EquipmentSlots.Num(); i++)
	{
		if (!EquipmentSlots[i])
		{
			SlotIndex = i;
			break;
		}
	}

	if (EquipmentSlots.IsValidIndex(SlotIndex))
	{
		EquipmentSlots[SlotIndex] = Item;
		BP_UpdateEquipmentMesh(SlotIndex, Item);
		OnEquipmentChanged.Broadcast(Item);
	}
}

void AMMOPlayerCharacter::UnEquipItem(int32 SlotIndex)
{
	if (!HasAuthority() || !EquipmentSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	UMMOEquipItem* Item = EquipmentSlots[SlotIndex];
	EquipmentSlots[SlotIndex] = nullptr;
	BP_UpdateEquipmentMesh(SlotIndex, nullptr);

	if (Item)
	{
		OnEquipmentChanged.Broadcast(Item);
	}
}

void AMMOPlayerCharacter::SetQuickSlotAbility(int32 SlotIndex, const FGameplayTag& AbilityTag)
{
	if (QuickSlotAbilities.IsValidIndex(SlotIndex))
	{
		QuickSlotAbilities[SlotIndex] = AbilityTag;
	}
	else if (SlotIndex >= 0)
	{
		QuickSlotAbilities.SetNum(SlotIndex + 1);
		QuickSlotAbilities[SlotIndex] = AbilityTag;
	}
}

void AMMOPlayerCharacter::ClearQuickSlotAbility(int32 SlotIndex)
{
	if (QuickSlotAbilities.IsValidIndex(SlotIndex))
	{
		QuickSlotAbilities[SlotIndex] = FGameplayTag();
	}
}

void AMMOPlayerCharacter::OnRep_Level(int32 OldLevel)
{
	Super::OnRep_Level(OldLevel);
	PlayerLevel = Level;
}

void AMMOPlayerCharacter::OnDeath()
{
	Super::OnDeath();

	if (HasAuthority())
	{
		PvPFlag = EMMOPvPFlag::Peaceful;
		RestState = EMMOPlayerRestState::Standing;
	}
}

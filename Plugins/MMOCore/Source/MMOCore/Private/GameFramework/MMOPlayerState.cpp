#include "GameFramework/MMOPlayerState.h"
#include "GameFramework/MMOGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AMMOPlayerState::AMMOPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;
	bAlwaysRelevant = true;

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicateMovement = false;

	PlayerLevel = 1;
	OnlineStatus = EMMOOnlineStatus::Offline;
	PlayerActivity = EMMOPlayerActivity::None;
	TotalPlayTime = 0.0f;
	CurrentInstanceID = INDEX_NONE;
}

void AMMOPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		PrimaryActorTick.SetTickFunctionEnable(true);
	}
}

void AMMOPlayerState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		AddPlayTime(DeltaTime);
	}
}

void AMMOPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMMOPlayerState, AccountID, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMOPlayerState, CharacterID, COND_OwnerOnly);
	DOREPLIFETIME(AMMOPlayerState, PlayerDisplayName);
	DOREPLIFETIME_CONDITION(AMMOPlayerState, OnlineStatus, COND_SkipOwner);
	DOREPLIFETIME(AMMOPlayerState, PlayerActivity);
	DOREPLIFETIME(AMMOPlayerState, PlayerLevel);
	DOREPLIFETIME_CONDITION(AMMOPlayerState, CurrentZoneID, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMOPlayerState, CurrentInstanceID, COND_OwnerOnly);
	DOREPLIFETIME(AMMOPlayerState, PartyID);
	DOREPLIFETIME(AMMOPlayerState, GuildID);
	DOREPLIFETIME_CONDITION(AMMOPlayerState, TotalPlayTime, COND_OwnerOnly);
}

void AMMOPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	AMMOPlayerState* MMOPS = Cast<AMMOPlayerState>(PlayerState);
	if (MMOPS)
	{
		MMOPS->AccountID = AccountID;
		MMOPS->CharacterID = CharacterID;
		MMOPS->PlayerDisplayName = PlayerDisplayName;
		MMOPS->OnlineStatus = OnlineStatus;
		MMOPS->PlayerActivity = PlayerActivity;
		MMOPS->PlayerLevel = PlayerLevel;
		MMOPS->CurrentZoneID = CurrentZoneID;
		MMOPS->CurrentInstanceID = CurrentInstanceID;
		MMOPS->PartyID = PartyID;
		MMOPS->GuildID = GuildID;
		MMOPS->TotalPlayTime = TotalPlayTime;
	}
}

void AMMOPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	PlayerDisplayName = GetPlayerName();
}

void AMMOPlayerState::SetAccountID(const FString& NewAccountID)
{
	AccountID = NewAccountID;
}

void AMMOPlayerState::SetCharacterID(const FString& NewCharacterID)
{
	CharacterID = NewCharacterID;
}

void AMMOPlayerState::SetPlayerDisplayName(const FString& NewDisplayName)
{
	PlayerDisplayName = NewDisplayName;
	SetPlayerName(NewDisplayName);
}

void AMMOPlayerState::SetOnlineStatus(EMMOOnlineStatus NewStatus)
{
	if (OnlineStatus != NewStatus)
	{
		EMMOOnlineStatus OldStatus = OnlineStatus;
		OnlineStatus = NewStatus;
		OnRep_OnlineStatus();
	}
}

void AMMOPlayerState::SetPlayerActivity(EMMOPlayerActivity NewActivity)
{
	if (PlayerActivity != NewActivity)
	{
		PlayerActivity = NewActivity;
		OnRep_PlayerActivity();
	}
}

void AMMOPlayerState::SetPlayerLevel(int32 NewLevel)
{
	if (PlayerLevel != NewLevel)
	{
		int32 OldLevel = PlayerLevel;
		PlayerLevel = NewLevel;
		OnPlayerLevelChanged.Broadcast(OldLevel, PlayerLevel);
	}
}

void AMMOPlayerState::SetCurrentZoneID(FName NewZoneID)
{
	CurrentZoneID = NewZoneID;
}

void AMMOPlayerState::SetCurrentInstanceID(int32 NewInstanceID)
{
	CurrentInstanceID = NewInstanceID;
}

void AMMOPlayerState::SetPartyID(const FString& NewPartyID)
{
	PartyID = NewPartyID;
}

void AMMOPlayerState::SetGuildID(const FString& NewGuildID)
{
	GuildID = NewGuildID;
}

void AMMOPlayerState::AddPlayTime(float DeltaTime)
{
	TotalPlayTime += DeltaTime;
}

FTimespan AMMOPlayerState::GetPlayTimeAsTimespan() const
{
	return FTimespan::FromSeconds(TotalPlayTime);
}

void AMMOPlayerState::Client_UpdateOnlineStatus_Implementation(EMMOOnlineStatus NewStatus)
{
	SetOnlineStatus(NewStatus);
}

bool AMMOPlayerState::Client_UpdateOnlineStatus_Validate(EMMOOnlineStatus NewStatus)
{
	return true;
}

void AMMOPlayerState::Client_UpdatePlayerLevel_Implementation(int32 NewLevel)
{
	SetPlayerLevel(NewLevel);
}

bool AMMOPlayerState::Client_UpdatePlayerLevel_Validate(int32 NewLevel)
{
	return NewLevel >= 1;
}

void AMMOPlayerState::OnRep_AccountID()
{
	UE_LOG(LogMMOCore, Verbose, TEXT("AccountID replicated: %s"), *AccountID);
}

void AMMOPlayerState::OnRep_CharacterID()
{
	UE_LOG(LogMMOCore, Verbose, TEXT("CharacterID replicated: %s"), *CharacterID);
}

void AMMOPlayerState::OnRep_PlayerDisplayName()
{
	SetPlayerName(PlayerDisplayName);
	OnDisplayNameChanged.Broadcast(PlayerDisplayName);
}

void AMMOPlayerState::OnRep_OnlineStatus()
{
	OnOnlineStatusChanged.Broadcast(OnlineStatus);

	if (OnlineStatus == EMMOOnlineStatus::Online)
	{
		UE_LOG(LogMMOCore, Log, TEXT("Player %s is now online"), *PlayerDisplayName);
	}
	else if (OnlineStatus == EMMOOnlineStatus::Offline)
	{
		UE_LOG(LogMMOCore, Log, TEXT("Player %s is now offline"), *PlayerDisplayName);
	}
}

void AMMOPlayerState::OnRep_PlayerActivity()
{
	OnPlayerActivityChanged.Broadcast(PlayerActivity);
}

void AMMOPlayerState::OnRep_PlayerLevel()
{
	UE_LOG(LogMMOCore, Verbose, TEXT("Player %s achieved level %d"), *PlayerDisplayName, PlayerLevel);
}

void AMMOPlayerState::OnRep_CurrentZone()
{
	UE_LOG(LogMMOCore, Verbose, TEXT("Player %s entered zone %s"), *PlayerDisplayName, *CurrentZoneID.ToString());
}

void AMMOPlayerState::OnRep_PartyID()
{
	UE_LOG(LogMMOCore, Verbose, TEXT("Player %s party ID set to %s"), *PlayerDisplayName, *PartyID);
}

void AMMOPlayerState::OnRep_GuildID()
{
	UE_LOG(LogMMOCore, Verbose, TEXT("Player %s guild ID set to %s"), *PlayerDisplayName, *GuildID);
}

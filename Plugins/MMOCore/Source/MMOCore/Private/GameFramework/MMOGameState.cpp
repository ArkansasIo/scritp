#include "GameFramework/MMOGameState.h"
#include "GameFramework/MMOPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AMMOGameState::AMMOGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	ServerTime = 0.0f;
	MaxPlayers = 1000;
	CurrentPlayerCount = 0;
	QueueSize = 0;
	ServerType = EMMOServerType::Normal;
	MatchState = EMMOMatchState::WaitingForPlayers;
	ServerPopulation = EMMOServerPopulation::Low;
}

void AMMOGameState::BeginPlay()
{
	Super::BeginPlay();

	ServerStartTime = GetWorld()->GetTimeSeconds();
	ServerVersion = TEXT("1.0.0");

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(TimeSyncTimerHandle, this, &AMMOGameState::BroadcastTimeSync, 30.0f, true);
	}
}

void AMMOGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		ServerTimestamp = GetWorld()->GetTimeSeconds();
	}
}

void AMMOGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMMOGameState, ServerName);
	DOREPLIFETIME(AMMOGameState, ServerID);
	DOREPLIFETIME(AMMOGameState, ServerType);
	DOREPLIFETIME_CONDITION(AMMOGameState, CurrentPlayerCount, COND_OwnerOnly);
	DOREPLIFETIME(AMMOGameState, MaxPlayers);
	DOREPLIFETIME_CONDITION(AMMOGameState, QueueSize, COND_OwnerOnly);
	DOREPLIFETIME(AMMOGameState, MatchState);
	DOREPLIFETIME(AMMOGameState, ServerPopulation);
	DOREPLIFETIME(AMMOGameState, ServerTimestamp);
	DOREPLIFETIME(AMMOGameState, ServerVersion);
	DOREPLIFETIME(AMMOGameState, OnlinePlayerNames);
}

void AMMOGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (HasAuthority())
	{
		CurrentPlayerCount = PlayerArray.Num();
		UpdateServerPopulation();

		AMMOPlayerState* MMOps = Cast<AMMOPlayerState>(PlayerState);
		if (MMOps)
		{
			OnlinePlayerNames.Add(MMOps->GetPlayerDisplayName());
		}
	}
}

void AMMOGameState::RemovePlayerState(APlayerState* PlayerState)
{
	if (HasAuthority())
	{
		AMMOPlayerState* MMOps = Cast<AMMOPlayerState>(PlayerState);
		if (MMOps)
		{
			OnlinePlayerNames.Remove(MMOps->GetPlayerDisplayName());
		}
	}

	Super::RemovePlayerState(PlayerState);

	if (HasAuthority())
	{
		CurrentPlayerCount = PlayerArray.Num();
		UpdateServerPopulation();
	}
}

float AMMOGameState::GetServerUptime() const
{
	if (ServerStartTime > 0.0f)
	{
		return GetWorld()->GetTimeSeconds() - ServerStartTime;
	}
	return 0.0f;
}

void AMMOGameState::SetServerName(const FString& NewName)
{
	ServerName = NewName;
}

void AMMOGameState::SetServerType(EMMOServerType NewType)
{
	ServerType = NewType;
}

void AMMOGameState::SetMatchState(EMMOMatchState NewState)
{
	if (MatchState != NewState)
	{
		EMMOMatchState OldState = MatchState;
		MatchState = NewState;
		OnMatchStateChanged.Broadcast(NewState);
		OnMatchStateChangedEvent(OldState, NewState);
		Multicast_MatchStateChanged(NewState);
	}
}

void AMMOGameState::SetMaxPlayers(int32 NewMax)
{
	MaxPlayers = NewMax;
	UpdateServerPopulation();
}

void AMMOGameState::SetQueueSize(int32 NewQueueSize)
{
	QueueSize = NewQueueSize;
}

void AMMOGameState::SendServerAnnouncement(const FString& Message, float Duration, bool bUrgent)
{
	FMMOServerAnnouncement Announcement;
	Announcement.Message = Message;
	Announcement.Duration = Duration;
	Announcement.bIsUrgent = bUrgent;

	Multicast_ServerAnnouncement(Announcement);
}

void AMMOGameState::Multicast_ServerAnnouncement_Implementation(const FMMOServerAnnouncement& Announcement)
{
	OnServerAnnouncement.Broadcast(Announcement);
	OnServerAnnouncementReceived(Announcement);

	UE_LOG(LogMMOCore, Log, TEXT("[SERVER ANNOUNCEMENT] %s"), *Announcement.Message);
}

bool AMMOGameState::Multicast_ServerAnnouncement_Validate(const FMMOServerAnnouncement& Announcement)
{
	return true;
}

void AMMOGameState::Multicast_MatchStateChanged_Implementation(EMMOMatchState NewState)
{
	UE_LOG(LogMMOCore, Log, TEXT("Match state changed to: %s"), *UEnum::GetValueAsString(NewState));
}

bool AMMOGameState::Multicast_MatchStateChanged_Validate(EMMOMatchState NewState)
{
	return true;
}

void AMMOGameState::Multicast_TimeSync_Implementation(float ServerTime)
{
	ServerTimestamp = ServerTime;
}

bool AMMOGameState::Multicast_TimeSync_Validate(float ServerTime)
{
	return true;
}

void AMMOGameState::BroadcastTimeSync()
{
	if (HasAuthority())
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		ServerTimestamp = CurrentTime;
		Multicast_TimeSync(CurrentTime);
	}
}

void AMMOGameState::OnRep_MatchState(EMMOMatchState OldState)
{
	OnMatchStateChanged.Broadcast(MatchState);
	OnMatchStateChangedEvent(OldState, MatchState);
}

void AMMOGameState::OnRep_ServerPopulation()
{
	OnServerPopulationChanged.Broadcast(ServerPopulation);
}

void AMMOGameState::OnRep_ServerName()
{
	UE_LOG(LogMMOCore, Log, TEXT("Server name updated: %s"), *ServerName);
}

void AMMOGameState::OnRep_PlayerCount()
{
	UpdateServerPopulation();
}

void AMMOGameState::UpdateServerPopulation()
{
	if (MaxPlayers > 0)
	{
		float Ratio = (float)CurrentPlayerCount / (float)MaxPlayers;

		if (Ratio >= 0.9f)
		{
			ServerPopulation = EMMOServerPopulation::Full;
		}
		else if (Ratio >= 0.6f)
		{
			ServerPopulation = EMMOServerPopulation::High;
		}
		else if (Ratio >= 0.3f)
		{
			ServerPopulation = EMMOServerPopulation::Medium;
		}
		else
		{
			ServerPopulation = EMMOServerPopulation::Low;
		}
	}
}

#include "GameFramework/MMOGameMode.h"
#include "GameFramework/MMOPlayerController.h"
#include "GameFramework/MMOPlayerState.h"
#include "GameFramework/MMOGameState.h"
#include "Engine/MMODeveloperSettings.h"
#include "Engine/World.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameSession.h"

AMMOGameMode::AMMOGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;

	GameStateClass = AMMOGameState::StaticClass();
	PlayerControllerClass = AMMOPlayerController::StaticClass();
	PlayerStateClass = AMMOPlayerState::StaticClass();
	DefaultPawnClass = APawn::StaticClass();
	HUDClass = AHUD::StaticClass();
}

void AMMOGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AMMOGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	const UMMODeveloperSettings* Settings = UMMODeveloperSettings::GetDeveloperSettings();
	if (Settings)
	{
		MaxPlayerCount = Settings->MaxPlayersPerZone;
		ServerTickRate = Settings->ServerTickRate;
	}

	UpdateServerTickRate();
}

void AMMOGameMode::InitGameState()
{
	Super::InitGameState();
}

void AMMOGameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(QueueTimerHandle, this, &AMMOGameMode::ProcessPlayerQueue, 1.0f, true);
}

void AMMOGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMMOGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(TickTimerHandle);
	GetWorldTimerManager().ClearTimer(QueueTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void AMMOGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
	{
		return;
	}

	if (GetNumPlayers() >= MaxPlayerCount)
	{
		ErrorMessage = TEXT("Server is full.");
		return;
	}
}

APlayerController* AMMOGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	APlayerController* PC = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

	if (PC)
	{
		AMMOPlayerController* MMOpc = Cast<AMMOPlayerController>(PC);
		if (MMOpc)
		{
			MMOpc->SetLoginState(EMMOLoginState::Unauthenticated);
		}
	}

	return PC;
}

void AMMOGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AMMOPlayerController* MMOpc = Cast<AMMOPlayerController>(NewPlayer);
	if (MMOpc)
	{
		OnPlayerJoined.Broadcast(MMOpc);
		OnPlayerPostLogin(MMOpc);
	}
}

void AMMOGameMode::Logout(AController* Exiting)
{
	AMMOPlayerController* MMOpc = Cast<AMMOPlayerController>(Exiting);
	if (MMOpc)
	{
		OnPlayerLeft.Broadcast(MMOpc);
		OnPlayerLogout(MMOpc);
	}

	Super::Logout(Exiting);
}

bool AMMOGameMode::PlayerCanRestart(APlayerController* PlayerController)
{
	AMMOPlayerController* MMOpc = Cast<AMMOPlayerController>(PlayerController);
	if (MMOpc && MMOpc->GetLoginState() != EMMOLoginState::Playing)
	{
		return false;
	}

	return Super::PlayerCanRestart(PlayerController);
}

void AMMOGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	AMMOPlayerController* MMOpc = Cast<AMMOPlayerController>(NewPlayer);
	if (MMOpc)
	{
		MMOpc->SetLoginState(EMMOLoginState::Playing);
	}
}

void AMMOGameMode::RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot)
{
	Super::RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
}

AActor* AMMOGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

bool AMMOGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return Super::ShouldSpawnAtStartSpot(Player);
}

void AMMOGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void AMMOGameMode::SetServerTickRate(float NewTickRate)
{
	ServerTickRate = FMath::Clamp(NewTickRate, 1.0f, 120.0f);
	UpdateServerTickRate();
}

bool AMMOGameMode::RegisterZone(FName ZoneID, const FMMOZoneInfo& ZoneInfo)
{
	if (RegisteredZones.Contains(ZoneID))
	{
		UE_LOG(LogMMOCore, Warning, TEXT("Zone %s already registered"), *ZoneID.ToString());
		return false;
	}

	RegisteredZones.Add(ZoneID, ZoneInfo);
	UE_LOG(LogMMOCore, Log, TEXT("Zone registered: %s"), *ZoneID.ToString());
	return true;
}

bool AMMOGameMode::UnregisterZone(FName ZoneID)
{
	if (!RegisteredZones.Contains(ZoneID))
	{
		UE_LOG(LogMMOCore, Warning, TEXT("Zone %s not found for unregister"), *ZoneID.ToString());
		return false;
	}

	RegisteredZones.Remove(ZoneID);
	UE_LOG(LogMMOCore, Log, TEXT("Zone unregistered: %s"), *ZoneID.ToString());
	return true;
}

bool AMMOGameMode::TeleportPlayerToZone(AMMOPlayerController* Player, FName TargetZoneID, FName TargetPlayerStartTag)
{
	if (!Player)
	{
		return false;
	}

	FMMOZoneInfo* ZoneInfo = RegisteredZones.Find(TargetZoneID);
	if (!ZoneInfo)
	{
		UE_LOG(LogMMOCore, Warning, TEXT("Target zone %s not found for teleport"), *TargetZoneID.ToString());
		return false;
	}

	HandleZoneTransition(Player, TargetZoneID, TargetPlayerStartTag);
	return true;
}

void AMMOGameMode::ServerTravelToMap(const FString& MapURL, bool bSeamless)
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogMMOCore, Log, TEXT("Server traveling to: %s (Seamless: %s)"), *MapURL, bSeamless ? TEXT("True") : TEXT("False"));

	if (bSeamless && bEnableSeamlessTravel)
	{
		GetWorld()->SeamlessTravel(MapURL);
	}
	else
	{
		GetWorld()->ServerTravel(MapURL);
	}
}

void AMMOGameMode::SeamlessTravelToZone(FName ZoneID)
{
	FMMOZoneInfo* ZoneInfo = RegisteredZones.Find(ZoneID);
	if (!ZoneInfo)
	{
		UE_LOG(LogMMOCore, Error, TEXT("Cannot travel to unknown zone: %s"), *ZoneID.ToString());
		return;
	}

	if (ZoneInfo->ZoneLevel.IsNull())
	{
		UE_LOG(LogMMOCore, Error, TEXT("Zone %s has no level assigned"), *ZoneID.ToString());
		return;
	}

	FString MapURL = ZoneInfo->ZoneLevel.ToString();
	ServerTravelToMap(MapURL, true);
}

void AMMOGameMode::BroadcastServerMessage(const FString& Message)
{
	AMMOGameState* MMOgs = GetGameState<AMMOGameState>();
	if (MMOgs)
	{
		MMOgs->SendServerAnnouncement(Message, 10.0f, false);
	}
}

int32 AMMOGameMode::GetCurrentPlayerCount() const
{
	return GetNumPlayers();
}

bool AMMOGameMode::CanJoinServer(AMMOPlayerController* Player, FString& OutErrorMessage) const
{
	if (!Player)
	{
		OutErrorMessage = TEXT("Invalid player controller.");
		return false;
	}

	if (GetNumPlayers() >= MaxPlayerCount)
	{
		OutErrorMessage = TEXT("Server is at maximum capacity.");
		return false;
	}

	return true;
}

FString AMMOGameMode::GetServerInfoString() const
{
	return FString::Printf(TEXT("Players: %d/%d | TickRate: %.1f"), GetNumPlayers(), MaxPlayerCount, ServerTickRate);
}

void AMMOGameMode::UpdateServerTickRate()
{
	if (ServerTickRate > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TickTimerHandle, this, &AMMOGameMode::Tick, 1.0f / ServerTickRate, true);
	}
}

void AMMOGameMode::ProcessPlayerQueue()
{
	if (PlayerQueue.Num() == 0)
	{
		return;
	}

	AMMOPlayerController* NextPlayer = PlayerQueue[0];
	PlayerQueue.RemoveAt(0);

	if (NextPlayer && IsValid(NextPlayer))
	{
		FString ErrorMessage;
		if (CanJoinServer(NextPlayer, ErrorMessage))
		{
			RestartPlayer(NextPlayer);
		}
	}
}

void AMMOGameMode::HandleZoneTransition(AMMOPlayerController* Player, FName TargetZoneID, FName PlayerStartTag)
{
	if (!Player || !HasAuthority())
	{
		return;
	}

	AMMOPlayerState* MMOps = Player->GetPlayerState<AMMOPlayerState>();
	if (MMOps)
	{
		MMOps->SetCurrentZoneID(TargetZoneID);
	}

	FMMOZoneInfo* ZoneInfo = RegisteredZones.Find(TargetZoneID);
	if (ZoneInfo && !ZoneInfo->ZoneLevel.IsNull())
	{
		FString MapURL = ZoneInfo->ZoneLevel.ToString();
		if (!PlayerStartTag.IsNone())
		{
			MapURL += FString::Printf(TEXT("?PlayerStart=%s"), *PlayerStartTag.ToString());
		}
		Player->ClientTravel(MapURL, TRAVEL_Relative);
	}

	OnZonePopulationChanged.Broadcast(TargetZoneID);
}

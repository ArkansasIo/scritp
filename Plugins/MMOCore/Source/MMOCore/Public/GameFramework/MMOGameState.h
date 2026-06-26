#pragma once

#include "MMOCore.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTagContainer.h"
#include "MMOGameState.generated.h"

class AMMOPlayerState;

UENUM(BlueprintType)
enum class EMMOServerType : uint8
{
	Normal,
	PvP,
	Roleplay,
	Event
};

UENUM(BlueprintType)
enum class EMMOMatchState : uint8
{
	WaitingForPlayers,
	Entering,
	InProgress,
	Warmup,
	Paused,
	Ending,
	ShuttingDown
};

UENUM(BlueprintType)
enum class EMMOServerPopulation : uint8
{
	Low,
	Medium,
	High,
	Full
};

USTRUCT(BlueprintType)
struct FMMOServerAnnouncement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Announcement")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "Announcement")
	float Duration = 10.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Announcement")
	bool bIsUrgent = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchStateChanged, EMMOMatchState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnServerPopulationChanged, EMMOServerPopulation, NewPopulation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnServerAnnouncement, const FMMOServerAnnouncement&, Announcement);

UCLASS(Blueprintable, BlueprintType)
class MMOCORE_API AMMOGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMMOGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UFUNCTION(BlueprintPure, Category = "MMO|Server")
	FString GetServerName() const { return ServerName; }

	UFUNCTION(BlueprintPure, Category = "MMO|Server")
	FString GetServerID() const { return ServerID; }

	UFUNCTION(BlueprintPure, Category = "MMO|Server")
	EMMOServerType GetServerType() const { return ServerType; }

	UFUNCTION(BlueprintPure, Category = "MMO|Server")
	int32 GetCurrentPlayerCount() const { return CurrentPlayerCount; }

	UFUNCTION(BlueprintPure, Category = "MMO|Server")
	int32 GetMaxPlayers() const { return MaxPlayers; }

	UFUNCTION(BlueprintPure, Category = "MMO|Server")
	int32 GetQueueSize() const { return QueueSize; }

	UFUNCTION(BlueprintPure, Category = "MMO|Server")
	float GetServerUptime() const;

	UFUNCTION(BlueprintPure, Category = "MMO|Server")
	FString GetServerVersion() const { return ServerVersion; }

	UFUNCTION(BlueprintPure, Category = "MMO|Server")
	EMMOMatchState GetMatchState() const { return MatchState; }

	UFUNCTION(BlueprintPure, Category = "MMO|Server")
	EMMOServerPopulation GetServerPopulation() const { return ServerPopulation; }

	UFUNCTION(BlueprintPure, Category = "MMO|Time")
	float GetReplicatedServerTime() const { return ServerTimestamp; }

	UFUNCTION(BlueprintCallable, Category = "MMO|Server")
	void SetServerName(const FString& NewName);

	UFUNCTION(BlueprintCallable, Category = "MMO|Server")
	void SetServerType(EMMOServerType NewType);

	UFUNCTION(BlueprintCallable, Category = "MMO|Server")
	void SetMatchState(EMMOMatchState NewState);

	UFUNCTION(BlueprintCallable, Category = "MMO|Server")
	void SetMaxPlayers(int32 NewMax);

	UFUNCTION(BlueprintCallable, Category = "MMO|Server")
	void SetQueueSize(int32 NewQueueSize);

	UFUNCTION(BlueprintCallable, Category = "MMO|Announcements")
	void SendServerAnnouncement(const FString& Message, float Duration = 10.0f, bool bUrgent = false);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multicast_ServerAnnouncement(const FMMOServerAnnouncement& Announcement);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multicast_MatchStateChanged(EMMOMatchState NewState);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multicast_TimeSync(float ServerTime);

	UFUNCTION(BlueprintCallable, Category = "MMO|Time")
	void BroadcastTimeSync();

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Event")
	void OnMatchStateChangedEvent(EMMOMatchState OldState, EMMOMatchState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Event")
	void OnServerAnnouncementReceived(const FMMOServerAnnouncement& Announcement);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Server")
	FString ServerName;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Server")
	FString ServerID;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Server")
	EMMOServerType ServerType = EMMOServerType::Normal;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Server")
	int32 CurrentPlayerCount = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Server")
	int32 MaxPlayers = 1000;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Server")
	int32 QueueSize = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Server")
	EMMOMatchState MatchState = EMMOMatchState::WaitingForPlayers;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Server")
	EMMOServerPopulation ServerPopulation = EMMOServerPopulation::Low;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Time")
	float ServerTimestamp = 0.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Server")
	FString ServerVersion;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Players")
	TArray<FString> OnlinePlayerNames;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnMatchStateChanged OnMatchStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnServerPopulationChanged OnServerPopulationChanged;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnServerAnnouncement OnServerAnnouncement;

protected:
	UFUNCTION()
	void OnRep_MatchState(EMMOMatchState OldState);

	UFUNCTION()
	void OnRep_ServerPopulation();

	UFUNCTION()
	void OnRep_ServerName();

	UFUNCTION()
	void OnRep_PlayerCount();

	void UpdateServerPopulation();

	float ServerStartTime = 0.0f;

	UPROPERTY()
	FTimerHandle TimeSyncTimerHandle;
};

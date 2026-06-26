#pragma once

#include "MMOCore.h"
#include "GameFramework/GameModeBase.h"
#include "MMOGameMode.generated.h"

class AMMOPlayerController;
class AMMOPlayerState;
class AMMOGameState;
class AMMOPawn;
class AMMOHUD;

UENUM(BlueprintType)
enum class EMMOZoneType : uint8
{
	Overworld,
	Instance,
	Dungeon,
	Raid,
	Arena,
	City,
	Battleground,
	Indoor
};

USTRUCT(BlueprintType)
struct FMMOZoneInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Zone")
	FName ZoneID;

	UPROPERTY(BlueprintReadOnly, Category = "Zone")
	FString ZoneName;

	UPROPERTY(BlueprintReadOnly, Category = "Zone")
	EMMOZoneType ZoneType = EMMOZoneType::Overworld;

	UPROPERTY(BlueprintReadOnly, Category = "Zone")
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Zone")
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Zone")
	TSoftObjectPtr<UWorld> ZoneLevel;
};

USTRUCT(BlueprintType)
struct FMMOServerInstance
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Server")
	FString ServerName;

	UPROPERTY(BlueprintReadOnly, Category = "Server")
	int32 InstanceID = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Server")
	FName MapName;

	UPROPERTY(BlueprintReadOnly, Category = "Server")
	int32 PlayerCount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoined, AMMOPlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, AMMOPlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZonePopulationChanged, FName, ZoneID);

UCLASS(Blueprintable, BlueprintType, config = Game)
class MMCORE_API AMMOGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMMOGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin Actor Interface
	virtual void PreInitializeComponents() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End Actor Interface

	//~ Begin GameModeBase Interface
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual bool PlayerCanRestart(APlayerController* PlayerController) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot) override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	//~ End GameModeBase Interface

	UFUNCTION(BlueprintCallable, Category = "MMO|Server")
	void SetServerTickRate(float NewTickRate);

	UFUNCTION(BlueprintCallable, Category = "MMO|Zone")
	bool RegisterZone(FName ZoneID, const FMMOZoneInfo& ZoneInfo);

	UFUNCTION(BlueprintCallable, Category = "MMO|Zone")
	bool UnregisterZone(FName ZoneID);

	UFUNCTION(BlueprintCallable, Category = "MMO|Zone")
	bool TeleportPlayerToZone(AMMOPlayerController* Player, FName TargetZoneID, FName TargetPlayerStartTag = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "MMO|Travel")
	void ServerTravelToMap(const FString& MapURL, bool bSeamless = true);

	UFUNCTION(BlueprintCallable, Category = "MMO|Travel")
	void SeamlessTravelToZone(FName ZoneID);

	UFUNCTION(Exec, Category = "MMO|Admin")
	void BroadcastServerMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "MMO|Player")
	int32 GetCurrentPlayerCount() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Player")
	bool CanJoinServer(AMMOPlayerController* Player, FString& OutErrorMessage) const;

	UFUNCTION(BlueprintPure, Category = "MMO|Server")
	FString GetServerInfoString() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Event")
	void OnPlayerPreLogin(const FString& PlayerID, bool& bAllowLogin, FString& DenyReason);

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Event")
	void OnPlayerPostLogin(AMMOPlayerController* NewPlayer);

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Event")
	void OnPlayerLogout(AMMOPlayerController* ExitingPlayer);

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Event")
	void OnZoneFull(FName ZoneID);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Config", config)
	int32 MaxPlayerCount = 1000;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Config", config)
	int32 MinLevelToJoin = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Config", config)
	float ServerTickRate = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Config", config)
	float PlayerRespawnTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Config", config)
	bool bEnableSeamlessTravel = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Config", config)
	TSoftObjectPtr<UWorld> LobbyMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Classes")
	TSubclassOf<AMMOPlayerController> MMOPlayerControllerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Classes")
	TSubclassOf<AMMOPlayerState> MMOPlayerStateClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Classes")
	TSubclassOf<AMMOGameState> MMOGameStateClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Classes")
	TSubclassOf<APawn> MMOPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Classes")
	TSubclassOf<AHUD> MMOHUDClass;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Zones")
	TMap<FName, FMMOZoneInfo> RegisteredZones;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnPlayerJoined OnPlayerJoined;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnPlayerLeft OnPlayerLeft;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnZonePopulationChanged OnZonePopulationChanged;

	void UpdateServerTickRate();
	void ProcessPlayerQueue();
	void HandleZoneTransition(AMMOPlayerController* Player, FName TargetZoneID, FName PlayerStartTag);

	UPROPERTY()
	TArray<AMMOPlayerController*> PlayerQueue;

	UPROPERTY()
	FTimerHandle TickTimerHandle;

	UPROPERTY()
	FTimerHandle QueueTimerHandle;
};

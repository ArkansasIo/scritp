#pragma once

#include "MMOCore.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "MMOPlayerState.generated.h"

class AMMOGameState;

UENUM(BlueprintType)
enum class EMMOOnlineStatus : uint8
{
	Online,
	Away,
	Busy,
	Offline
};

UENUM(BlueprintType)
enum class EMMOPlayerActivity : uint8
{
	None,
	Combat,
	Trading,
	Crafting,
	Gathering,
	Questing,
	Socializing,
	AFK
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOnlineStatusChanged, EMMOOnlineStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerActivityChanged, EMMOPlayerActivity, NewActivity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLevelChanged, int32, OldLevel, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDisplayNameChanged, const FString&, NewDisplayName);

UCLASS(Blueprintable, BlueprintType)
class MMOCORE_API AMMOPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMMOPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OnRep_PlayerName() override;

	void SetAccountID(const FString& NewAccountID);
	void SetCharacterID(const FString& NewCharacterID);
	void SetPlayerDisplayName(const FString& NewDisplayName);
	void SetOnlineStatus(EMMOOnlineStatus NewStatus);
	void SetPlayerActivity(EMMOPlayerActivity NewActivity);
	void SetPlayerLevel(int32 NewLevel);
	void SetCurrentZoneID(FName NewZoneID);
	void SetCurrentInstanceID(int32 NewInstanceID);
	void SetPartyID(const FString& NewPartyID);
	void SetGuildID(const FString& NewGuildID);
	void AddPlayTime(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	FString GetAccountID() const { return AccountID; }

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	FString GetCharacterID() const { return CharacterID; }

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	FString GetPlayerDisplayName() const { return PlayerDisplayName; }

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	EMMOOnlineStatus GetOnlineStatus() const { return OnlineStatus; }

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	EMMOPlayerActivity GetPlayerActivity() const { return PlayerActivity; }

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	FName GetCurrentZoneID() const { return CurrentZoneID; }

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	int32 GetCurrentInstanceID() const { return CurrentInstanceID; }

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	FString GetPartyID() const { return PartyID; }

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	FString GetGuildID() const { return GuildID; }

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	int32 GetPlayerLevel() const { return PlayerLevel; }

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	float GetTotalPlayTime() const { return TotalPlayTime; }

	UFUNCTION(BlueprintPure, Category = "MMO|PlayerState")
	FTimespan GetPlayTimeAsTimespan() const;

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_UpdateOnlineStatus(EMMOOnlineStatus NewStatus);

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_UpdatePlayerLevel(int32 NewLevel);

protected:
	UFUNCTION()
	void OnRep_AccountID();

	UFUNCTION()
	void OnRep_CharacterID();

	UFUNCTION()
	void OnRep_PlayerDisplayName();

	UFUNCTION()
	void OnRep_OnlineStatus();

	UFUNCTION()
	void OnRep_PlayerActivity();

	UFUNCTION()
	void OnRep_PlayerLevel();

	UFUNCTION()
	void OnRep_CurrentZone();

	UFUNCTION()
	void OnRep_PartyID();

	UFUNCTION()
	void OnRep_GuildID();

	UPROPERTY(ReplicatedUsing = OnRep_AccountID, BlueprintReadOnly, Category = "MMO|Account")
	FString AccountID;

	UPROPERTY(ReplicatedUsing = OnRep_CharacterID, BlueprintReadOnly, Category = "MMO|Character")
	FString CharacterID;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerDisplayName, BlueprintReadOnly, Category = "MMO|Character")
	FString PlayerDisplayName;

	UPROPERTY(ReplicatedUsing = OnRep_OnlineStatus, BlueprintReadOnly, Category = "MMO|Status")
	EMMOOnlineStatus OnlineStatus = EMMOOnlineStatus::Offline;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerActivity, BlueprintReadOnly, Category = "MMO|Status")
	EMMOPlayerActivity PlayerActivity = EMMOPlayerActivity::None;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerLevel, BlueprintReadOnly, Category = "MMO|Character")
	int32 PlayerLevel = 1;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentZone, BlueprintReadOnly, Category = "MMO|Zone")
	FName CurrentZoneID;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Zone")
	int32 CurrentInstanceID = INDEX_NONE;

	UPROPERTY(ReplicatedUsing = OnRep_PartyID, BlueprintReadOnly, Category = "MMO|Social")
	FString PartyID;

	UPROPERTY(ReplicatedUsing = OnRep_GuildID, BlueprintReadOnly, Category = "MMO|Social")
	FString GuildID;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Stats")
	float TotalPlayTime = 0.0f;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnOnlineStatusChanged OnOnlineStatusChanged;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnPlayerActivityChanged OnPlayerActivityChanged;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnPlayerLevelChanged OnPlayerLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnDisplayNameChanged OnDisplayNameChanged;
};

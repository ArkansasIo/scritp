#pragma once

#include "MMOCore.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "MMOPlayerController.generated.h"

class AMMOGameMode;
class AMMOPlayerState;
class AMMOHUD;

UENUM(BlueprintType)
enum class EMMOLoginState : uint8
{
	Unauthenticated,
	LoggingIn,
	Authenticated,
	CharacterSelect,
	Spawning,
	Playing
};

UENUM(BlueprintType)
enum class EMMOInputMode : uint8
{
	GameOnly,
	UIOnly,
	GameAndUI,
	Chat
};

UENUM(BlueprintType)
enum class EMMOChatChannel : uint8
{
	Global,
	Local,
	Say,
	Yell,
	Party,
	Guild,
	Whisper,
	System,
	Trade,
	LocalDefense,
	World,
	Custom
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginStateChanged, EMMOLoginState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatMessageReceived, EMMOChatChannel, Channel, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartyInviteReceived, const FString&, PartyLeaderName, const FString&, PartyID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuildInviteReceived, const FString&, GuildName, const FString&, GuildID);

UCLASS(Blueprintable, BlueprintType, config = Game)
class MMOCORE_API AMMOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMMOPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerState() override;
	virtual void Destroyed() override;

	void SetLoginState(EMMOLoginState NewState);
	UFUNCTION(BlueprintCallable, Category = "MMO|Login")
	EMMOLoginState GetLoginState() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Login")
	void RequestLogin(const FString& Username, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "MMO|Login")
	void RequestCreateAccount(const FString& Username, const FString& Password, const FString& Email);

	UFUNCTION(BlueprintCallable, Category = "MMO|Login")
	void RequestCharacterList();

	UFUNCTION(BlueprintCallable, Category = "MMO|Login")
	void RequestCreateCharacter(const FString& CharacterName, const FString& ClassName);

	UFUNCTION(BlueprintCallable, Category = "MMO|Login")
	void RequestDeleteCharacter(const FString& CharacterID);

	UFUNCTION(BlueprintCallable, Category = "MMO|Login")
	void RequestSelectCharacter(const FString& CharacterID);

	UFUNCTION(BlueprintCallable, Category = "MMO|Login")
	void RequestLogout();

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_LoginResponse(bool bSuccess, const FString& ErrorMessage);

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_CharacterListResponse(const TArray<FString>& CharacterNames, const TArray<FString>& CharacterIDs);

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_CharacterCreated(bool bSuccess, const FString& CharacterID, const FString& ErrorMessage);

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_CharacterDeleted(bool bSuccess, const FString& ErrorMessage);

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_EnterWorld(const FString& SaveGameData);

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_LoginFailed(const FString& ErrorMessage);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestLogin(const FString& Username, const FString& Password);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestCreateAccount(const FString& Username, const FString& Password, const FString& Email);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestCharacterList();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestCreateCharacter(const FString& CharacterName, const FString& ClassName);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestDeleteCharacter(const FString& CharacterID);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestSelectCharacter(const FString& CharacterID);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestLogout();

	UFUNCTION(BlueprintCallable, Category = "MMO|Chat")
	void SendChatMessage(EMMOChatChannel Channel, const FString& Message);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendChatMessage(EMMOChatChannel Channel, const FString& Message);

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_ReceiveChatMessage(EMMOChatChannel Channel, const FString& SenderName, const FString& Message);

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_SystemMessage(const FString& Message);

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_PartyInvite(const FString& PartyLeaderName, const FString& PartyID);

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_GuildInvite(const FString& GuildName, const FString& GuildID);

	UFUNCTION(BlueprintCallable, Category = "MMO|Input")
	void SetInputModeForState(EMMOInputMode NewInputMode);

	UFUNCTION(BlueprintCallable, Category = "MMO|Input")
	void ShowUI(bool bShowCursor, bool bIgnoreInput);

	UFUNCTION(BlueprintCallable, Category = "MMO|Input")
	void HideUI();

	UFUNCTION(BlueprintPure, Category = "MMO|UI")
	EMMOInputMode GetCurrentInputMode() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Network")
	float GetServerTime() const;

	UFUNCTION(Client, Reliable, WithValidation)
	void Client_SyncServerTime(float ServerTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Event")
	void OnLoginStateChangedEvent(EMMOLoginState OldState, EMMOLoginState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Event")
	void OnChatMessageReceivedEvent(EMMOChatChannel Channel, const FString& SenderName, const FString& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Event")
	void OnSystemMessageReceived(const FString& Message);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Player")
	FString PlayerDisplayName;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Player")
	FString CharacterID;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Player")
	FString AccountID;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|UI")
	EMMOLoginState LoginState = EMMOLoginState::Unauthenticated;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnLoginStateChanged OnLoginStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnChatMessageReceived OnChatMessageReceived;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnPartyInviteReceived OnPartyInviteReceived;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnGuildInviteReceived OnGuildInviteReceived;

protected:
	void HandleLoginStateChanged(EMMOLoginState OldState, EMMOLoginState NewState);
	void HandleChatMessage(EMMOChatChannel Channel, const FString& SenderName, const FString& Message);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMO|UI")
	EMMOInputMode CurrentInputMode = EMMOInputMode::GameOnly;

	UPROPERTY(config)
	float LoginTimeout = 30.0f;

	UPROPERTY()
	FTimerHandle LoginTimeoutHandle;

	UPROPERTY()
	FTimerHandle ReconnectHandle;

	UFUNCTION()
	void OnLoginTimeout();
};

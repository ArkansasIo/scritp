#include "GameFramework/MMOPlayerController.h"
#include "GameFramework/MMOGameMode.h"
#include "GameFramework/MMOPlayerState.h"
#include "GameFramework/MMOGameState.h"
#include "Engine/MMODeveloperSettings.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetDriver.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameSession.h"

AMMOPlayerController::AMMOPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;

	NetUpdateFrequency = 100.0f;
	MinNetUpdateFrequency = 10.0f;

	bReplicates = true;
	bAlwaysRelevant = true;
}

void AMMOPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		SetLoginState(EMMOLoginState::Unauthenticated);
	}
}

void AMMOPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LoginTimeoutHandle);
	GetWorldTimerManager().ClearTimer(ReconnectHandle);

	Super::EndPlay(EndPlayReason);
}

void AMMOPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMMOPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMMOPlayerController, PlayerDisplayName, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMOPlayerController, CharacterID, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMMOPlayerController, AccountID, COND_OwnerOnly);
}

void AMMOPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void AMMOPlayerController::Destroyed()
{
	Super::Destroyed();
}

void AMMOPlayerController::SetLoginState(EMMOLoginState NewState)
{
	if (LoginState == NewState)
	{
		return;
	}

	EMMOLoginState OldState = LoginState;
	LoginState = NewState;

	HandleLoginStateChanged(OldState, NewState);

	OnLoginStateChanged.Broadcast(NewState);
	OnLoginStateChangedEvent(OldState, NewState);
}

EMMOLoginState AMMOPlayerController::GetLoginState() const
{
	return LoginState;
}

void AMMOPlayerController::RequestLogin(const FString& Username, const FString& Password)
{
	if (Username.IsEmpty() || Password.IsEmpty())
	{
		Client_LoginResponse(false, TEXT("Username and password are required."));
		return;
	}

	SetLoginState(EMMOLoginState::LoggingIn);
	Server_RequestLogin(Username, Password);

	GetWorldTimerManager().SetTimer(LoginTimeoutHandle, this, &AMMOPlayerController::OnLoginTimeout, LoginTimeout, false);
}

void AMMOPlayerController::RequestCreateAccount(const FString& Username, const FString& Password, const FString& Email)
{
	if (Username.IsEmpty() || Password.IsEmpty() || Email.IsEmpty())
	{
		Client_LoginResponse(false, TEXT("Username, password, and email are required."));
		return;
	}

	Server_RequestCreateAccount(Username, Password, Email);
}

void AMMOPlayerController::RequestCharacterList()
{
	Server_RequestCharacterList();
}

void AMMOPlayerController::RequestCreateCharacter(const FString& CharacterName, const FString& ClassName)
{
	if (CharacterName.IsEmpty())
	{
		Client_CharacterCreated(false, TEXT(""), TEXT("Character name is required."));
		return;
	}

	Server_RequestCreateCharacter(CharacterName, ClassName);
}

void AMMOPlayerController::RequestDeleteCharacter(const FString& CharacterID)
{
	if (CharacterID.IsEmpty())
	{
		Client_CharacterDeleted(false, TEXT("Character ID is required."));
		return;
	}

	Server_RequestDeleteCharacter(CharacterID);
}

void AMMOPlayerController::RequestSelectCharacter(const FString& CharacterID)
{
	if (CharacterID.IsEmpty())
	{
		Client_LoginResponse(false, TEXT("Character ID is required."));
		return;
	}

	SetLoginState(EMMOLoginState::Spawning);
	Server_RequestSelectCharacter(CharacterID);
}

void AMMOPlayerController::RequestLogout()
{
	Server_RequestLogout();
}

void AMMOPlayerController::Client_LoginResponse_Implementation(bool bSuccess, const FString& ErrorMessage)
{
	GetWorldTimerManager().ClearTimer(LoginTimeoutHandle);

	if (bSuccess)
	{
		SetLoginState(EMMOLoginState::Authenticated);
		UE_LOG(LogMMOCoreLogin, Log, TEXT("Login successful for account %s"), *AccountID);
	}
	else
	{
		SetLoginState(EMMOLoginState::Unauthenticated);
		UE_LOG(LogMMOCoreLogin, Warning, TEXT("Login failed: %s"), *ErrorMessage);
		Client_LoginFailed(ErrorMessage);
	}
}

bool AMMOPlayerController::Client_LoginResponse_Validate(bool bSuccess, const FString& ErrorMessage)
{
	return true;
}

void AMMOPlayerController::Client_CharacterListResponse_Implementation(const TArray<FString>& CharacterNames, const TArray<FString>& CharacterIDs)
{
	SetLoginState(EMMOLoginState::CharacterSelect);
}

bool AMMOPlayerController::Client_CharacterListResponse_Validate(const TArray<FString>& CharacterNames, const TArray<FString>& CharacterIDs)
{
	return true;
}

void AMMOPlayerController::Client_CharacterCreated_Implementation(bool bSuccess, const FString& CharacterID, const FString& ErrorMessage)
{
	if (bSuccess)
	{
		UE_LOG(LogMMOCoreLogin, Log, TEXT("Character created: %s"), *CharacterID);
	}
	else
	{
		UE_LOG(LogMMOCoreLogin, Warning, TEXT("Character creation failed: %s"), *ErrorMessage);
	}
}

bool AMMOPlayerController::Client_CharacterCreated_Validate(bool bSuccess, const FString& CharacterID, const FString& ErrorMessage)
{
	return true;
}

void AMMOPlayerController::Client_CharacterDeleted_Implementation(bool bSuccess, const FString& ErrorMessage)
{
	if (bSuccess)
	{
		UE_LOG(LogMMOCoreLogin, Log, TEXT("Character deleted successfully"));
	}
	else
	{
		UE_LOG(LogMMOCoreLogin, Warning, TEXT("Character deletion failed: %s"), *ErrorMessage);
	}
}

bool AMMOPlayerController::Client_CharacterDeleted_Validate(bool bSuccess, const FString& ErrorMessage)
{
	return true;
}

void AMMOPlayerController::Client_EnterWorld_Implementation(const FString& SaveGameData)
{
	SetLoginState(EMMOLoginState::Playing);

	AMMOGameMode* GM = GetWorld()->GetAuthGameMode<AMMOGameMode>();
	if (GM)
	{
		GM->RestartPlayer(this);
	}
}

bool AMMOPlayerController::Client_EnterWorld_Validate(const FString& SaveGameData)
{
	return true;
}

void AMMOPlayerController::Client_LoginFailed_Implementation(const FString& ErrorMessage)
{
	UE_LOG(LogMMOCoreLogin, Warning, TEXT("Login failed: %s"), *ErrorMessage);
}

bool AMMOPlayerController::Client_LoginFailed_Validate(const FString& ErrorMessage)
{
	return true;
}

void AMMOPlayerController::Server_RequestLogin_Implementation(const FString& Username, const FString& Password)
{
	AccountID = Username;
	PlayerDisplayName = Username;

	Client_LoginResponse(true, TEXT(""));
}

bool AMMOPlayerController::Server_RequestLogin_Validate(const FString& Username, const FString& Password)
{
	return !Username.IsEmpty() && !Password.IsEmpty();
}

void AMMOPlayerController::Server_RequestCreateAccount_Implementation(const FString& Username, const FString& Password, const FString& Email)
{
	UE_LOG(LogMMOCoreLogin, Log, TEXT("Account creation requested: %s / %s"), *Username, *Email);
}

bool AMMOPlayerController::Server_RequestCreateAccount_Validate(const FString& Username, const FString& Password, const FString& Email)
{
	return !Username.IsEmpty() && !Password.IsEmpty() && !Email.IsEmpty();
}

void AMMOPlayerController::Server_RequestCharacterList_Implementation()
{
	TArray<FString> CharacterNames;
	TArray<FString> CharacterIDs;

	Client_CharacterListResponse(CharacterNames, CharacterIDs);
}

bool AMMOPlayerController::Server_RequestCharacterList_Validate()
{
	return true;
}

void AMMOPlayerController::Server_RequestCreateCharacter_Implementation(const FString& CharacterName, const FString& ClassName)
{
	FString NewCharacterID = FGuid::NewGuid().ToString();
	CharacterID = NewCharacterID;
	PlayerDisplayName = CharacterName;

	Client_CharacterCreated(true, NewCharacterID, TEXT(""));
}

bool AMMOPlayerController::Server_RequestCreateCharacter_Validate(const FString& CharacterName, const FString& ClassName)
{
	return !CharacterName.IsEmpty();
}

void AMMOPlayerController::Server_RequestDeleteCharacter_Implementation(const FString& CharacterID)
{
	Client_CharacterDeleted(true, TEXT(""));
}

bool AMMOPlayerController::Server_RequestDeleteCharacter_Validate(const FString& CharacterID)
{
	return !CharacterID.IsEmpty();
}

void AMMOPlayerController::Server_RequestSelectCharacter_Implementation(const FString& CharacterID)
{
	AMMOPlayerState* MMOps = GetPlayerState<AMMOPlayerState>();
	if (MMOps)
	{
		MMOps->SetCharacterID(CharacterID);
		MMOps->SetPlayerDisplayName(PlayerDisplayName);
	}

	Client_EnterWorld(TEXT(""));
}

bool AMMOPlayerController::Server_RequestSelectCharacter_Validate(const FString& CharacterID)
{
	return !CharacterID.IsEmpty();
}

void AMMOPlayerController::Server_RequestLogout_Implementation()
{
	SetLoginState(EMMOLoginState::Unauthenticated);
	CharacterID.Empty();
	PlayerDisplayName.Empty();
}

bool AMMOPlayerController::Server_RequestLogout_Validate()
{
	return true;
}

void AMMOPlayerController::SendChatMessage(EMMOChatChannel Channel, const FString& Message)
{
	if (Message.IsEmpty())
	{
		return;
	}

	if (Message.Len() > 200)
	{
		Client_SystemMessage(TEXT("Message too long."));
		return;
	}

	Server_SendChatMessage(Channel, Message);
}

void AMMOPlayerController::Server_SendChatMessage_Implementation(EMMOChatChannel Channel, const FString& Message)
{
	FString FormattedMessage = FString::Printf(TEXT("[%s] %s: %s"),
		*UEnum::GetValueAsString(Channel),
		*PlayerDisplayName,
		*Message);

	UE_LOG(LogMMOCoreChat, Log, TEXT("%s"), *FormattedMessage);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMMOPlayerController* OtherPC = Cast<AMMOPlayerController>(It->Get());
		if (OtherPC && OtherPC->IsLocalController())
		{
			OtherPC->Client_ReceiveChatMessage(Channel, PlayerDisplayName, Message);
		}
	}
}

bool AMMOPlayerController::Server_SendChatMessage_Validate(EMMOChatChannel Channel, const FString& Message)
{
	return !Message.IsEmpty() && Message.Len() <= 200;
}

void AMMOPlayerController::Client_ReceiveChatMessage_Implementation(EMMOChatChannel Channel, const FString& SenderName, const FString& Message)
{
	HandleChatMessage(Channel, SenderName, Message);
	OnChatMessageReceived.Broadcast(Channel, Message);
	OnChatMessageReceivedEvent(Channel, SenderName, Message);
}

bool AMMOPlayerController::Client_ReceiveChatMessage_Validate(EMMOChatChannel Channel, const FString& SenderName, const FString& Message)
{
	return true;
}

void AMMOPlayerController::Client_SystemMessage_Implementation(const FString& Message)
{
	OnSystemMessageReceived(Message);
}

bool AMMOPlayerController::Client_SystemMessage_Validate(const FString& Message)
{
	return true;
}

void AMMOPlayerController::Client_PartyInvite_Implementation(const FString& PartyLeaderName, const FString& PartyID)
{
	OnPartyInviteReceived.Broadcast(PartyLeaderName, PartyID);
}

bool AMMOPlayerController::Client_PartyInvite_Validate(const FString& PartyLeaderName, const FString& PartyID)
{
	return true;
}

void AMMOPlayerController::Client_GuildInvite_Implementation(const FString& GuildName, const FString& GuildID)
{
	OnGuildInviteReceived.Broadcast(GuildName, GuildID);
}

bool AMMOPlayerController::Client_GuildInvite_Validate(const FString& GuildName, const FString& GuildID)
{
	return true;
}

void AMMOPlayerController::SetInputModeForState(EMMOInputMode NewInputMode)
{
	CurrentInputMode = NewInputMode;

	switch (NewInputMode)
	{
	case EMMOInputMode::GameOnly:
		ShowUI(false, true);
		break;
	case EMMOInputMode::UIOnly:
		ShowUI(true, true);
		break;
	case EMMOInputMode::GameAndUI:
		ShowUI(true, false);
		break;
	case EMMOInputMode::Chat:
		ShowUI(true, false);
		break;
	}
}

void AMMOPlayerController::ShowUI(bool bShowCursor, bool bIgnoreInput)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);

	SetInputMode(InputMode);
	bShowMouseCursor = bShowCursor;

	if (bIgnoreInput)
	{
		FInputModeUIOnly UIOnly;
		UIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(UIOnly);
	}
	else if (!bShowCursor)
	{
		FInputModeGameOnly GameOnly;
		SetInputMode(GameOnly);
	}
}

void AMMOPlayerController::HideUI()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

EMMOInputMode AMMOPlayerController::GetCurrentInputMode() const
{
	return CurrentInputMode;
}

float AMMOPlayerController::GetServerTime() const
{
	AMMOGameState* MMOgs = GetWorld()->GetGameState<AMMOGameState>();
	if (MMOgs)
	{
		return MMOgs->GetReplicatedServerTime();
	}
	return 0.0f;
}

void AMMOPlayerController::Client_SyncServerTime_Implementation(float ServerTime)
{
	AMMOGameState* MMOgs = GetWorld()->GetGameState<AMMOGameState>();
	if (MMOgs)
	{
		MMOgs->ServerTimestamp = ServerTime;
	}
}

bool AMMOPlayerController::Client_SyncServerTime_Validate(float ServerTime)
{
	return true;
}

void AMMOPlayerController::HandleLoginStateChanged(EMMOLoginState OldState, EMMOLoginState NewState)
{
	UE_LOG(LogMMOCoreLogin, Verbose, TEXT("Login state changed: %s -> %s"),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(NewState));

	switch (NewState)
	{
	case EMMOLoginState::Unauthenticated:
		SetInputModeForState(EMMOInputMode::UIOnly);
		break;
	case EMMOLoginState::LoggingIn:
		SetInputModeForState(EMMOInputMode::UIOnly);
		break;
	case EMMOLoginState::Authenticated:
		SetInputModeForState(EMMOInputMode::UIOnly);
		break;
	case EMMOLoginState::CharacterSelect:
		SetInputModeForState(EMMOInputMode::UIOnly);
		break;
	case EMMOLoginState::Spawning:
		SetInputModeForState(EMMOInputMode::GameAndUI);
		break;
	case EMMOLoginState::Playing:
		SetInputModeForState(EMMOInputMode::GameOnly);
		break;
	}
}

void AMMOPlayerController::HandleChatMessage(EMMOChatChannel Channel, const FString& SenderName, const FString& Message)
{
}

void AMMOPlayerController::OnLoginTimeout()
{
	UE_LOG(LogMMOCoreLogin, Warning, TEXT("Login timed out for player %s"), *AccountID);
	SetLoginState(EMMOLoginState::Unauthenticated);
	Client_LoginFailed(TEXT("Login timed out."));
}

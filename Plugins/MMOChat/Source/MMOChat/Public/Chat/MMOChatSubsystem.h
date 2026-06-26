#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Chat/MMOEmoteSystem.h"
#include "GameFramework/MMOPlayerController.h"
#include "MMOChatSubsystem.generated.h"

struct FMMOChatMessage;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMMOChatMessageReceived, const FMMOChatMessage&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatChannelJoined, EMMOChatChannel, Channel, const FString&, CustomName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatChannelLeft, EMMOChatChannel, Channel, const FString&, CustomName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerMuted, const FString&, PlayerName, bool, bIsMuted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemMessage, const FString&, Message);

USTRUCT(BlueprintType)
struct MMOCHAT_API FMMOChatChannelInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    EMMOChatChannel ChannelType;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    FString CustomName;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    TArray<FString> MemberNames;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    bool bIsCustom;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    FString Password;

    FMMOChatChannelInfo()
        : ChannelType(EMMOChatChannel::Say)
        , bIsCustom(false)
    {
    }
};

USTRUCT(BlueprintType)
struct MMOCHAT_API FMMOChatRateLimitEntry
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FDateTime> Timestamps;

    UPROPERTY()
    int32 MaxMessages;

    UPROPERTY()
    FTimespan Window;

    FMMOChatRateLimitEntry()
        : MaxMessages(5)
        , Window(FTimespan::FromSeconds(3.0))
    {
    }

    bool CheckAndAdd()
    {
        FDateTime Now = FDateTime::UtcNow();
        Timestamps.RemoveAll([&](const FDateTime& T) { return (Now - T) > Window; });
        if (Timestamps.Num() >= MaxMessages)
        {
            return false;
        }
        Timestamps.Add(Now);
        return true;
    }
};

DECLARE_DELEGATE_OneParam(FMMOChatServerSendDelegate, const FMMOChatMessage&);
DECLARE_DELEGATE_OneParam(FMMOChatClientReceiveDelegate, const FMMOChatMessage&);

UCLASS(BlueprintType, DisplayName = "MMO Chat Subsystem")
class MMOCHAT_API UMMOChatSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

    UPROPERTY(BlueprintAssignable, Category = "Chat|Events")
	FOnMMOChatMessageReceived OnChatMessageReceived;

    UPROPERTY(BlueprintAssignable, Category = "Chat|Events")
    FOnChatChannelJoined OnChatChannelJoined;

    UPROPERTY(BlueprintAssignable, Category = "Chat|Events")
    FOnChatChannelLeft OnChatChannelLeft;

    UPROPERTY(BlueprintAssignable, Category = "Chat|Events")
    FOnPlayerMuted OnPlayerMuted;

    UPROPERTY(BlueprintAssignable, Category = "Chat|Events")
    FOnSystemMessage OnSystemMessage;

    FMMOChatServerSendDelegate OnServerSendRequest;
    FMMOChatClientReceiveDelegate OnClientReceiveRequest;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void SendMessage(const FMMOChatMessage& Message);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void ReceiveMessage(const FMMOChatMessage& Message);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void RequestJoinChannel(EMMOChatChannel Channel, const FString& CustomName, const FString& Password);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void RequestLeaveChannel(EMMOChatChannel Channel, const FString& CustomName);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void RequestMutePlayer(const FString& PlayerName, bool bMute);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void RequestMuteChannel(EMMOChatChannel Channel, bool bMute);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void RequestCreateCustomChannel(const FString& ChannelName, const FString& Password);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void HandleSendMessage(const FMMOChatMessage& Message);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void HandleJoinChannel(EMMOChatChannel Channel, const FString& CustomName, const FString& Password);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void HandleLeaveChannel(EMMOChatChannel Channel, const FString& CustomName);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void HandleMutePlayer(const FString& PlayerName, bool bMute);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void HandleMuteChannel(EMMOChatChannel Channel, bool bMute);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void HandleCreateCustomChannel(const FString& ChannelName, const FString& Password);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    TArray<FMMOChatMessage> GetChatHistory(EMMOChatChannel Channel, int32 MaxMessages = 50) const;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    TArray<FMMOChatMessage> GetChatHistoryByName(const FString& ChannelName, int32 MaxMessages = 50) const;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    TArray<FMMOChatChannelInfo> GetJoinedChannels() const;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    bool IsPlayerMuted(const FString& PlayerName) const;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    bool IsChannelMuted(EMMOChatChannel Channel) const;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    bool IsChannelMutedByName(const FString& ChannelName) const;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    UMMOEmoteSystem* GetEmoteSystem() const { return EmoteSystem; }

    UFUNCTION(BlueprintCallable, Category = "Chat")
    int32 GetMaxHistoryPerChannel() const { return MaxHistoryPerChannel; }

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void SetMaxHistoryPerChannel(int32 NewMax) { MaxHistoryPerChannel = NewMax; }

    UFUNCTION(BlueprintCallable, Category = "Chat")
    bool IsRateLimited() const;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    bool IsServer() const;

protected:
    bool ValidateMessage(const FMMOChatMessage& Message, APlayerController* SenderPC);
    bool ContainsProfanity(const FString& MessageText) const;
    FString FilterProfanity(const FString& MessageText) const;
    bool IsSpam(const FString& MessageText) const;
    void LogChatMessage(const FMMOChatMessage& Message);
    void ProcessChatCommand(const FString& Command, APlayerController* SenderPC);
    void BroadcastMessageToChannel(const FMMOChatMessage& Message);
    void RouteMessageToClients(const FMMOChatMessage& Message);

    UPROPERTY(EditDefaultsOnly, Category = "Chat|Config")
    int32 MaxHistoryPerChannel;

    UPROPERTY(EditDefaultsOnly, Category = "Chat|Config")
    TSet<FString> ProfanityFilterList;

    UPROPERTY(EditDefaultsOnly, Category = "Chat|Config")
    TArray<FString> SpamDetectionPatterns;

    UPROPERTY(EditDefaultsOnly, Category = "Chat|Config")
    float ChatRangeSay;

    UPROPERTY(EditDefaultsOnly, Category = "Chat|Config")
    float ChatRangeYell;

	TMap<EMMOChatChannel, TArray<FMMOChatMessage>> ChannelHistory;

	TMap<FString, TArray<FMMOChatMessage>> CustomChannelHistory;

    UPROPERTY()
    TMap<FString, FMMOChatChannelInfo> CustomChannels;

    UPROPERTY()
    TMap<EMMOChatChannel, FMMOChatChannelInfo> ActiveChannels;

    UPROPERTY()
    TMap<FString, FDateTime> MutedPlayers;

    UPROPERTY()
    TSet<EMMOChatChannel> MutedChannels;

    UPROPERTY()
    TSet<FString> MutedCustomChannels;

    UPROPERTY()
    TMap<FString, FMMOChatRateLimitEntry> RateLimitMap;

    UPROPERTY()
    UMMOEmoteSystem* EmoteSystem;

    UPROPERTY()
    TMap<FString, FString> CommandPrefixes;

    UPROPERTY()
    TArray<FString> ProfanityReplacements;
};

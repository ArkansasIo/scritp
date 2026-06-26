#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/MMOPlayerController.h"
#include "MMOChatMessage.generated.h"

UENUM(BlueprintType)
enum class EMMOChatType : uint8
{
    Normal          UMETA(DisplayName = "Normal"),
    Emote           UMETA(DisplayName = "Emote"),
    System          UMETA(DisplayName = "System"),
    Announcement    UMETA(DisplayName = "Announcement"),
    Error           UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct MMOCHAT_API FMMOChatMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    FString SenderId;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    FString SenderName;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    EMMOChatChannel Channel;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    FString MessageText;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    EMMOChatType ChatType;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    FString TargetPlayerName;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    FLinearColor CustomColor;

    UPROPERTY(BlueprintReadWrite, Category = "Chat")
    int32 FontSize;

    FMMOChatMessage()
        : Channel(EMMOChatChannel::Local)
        , Timestamp(FDateTime::UtcNow())
        , ChatType(EMMOChatType::Normal)
        , FontSize(12)
        , CustomColor(FLinearColor::White)
    {
    }

    FString ToChatString() const
    {
        return FString::Printf(TEXT("[%s] %s: %s"),
            *ChannelToString(),
            *SenderName,
            *MessageText);
    }

    static FString ChannelToString(EMMOChatChannel InChannel)
    {
        switch (InChannel)
        {
        case EMMOChatChannel::Global: return TEXT("Global");
        case EMMOChatChannel::Local: return TEXT("Local");
        case EMMOChatChannel::Say: return TEXT("Say");
        case EMMOChatChannel::Yell: return TEXT("Yell");
        case EMMOChatChannel::Whisper: return TEXT("Whisper");
        case EMMOChatChannel::Party: return TEXT("Party");
        case EMMOChatChannel::Guild: return TEXT("Guild");
        case EMMOChatChannel::Trade: return TEXT("Trade");
        case EMMOChatChannel::LocalDefense: return TEXT("LocalDefense");
        case EMMOChatChannel::World: return TEXT("World");
        case EMMOChatChannel::System: return TEXT("System");
        case EMMOChatChannel::Custom: return TEXT("Custom");
        default: return TEXT("Unknown");
        }
    }

    FString ChannelToString() const
    {
        return ChannelToString(Channel);
    }
};

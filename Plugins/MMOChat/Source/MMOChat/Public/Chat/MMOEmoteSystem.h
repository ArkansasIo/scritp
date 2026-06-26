#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "MMOEmoteSystem.generated.h"

class UAnimMontage;
class USoundBase;
DECLARE_DELEGATE_TwoParams(FMMOEmotePlayDelegate, const FName& /*EmoteId*/, const FString& /*SenderName*/);

UENUM(BlueprintType)
enum class EMMOEmoteCategory : uint8
{
    Social  UMETA(DisplayName = "Social"),
    Dance   UMETA(DisplayName = "Dance"),
    Battle  UMETA(DisplayName = "Battle"),
    Funny   UMETA(DisplayName = "Funny"),
    Respect UMETA(DisplayName = "Respect"),
    Custom  UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct MMCHAT_API FMMOEmoteDefinition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Emote")
    FName EmoteId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Emote")
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Emote")
    FString Command;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Emote")
    TSoftObjectPtr<UAnimMontage> AnimationMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Emote")
    TSoftObjectPtr<USoundBase> SoundEffect;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Emote")
    FString TextTemplate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Emote")
    EMMOEmoteCategory Category;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Emote")
    bool bIsCustom;

    FMMOEmoteDefinition()
        : Category(EMMOEmoteCategory::Social)
        , bIsCustom(false)
    {
    }
};

UCLASS(BlueprintType, Within = MMOChatSubsystem)
class MMCHAT_API UMMOEmoteSystem : public UObject
{
    GENERATED_BODY()

public:
    void InitializeDefaultEmotes();

    UFUNCTION(BlueprintCallable, Category = "Emote")
    const FMMOEmoteDefinition* FindEmoteByCommand(const FString& Command) const;

    UFUNCTION(BlueprintCallable, Category = "Emote")
    const FMMOEmoteDefinition* FindEmoteById(const FName& EmoteId) const;

    UFUNCTION(BlueprintCallable, Category = "Emote")
    TArray<FMMOEmoteDefinition> GetEmotesByCategory(EMMOEmoteCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Emote")
    TArray<FMMOEmoteDefinition> GetAllEmotes() const;

    UFUNCTION(BlueprintCallable, Category = "Emote")
    bool RegisterCustomEmote(const FMMOEmoteDefinition& Emote);

    UFUNCTION(BlueprintCallable, Category = "Emote")
    FString FormatEmoteText(const FMMOEmoteDefinition& Emote, const FString& SenderName, const FString& TargetName = FString()) const;

    UFUNCTION(BlueprintCallable, Category = "Emote")
    void PlayEmote(const FName& EmoteId);

    UFUNCTION(BlueprintCallable, Category = "Emote")
    void PlayEmoteAnimation(const FName& EmoteId);

    FMMOEmotePlayDelegate OnServerPlayEmote;
    FMMOEmotePlayDelegate OnMulticastPlayEmote;

    void SetWorldContext(UWorld* InWorld);

protected:
    UPROPERTY()
    TArray<FMMOEmoteDefinition> Emotes;

    UPROPERTY()
    TMap<FString, int32> CommandToIndex;

    UPROPERTY()
    TMap<FName, int32> IdToIndex;

    UPROPERTY()
    UWorld* WorldContext;
};

#include "Chat/MMOEmoteSystem.h"
#include "Chat/MMOChatSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

void UMMOEmoteSystem::InitializeDefaultEmotes()
{
    Emotes.Empty();
    CommandToIndex.Empty();
    IdToIndex.Empty();

    auto AddEmote = [&](const FMMOEmoteDefinition& Def) -> void
    {
        int32 Idx = Emotes.Add(Def);
        CommandToIndex.Add(Def.Command.ToLower(), Idx);
        IdToIndex.Add(Def.EmoteId, Idx);
    };

    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("wave");
        E.DisplayName = TEXT("Wave");
        E.Command = TEXT("/wave");
        E.TextTemplate = TEXT("{Sender} waves at {Target}.");
        E.Category = EMMOEmoteCategory::Social;
        AddEmote(E);
    }
    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("bow");
        E.DisplayName = TEXT("Bow");
        E.Command = TEXT("/bow");
        E.TextTemplate = TEXT("{Sender} bows respectfully.");
        E.Category = EMMOEmoteCategory::Respect;
        AddEmote(E);
    }
    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("cheer");
        E.DisplayName = TEXT("Cheer");
        E.Command = TEXT("/cheer");
        E.TextTemplate = TEXT("{Sender} cheers loudly!");
        E.Category = EMMOEmoteCategory::Social;
        AddEmote(E);
    }
    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("clap");
        E.DisplayName = TEXT("Clap");
        E.Command = TEXT("/clap");
        E.TextTemplate = TEXT("{Sender} claps.");
        E.Category = EMMOEmoteCategory::Respect;
        AddEmote(E);
    }
    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("dance");
        E.DisplayName = TEXT("Dance");
        E.Command = TEXT("/dance");
        E.TextTemplate = TEXT("{Sender} breaks into a dance!");
        E.Category = EMMOEmoteCategory::Dance;
        AddEmote(E);
    }
    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("laugh");
        E.DisplayName = TEXT("Laugh");
        E.Command = TEXT("/laugh");
        E.TextTemplate = TEXT("{Sender} laughs heartily.");
        E.Category = EMMOEmoteCategory::Funny;
        AddEmote(E);
    }
    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("salute");
        E.DisplayName = TEXT("Salute");
        E.Command = TEXT("/salute");
        E.TextTemplate = TEXT("{Sender} salutes.");
        E.Category = EMMOEmoteCategory::Respect;
        AddEmote(E);
    }
    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("taunt");
        E.DisplayName = TEXT("Taunt");
        E.Command = TEXT("/taunt");
        E.TextTemplate = TEXT("{Sender} taunts {Target}!");
        E.Category = EMMOEmoteCategory::Battle;
        AddEmote(E);
    }
    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("roar");
        E.DisplayName = TEXT("Roar");
        E.Command = TEXT("/roar");
        E.TextTemplate = TEXT("{Sender} lets out a mighty roar!");
        E.Category = EMMOEmoteCategory::Battle;
        AddEmote(E);
    }
    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("silly");
        E.DisplayName = TEXT("Silly");
        E.Command = TEXT("/silly");
        E.TextTemplate = TEXT("{Sender} makes a silly face.");
        E.Category = EMMOEmoteCategory::Funny;
        AddEmote(E);
    }
    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("me");
        E.DisplayName = TEXT("Emote");
        E.Command = TEXT("/me");
        E.TextTemplate = TEXT("{Sender} {CustomText}");
        E.Category = EMMOEmoteCategory::Custom;
        E.bIsCustom = true;
        AddEmote(E);
    }
    {
        FMMOEmoteDefinition E;
        E.EmoteId = TEXT("emote");
        E.DisplayName = TEXT("Emote");
        E.Command = TEXT("/emote");
        E.TextTemplate = TEXT("{Sender} {CustomText}");
        E.Category = EMMOEmoteCategory::Custom;
        E.bIsCustom = true;
        AddEmote(E);
    }
}

const FMMOEmoteDefinition* UMMOEmoteSystem::FindEmoteByCommand(const FString& Command) const
{
    FString LowerCmd = Command.ToLower().TrimStartAndEnd();
    const int32* Idx = CommandToIndex.Find(LowerCmd);
    if (Idx && Emotes.IsValidIndex(*Idx))
    {
        return &Emotes[*Idx];
    }
    return nullptr;
}

const FMMOEmoteDefinition* UMMOEmoteSystem::FindEmoteById(const FName& EmoteId) const
{
    const int32* Idx = IdToIndex.Find(EmoteId);
    if (Idx && Emotes.IsValidIndex(*Idx))
    {
        return &Emotes[*Idx];
    }
    return nullptr;
}

TArray<FMMOEmoteDefinition> UMMOEmoteSystem::GetEmotesByCategory(EMMOEmoteCategory Category) const
{
    TArray<FMMOEmoteDefinition> Result;
    for (const FMMOEmoteDefinition& E : Emotes)
    {
        if (E.Category == Category)
        {
            Result.Add(E);
        }
    }
    return Result;
}

TArray<FMMOEmoteDefinition> UMMOEmoteSystem::GetAllEmotes() const
{
    return Emotes;
}

bool UMMOEmoteSystem::RegisterCustomEmote(const FMMOEmoteDefinition& Emote)
{
    if (Emote.EmoteId.IsNone() || Emote.Command.IsEmpty())
    {
        return false;
    }
    if (CommandToIndex.Contains(Emote.Command.ToLower()))
    {
        return false;
    }
    int32 Idx = Emotes.Add(Emote);
    CommandToIndex.Add(Emote.Command.ToLower(), Idx);
    IdToIndex.Add(Emote.EmoteId, Idx);
    return true;
}

FString UMMOEmoteSystem::FormatEmoteText(const FMMOEmoteDefinition& Emote, const FString& SenderName, const FString& TargetName) const
{
    FString Result = Emote.TextTemplate;
    Result.ReplaceInline(TEXT("{Sender}"), *SenderName);

    if (TargetName.IsEmpty())
    {
        Result.ReplaceInline(TEXT(" at {Target}."), TEXT("."));
        Result.ReplaceInline(TEXT("{Target}"), TEXT("everyone"));
    }
    else
    {
        Result.ReplaceInline(TEXT("{Target}"), *TargetName);
    }

    if (Emote.bIsCustom)
    {
        int32 SpaceIdx = Emote.Command.Find(TEXT(" "));
        FString CustomPart;
        if (SpaceIdx != INDEX_NONE)
        {
            CustomPart = Emote.Command.Mid(SpaceIdx + 1).TrimStartAndEnd();
        }
        if (CustomPart.IsEmpty())
        {
            CustomPart = TEXT("makes an expression");
        }
        Result.ReplaceInline(TEXT("{CustomText}"), *CustomPart);
    }

    return Result;
}

void UMMOEmoteSystem::PlayEmote(const FName& EmoteId)
{
    const FMMOEmoteDefinition* Emote = FindEmoteById(EmoteId);
    if (!Emote)
    {
        return;
    }

    UMMOChatSubsystem* ChatSub = GetTypedOuter<UMMOChatSubsystem>();
    if (!ChatSub || !ChatSub->GetWorld())
    {
        return;
    }

    APlayerController* OwningPC = ChatSub->GetWorld()->GetFirstPlayerController();
    FString SenderName = TEXT("Unknown");
    if (OwningPC && OwningPC->PlayerState)
    {
        SenderName = OwningPC->PlayerState->GetPlayerName();
    }

    if (ChatSub->IsServer())
    {
        OnServerPlayEmote.ExecuteIfBound(EmoteId, SenderName);
    }

    OnMulticastPlayEmote.ExecuteIfBound(EmoteId, SenderName);

    PlayEmoteAnimation(EmoteId);

    FString EmoteText = FormatEmoteText(*Emote, SenderName);
    if (ChatSub)
    {
        FMMOChatMessage Msg;
        Msg.SenderName = SenderName;
        Msg.Channel = EMMOChatChannel::Say;
        Msg.MessageText = EmoteText;
        Msg.ChatType = EMMOChatType::Emote;
        Msg.Timestamp = FDateTime::UtcNow();
        Msg.CustomColor = FLinearColor(0.5f, 0.8f, 1.0f);
        ChatSub->ReceiveMessage(Msg);
    }
}

void UMMOEmoteSystem::PlayEmoteAnimation(const FName& EmoteId)
{
    const FMMOEmoteDefinition* Emote = FindEmoteById(EmoteId);
    if (!Emote || !WorldContext)
    {
        return;
    }

    ACharacter* Char = nullptr;
    for (TActorIterator<ACharacter> It(WorldContext); It; ++It)
    {
        Char = *It;
        break;
    }

    if (Char && Char->GetMesh())
    {
        if (!Emote->AnimationMontage.IsNull())
        {
            UAnimMontage* Montage = Emote->AnimationMontage.LoadSynchronous();
            if (Montage)
            {
                Char->GetMesh()->GetAnimInstance()->Montage_Play(Montage);
            }
        }

        if (!Emote->SoundEffect.IsNull())
        {
            USoundBase* Sound = Emote->SoundEffect.LoadSynchronous();
            if (Sound)
            {
                UGameplayStatics::PlaySoundAtLocation(WorldContext, Sound, Char->GetActorLocation());
            }
        }
    }
}

void UMMOEmoteSystem::SetWorldContext(UWorld* InWorld)
{
    WorldContext = InWorld;
}

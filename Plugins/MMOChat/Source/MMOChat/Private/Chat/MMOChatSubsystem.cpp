#include "Chat/MMOChatSubsystem.h"
#include "Chat/MMOChatMessage.h"
#include "Chat/MMOEmoteSystem.h"
#include "MMOChat.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "Misc/CString.h"

void UMMOChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    MaxHistoryPerChannel = 100;
    ChatRangeSay = 4000.0f;
    ChatRangeYell = 10000.0f;

    ProfanityFilterList.Add(TEXT("badword1"));
    ProfanityFilterList.Add(TEXT("badword2"));
    ProfanityFilterList.Add(TEXT("badword3"));

    SpamDetectionPatterns.Add(TEXT("buy cheap"));
    SpamDetectionPatterns.Add(TEXT("visit http"));
    SpamDetectionPatterns.Add(TEXT("free gold"));

    ProfanityReplacements.Add(TEXT("****"));
    ProfanityReplacements.Add(TEXT("$@!#"));
    ProfanityReplacements.Add(TEXT("***"));

    CommandPrefixes.Add(TEXT("/who"), TEXT("Lists players in the current area."));
    CommandPrefixes.Add(TEXT("/invite"), TEXT("Invites a player to your party."));
    CommandPrefixes.Add(TEXT("/friend"), TEXT("Sends a friend request."));
    CommandPrefixes.Add(TEXT("/whisper"), TEXT("Sends a private message. Usage: /whisper <PlayerName> <Message>"));
    CommandPrefixes.Add(TEXT("/w"), TEXT("Alias for /whisper."));
    CommandPrefixes.Add(TEXT("/party"), TEXT("Party chat. Usage: /party <Message>"));
    CommandPrefixes.Add(TEXT("/p"), TEXT("Alias for /party."));
    CommandPrefixes.Add(TEXT("/guild"), TEXT("Guild chat. Usage: /guild <Message>"));
    CommandPrefixes.Add(TEXT("/g"), TEXT("Alias for /guild."));
    CommandPrefixes.Add(TEXT("/yell"), TEXT("Shouts a message to nearby players."));
    CommandPrefixes.Add(TEXT("/y"), TEXT("Alias for /yell."));
    CommandPrefixes.Add(TEXT("/trade"), TEXT("Trade channel. Usage: /trade <Message>"));
    CommandPrefixes.Add(TEXT("/world"), TEXT("World chat. Usage: /world <Message>"));
    CommandPrefixes.Add(TEXT("/reply"), TEXT("Replies to last whisper. Usage: /reply <Message>"));
    CommandPrefixes.Add(TEXT("/r"), TEXT("Alias for /reply."));
    CommandPrefixes.Add(TEXT("/emotelist"), TEXT("Shows available emotes."));
    CommandPrefixes.Add(TEXT("/help"), TEXT("Shows available commands."));

    for (EMMOChatChannel Channel : {EMMOChatChannel::Say, EMMOChatChannel::Yell, EMMOChatChannel::Whisper,
        EMMOChatChannel::Party, EMMOChatChannel::Guild, EMMOChatChannel::Trade,
        EMMOChatChannel::LocalDefense, EMMOChatChannel::World, EMMOChatChannel::System})
    {
        FMMOChatChannelInfo Info;
        Info.ChannelType = Channel;
        Info.bIsCustom = false;
        ActiveChannels.Add(Channel, Info);
    }

    EmoteSystem = NewObject<UMMOEmoteSystem>(this);
    EmoteSystem->SetWorldContext(GetWorld());
    EmoteSystem->InitializeDefaultEmotes();

    UE_LOG(LogMMOChat, Log, TEXT("MMOChatSubsystem initialized."));
}

void UMMOChatSubsystem::Deinitialize()
{
    ChannelHistory.Empty();
    CustomChannelHistory.Empty();
    MutedPlayers.Empty();
    MutedChannels.Empty();
    RateLimitMap.Empty();

    UE_LOG(LogMMOChat, Log, TEXT("MMOChatSubsystem deinitialized."));

    Super::Deinitialize();
}

bool UMMOChatSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool UMMOChatSubsystem::IsServer() const
{
    UWorld* World = GetWorld();
    return World && World->GetNetMode() != NM_Client;
}

void UMMOChatSubsystem::SendMessage(const FMMOChatMessage& Message)
{
    if (Message.MessageText.IsEmpty() || Message.MessageText.Len() > 512)
    {
        return;
    }

    if (IsServer())
    {
        HandleSendMessage(Message);
    }
    else
    {
        OnServerSendRequest.ExecuteIfBound(Message);
    }
}

void UMMOChatSubsystem::ReceiveMessage(const FMMOChatMessage& Message)
{
    FMMOChatMessage InternalMsg = Message;

    if (Message.Channel == EMMOChatChannel::Custom)
    {
        FString ChannelKey = Message.TargetPlayerName.IsEmpty()
            ? TEXT("Custom_") + Message.SenderName
            : Message.TargetPlayerName;

        TArray<FMMOChatMessage>& Hist = CustomChannelHistory.FindOrAdd(ChannelKey);
        Hist.Add(InternalMsg);
        while (Hist.Num() > MaxHistoryPerChannel)
        {
            Hist.RemoveAt(0);
        }
    }
    else
    {
        TArray<FMMOChatMessage>& Hist = ChannelHistory.FindOrAdd(Message.Channel);
        Hist.Add(InternalMsg);
        while (Hist.Num() > MaxHistoryPerChannel)
        {
            Hist.RemoveAt(0);
        }
    }

    OnChatMessageReceived.Broadcast(InternalMsg);
}

void UMMOChatSubsystem::RequestJoinChannel(EMMOChatChannel Channel, const FString& CustomName, const FString& Password)
{
    if (IsServer())
    {
        HandleJoinChannel(Channel, CustomName, Password);
    }
}

void UMMOChatSubsystem::RequestLeaveChannel(EMMOChatChannel Channel, const FString& CustomName)
{
    if (IsServer())
    {
        HandleLeaveChannel(Channel, CustomName);
    }
}

void UMMOChatSubsystem::RequestMutePlayer(const FString& PlayerName, bool bMute)
{
    if (IsServer())
    {
        HandleMutePlayer(PlayerName, bMute);
    }
}

void UMMOChatSubsystem::RequestMuteChannel(EMMOChatChannel Channel, bool bMute)
{
    if (IsServer())
    {
        HandleMuteChannel(Channel, bMute);
    }
}

void UMMOChatSubsystem::RequestCreateCustomChannel(const FString& ChannelName, const FString& Password)
{
    if (IsServer())
    {
        HandleCreateCustomChannel(ChannelName, Password);
    }
}

void UMMOChatSubsystem::HandleSendMessage(const FMMOChatMessage& Message)
{
    APlayerController* SenderPC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!SenderPC)
    {
        return;
    }

    if (!ValidateMessage(Message, SenderPC))
    {
        return;
    }

    FString LowerText = Message.MessageText.ToLower();
    if (LowerText.StartsWith(TEXT("/")))
    {
        ProcessChatCommand(Message.MessageText, SenderPC);
        return;
    }

    FMMOChatMessage ProcessedMsg = Message;
    ProcessedMsg.Timestamp = FDateTime::UtcNow();

    if (SenderPC->PlayerState)
    {
        ProcessedMsg.SenderName = SenderPC->PlayerState->GetPlayerName();
    }

    ProcessedMsg.MessageText = FilterProfanity(ProcessedMsg.MessageText);

    LogChatMessage(ProcessedMsg);

    BroadcastMessageToChannel(ProcessedMsg);

    OnChatMessageReceived.Broadcast(ProcessedMsg);
}

void UMMOChatSubsystem::HandleJoinChannel(EMMOChatChannel Channel, const FString& CustomName, const FString& Password)
{
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC || !PC->PlayerState)
    {
        return;
    }

    FString PlayerName = PC->PlayerState->GetPlayerName();

    if (Channel == EMMOChatChannel::Custom)
    {
        if (CustomName.IsEmpty())
        {
            return;
        }

        FMMOChatChannelInfo* Existing = CustomChannels.Find(CustomName);
        if (Existing)
        {
            if (!Existing->Password.IsEmpty() && Existing->Password != Password)
            {
                FMMOChatMessage ErrMsg;
                ErrMsg.ChatType = EMMOChatType::Error;
                ErrMsg.MessageText = FString::Printf(TEXT("Incorrect password for channel '%s'."), *CustomName);
                ReceiveMessage(ErrMsg);
                return;
            }
            Existing->MemberNames.AddUnique(PlayerName);
        }
        else
        {
            FMMOChatChannelInfo NewChannel;
            NewChannel.ChannelType = EMMOChatChannel::Custom;
            NewChannel.CustomName = CustomName;
            NewChannel.bIsCustom = true;
            NewChannel.Password = Password;
            NewChannel.MemberNames.Add(PlayerName);
            CustomChannels.Add(CustomName, NewChannel);
        }

        OnChatChannelJoined.Broadcast(Channel, CustomName);
    }
    else
    {
        if (ActiveChannels.Contains(Channel))
        {
            ActiveChannels[Channel].MemberNames.AddUnique(PlayerName);
        }
        else
        {
            FMMOChatChannelInfo Info;
            Info.ChannelType = Channel;
            Info.bIsCustom = false;
            Info.MemberNames.Add(PlayerName);
            ActiveChannels.Add(Channel, Info);
        }

        OnChatChannelJoined.Broadcast(Channel, FString());
    }
}

void UMMOChatSubsystem::HandleLeaveChannel(EMMOChatChannel Channel, const FString& CustomName)
{
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC || !PC->PlayerState)
    {
        return;
    }

    FString PlayerName = PC->PlayerState->GetPlayerName();

    if (Channel == EMMOChatChannel::Custom)
    {
        FMMOChatChannelInfo* Info = CustomChannels.Find(CustomName);
        if (Info)
        {
            Info->MemberNames.Remove(PlayerName);
            if (Info->MemberNames.Num() == 0)
            {
                CustomChannels.Remove(CustomName);
            }
        }
    }
    else if (ActiveChannels.Contains(Channel))
    {
        ActiveChannels[Channel].MemberNames.Remove(PlayerName);
    }

    OnChatChannelLeft.Broadcast(Channel, CustomName);
}

void UMMOChatSubsystem::HandleMutePlayer(const FString& PlayerName, bool bMute)
{
    if (PlayerName.IsEmpty())
    {
        return;
    }

    if (bMute)
    {
        MutedPlayers.Add(PlayerName, FDateTime::UtcNow() + FTimespan::FromHours(1));
    }
    else
    {
        MutedPlayers.Remove(PlayerName);
    }

    OnPlayerMuted.Broadcast(PlayerName, bMute);
}

void UMMOChatSubsystem::HandleMuteChannel(EMMOChatChannel Channel, bool bMute)
{
    if (bMute)
    {
        MutedChannels.Add(Channel);
    }
    else
    {
        MutedChannels.Remove(Channel);
    }
}

void UMMOChatSubsystem::HandleCreateCustomChannel(const FString& ChannelName, const FString& Password)
{
    if (ChannelName.IsEmpty() || ChannelName.Len() > 32)
    {
        return;
    }

    if (CustomChannels.Contains(ChannelName))
    {
        FMMOChatMessage ErrMsg;
        ErrMsg.ChatType = EMMOChatType::Error;
        ErrMsg.MessageText = FString::Printf(TEXT("Channel '%s' already exists."), *ChannelName);
        ReceiveMessage(ErrMsg);
        return;
    }

    FMMOChatChannelInfo NewChannel;
    NewChannel.ChannelType = EMMOChatChannel::Custom;
    NewChannel.CustomName = ChannelName;
    NewChannel.bIsCustom = true;
    NewChannel.Password = Password;

    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (PC && PC->PlayerState)
    {
        NewChannel.MemberNames.Add(PC->PlayerState->GetPlayerName());
    }

    CustomChannels.Add(ChannelName, NewChannel);

    {
        FMMOChatMessage SysMsg;
        SysMsg.ChatType = EMMOChatType::System;
        SysMsg.MessageText = FString::Printf(TEXT("Channel '%s' created."), *ChannelName);
        ReceiveMessage(SysMsg);
    }

    OnChatChannelJoined.Broadcast(EMMOChatChannel::Custom, ChannelName);
}

TArray<FMMOChatMessage> UMMOChatSubsystem::GetChatHistory(EMMOChatChannel Channel, int32 MaxMessages) const
{
    const TArray<FMMOChatMessage>* History = ChannelHistory.Find(Channel);
    if (!History)
    {
        return {};
    }

    int32 StartIdx = FMath::Max(0, History->Num() - MaxMessages);
    return TArray<FMMOChatMessage>(History->GetData() + StartIdx, History->Num() - StartIdx);
}

TArray<FMMOChatMessage> UMMOChatSubsystem::GetChatHistoryByName(const FString& ChannelName, int32 MaxMessages) const
{
    const TArray<FMMOChatMessage>* History = CustomChannelHistory.Find(ChannelName);
    if (!History)
    {
        return {};
    }

    int32 StartIdx = FMath::Max(0, History->Num() - MaxMessages);
    return TArray<FMMOChatMessage>(History->GetData() + StartIdx, History->Num() - StartIdx);
}

TArray<FMMOChatChannelInfo> UMMOChatSubsystem::GetJoinedChannels() const
{
    TArray<FMMOChatChannelInfo> Result;
    ActiveChannels.GenerateValueArray(Result);
    for (const auto& Pair : CustomChannels)
    {
        Result.Add(Pair.Value);
    }
    return Result;
}

bool UMMOChatSubsystem::IsPlayerMuted(const FString& PlayerName) const
{
    const FDateTime* Expiry = MutedPlayers.Find(PlayerName);
    if (!Expiry)
    {
        return false;
    }
    if (FDateTime::UtcNow() > *Expiry)
    {
        return false;
    }
    return true;
}

bool UMMOChatSubsystem::IsChannelMuted(EMMOChatChannel Channel) const
{
    return MutedChannels.Contains(Channel);
}

bool UMMOChatSubsystem::IsChannelMutedByName(const FString& ChannelName) const
{
    return MutedCustomChannels.Contains(ChannelName);
}

bool UMMOChatSubsystem::IsRateLimited() const
{
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC || !PC->PlayerState)
    {
        return false;
    }
    FString PlayerName = PC->PlayerState->GetPlayerName();
    const FMMOChatRateLimitEntry* Entry = RateLimitMap.Find(PlayerName);
    if (!Entry)
    {
        return false;
    }
    FDateTime Now = FDateTime::UtcNow();
    int32 RecentCount = 0;
    for (const FDateTime& T : Entry->Timestamps)
    {
        if ((Now - T) <= Entry->Window)
        {
            RecentCount++;
        }
    }
    return RecentCount >= Entry->MaxMessages;
}

bool UMMOChatSubsystem::ValidateMessage(const FMMOChatMessage& Message, APlayerController* SenderPC)
{
    if (!SenderPC || !SenderPC->PlayerState)
    {
        UE_LOG(LogMMOChat, Warning, TEXT("ValidateMessage: No sender PC or PlayerState."));
        return false;
    }

    FString PlayerName = SenderPC->PlayerState->GetPlayerName();

    if (MutedPlayers.Contains(PlayerName))
    {
        const FDateTime* Expiry = MutedPlayers.Find(PlayerName);
        if (Expiry && FDateTime::UtcNow() < *Expiry)
        {
            FMMOChatMessage ErrorMsg;
            ErrorMsg.ChatType = EMMOChatType::Error;
            ErrorMsg.MessageText = TEXT("You are muted.");
            ReceiveMessage(ErrorMsg);
            return false;
        }
        MutedPlayers.Remove(PlayerName);
    }

    if (IsChannelMuted(Message.Channel))
    {
        return false;
    }

    FMMOChatRateLimitEntry& RateEntry = RateLimitMap.FindOrAdd(PlayerName);
    if (!RateEntry.CheckAndAdd())
    {
        FMMOChatMessage ErrorMsg;
        ErrorMsg.ChatType = EMMOChatType::Error;
        ErrorMsg.MessageText = TEXT("You are sending messages too fast. Please wait.");
        ReceiveMessage(ErrorMsg);
        return false;
    }

    if (ContainsProfanity(Message.MessageText))
    {
        FMMOChatMessage WarnMsg;
        WarnMsg.ChatType = EMMOChatType::System;
        WarnMsg.MessageText = TEXT("Your message contained filtered language and has been sanitized.");
        ReceiveMessage(WarnMsg);
    }

    if (IsSpam(Message.MessageText))
    {
        MutedPlayers.Add(PlayerName, FDateTime::UtcNow() + FTimespan::FromMinutes(5));
        FMMOChatMessage SpamMsg;
        SpamMsg.ChatType = EMMOChatType::Error;
        SpamMsg.MessageText = TEXT("Spam detected. You have been muted for 5 minutes.");
        ReceiveMessage(SpamMsg);
        return false;
    }

    return true;
}

bool UMMOChatSubsystem::ContainsProfanity(const FString& MessageText) const
{
    FString LowerMsg = MessageText.ToLower();
    for (const FString& Word : ProfanityFilterList)
    {
        if (LowerMsg.Contains(Word))
        {
            return true;
        }
    }
    return false;
}

FString UMMOChatSubsystem::FilterProfanity(const FString& MessageText) const
{
    FString Filtered = MessageText;
    FString LowerMsg = Filtered.ToLower();

    int32 ReplacementIdx = 0;
    for (const FString& Word : ProfanityFilterList)
    {
        int32 FoundIdx = LowerMsg.Find(Word);
        if (FoundIdx != INDEX_NONE)
        {
            FString Replacement = TEXT("****");
            if (ProfanityReplacements.IsValidIndex(ReplacementIdx))
            {
                Replacement = ProfanityReplacements[ReplacementIdx];
            }
            Filtered = Filtered.Left(FoundIdx) + Replacement + Filtered.Mid(FoundIdx + Word.Len());
            LowerMsg = Filtered.ToLower();
            ReplacementIdx++;
        }
    }

    return Filtered;
}

bool UMMOChatSubsystem::IsSpam(const FString& MessageText) const
{
    FString LowerMsg = MessageText.ToLower();
    for (const FString& Pattern : SpamDetectionPatterns)
    {
        if (LowerMsg.Contains(Pattern))
        {
            return true;
        }
    }

    int32 UpperCount = 0;
    int32 TotalChars = 0;
    for (TCHAR Ch : MessageText)
    {
        if (FChar::IsAlpha(Ch))
        {
            TotalChars++;
            if (FChar::IsUpper(Ch))
            {
                UpperCount++;
            }
        }
    }
    if (TotalChars > 10 && (float)UpperCount / TotalChars > 0.7f)
    {
        return true;
    }

    TCHAR LastChar = 0;
    int32 RepeatCount = 0;
    int32 MaxRepeat = 0;
    for (TCHAR Ch : MessageText)
    {
        if (Ch == LastChar)
        {
            RepeatCount++;
            MaxRepeat = FMath::Max(MaxRepeat, RepeatCount);
        }
        else
        {
            RepeatCount = 0;
        }
        LastChar = Ch;
    }
    if (MaxRepeat >= 10)
    {
        return true;
    }

    return false;
}

void UMMOChatSubsystem::LogChatMessage(const FMMOChatMessage& Message)
{
    UE_LOG(LogMMOChat, Log, TEXT("[%s][%s] %s: %s"),
        *FDateTime::UtcNow().ToString(TEXT("%Y-%m-%d %H:%M:%S")),
        *Message.ChannelToString(),
        *Message.SenderName,
        *Message.MessageText);
}

void UMMOChatSubsystem::ProcessChatCommand(const FString& Command, APlayerController* SenderPC)
{
    if (!SenderPC || !SenderPC->PlayerState)
    {
        return;
    }

    FString FullCommand = Command.TrimStartAndEnd();
    int32 SpaceIdx = FullCommand.Find(TEXT(" "));
    FString CmdName = SpaceIdx != INDEX_NONE ? FullCommand.Left(SpaceIdx).ToLower() : FullCommand.ToLower();
    FString Args = SpaceIdx != INDEX_NONE ? FullCommand.Mid(SpaceIdx + 1).TrimStartAndEnd() : FString();

    FString PlayerName = SenderPC->PlayerState->GetPlayerName();

    if (CmdName == TEXT("/help"))
    {
        FMMOChatMessage HelpMsg;
        HelpMsg.ChatType = EMMOChatType::System;
        HelpMsg.MessageText = TEXT("Available commands: /who, /whisper <name> <msg>, /w, /party, /p, /guild, /g, /yell, /y, /trade, /world, /reply, /r, /invite <name>, /friend <name>, /emotelist, /help");
        HelpMsg.MessageText += TEXT("\nEmotes: /wave, /bow, /cheer, /clap, /dance, /laugh, /salute, /taunt, /roar, /silly, /me, /emote");
        ReceiveMessage(HelpMsg);
        return;
    }

    if (CmdName == TEXT("/who"))
    {
        FMMOChatMessage WhoMsg;
        WhoMsg.ChatType = EMMOChatType::System;
        WhoMsg.MessageText = TEXT("Players online: ");

        UWorld* World = GetWorld();
        if (World && World->GetGameState())
        {
            TArray<FString> PlayerNames;
            for (APlayerState* PS : World->GetGameState()->PlayerArray)
            {
                if (PS)
                {
                    PlayerNames.Add(PS->GetPlayerName());
                }
            }
            WhoMsg.MessageText += FString::Join(PlayerNames, TEXT(", "));
        }
        else
        {
            WhoMsg.MessageText += TEXT("(unavailable)");
        }
        ReceiveMessage(WhoMsg);
        return;
    }

    if (CmdName == TEXT("/whisper") || CmdName == TEXT("/w"))
    {
        if (Args.IsEmpty())
        {
            FMMOChatMessage ErrMsg;
            ErrMsg.ChatType = EMMOChatType::Error;
            ErrMsg.MessageText = TEXT("Usage: /whisper <PlayerName> <Message>");
            ReceiveMessage(ErrMsg);
            return;
        }

        int32 SpaceInArgs = Args.Find(TEXT(" "));
        if (SpaceInArgs == INDEX_NONE)
        {
            FMMOChatMessage ErrMsg;
            ErrMsg.ChatType = EMMOChatType::Error;
            ErrMsg.MessageText = TEXT("Usage: /whisper <PlayerName> <Message>");
            ReceiveMessage(ErrMsg);
            return;
        }

        FString TargetName = Args.Left(SpaceInArgs);
        FString WhisperText = Args.Mid(SpaceInArgs + 1).TrimStartAndEnd();

        FMMOChatMessage WhisperOut;
        WhisperOut.SenderName = PlayerName;
        WhisperOut.Channel = EMMOChatChannel::Whisper;
        WhisperOut.MessageText = WhisperText;
        WhisperOut.TargetPlayerName = TargetName;
        WhisperOut.ChatType = EMMOChatType::Normal;
        WhisperOut.Timestamp = FDateTime::UtcNow();

        FMMOChatMessage WhisperOutgoing = WhisperOut;
        WhisperOutgoing.MessageText = FString::Printf(TEXT("[To %s] %s"), *TargetName, *WhisperText);
        ReceiveMessage(WhisperOutgoing);

        UE_LOG(LogMMOChat, Log, TEXT("[Whisper] %s -> %s: %s"), *PlayerName, *TargetName, *WhisperText);

        RouteMessageToClients(WhisperOut);
        return;
    }

    if (CmdName == TEXT("/party") || CmdName == TEXT("/p"))
    {
        if (Args.IsEmpty())
        {
            FMMOChatMessage ErrMsg;
            ErrMsg.ChatType = EMMOChatType::Error;
            ErrMsg.MessageText = TEXT("Usage: /party <Message>");
            ReceiveMessage(ErrMsg);
            return;
        }

        FMMOChatMessage PartyMsg;
        PartyMsg.SenderName = PlayerName;
        PartyMsg.Channel = EMMOChatChannel::Party;
        PartyMsg.MessageText = Args;
        PartyMsg.ChatType = EMMOChatType::Normal;
        PartyMsg.Timestamp = FDateTime::UtcNow();

        BroadcastMessageToChannel(PartyMsg);
        LogChatMessage(PartyMsg);
        RouteMessageToClients(PartyMsg);
        return;
    }

    if (CmdName == TEXT("/guild") || CmdName == TEXT("/g"))
    {
        if (Args.IsEmpty())
        {
            FMMOChatMessage ErrMsg;
            ErrMsg.ChatType = EMMOChatType::Error;
            ErrMsg.MessageText = TEXT("Usage: /guild <Message>");
            ReceiveMessage(ErrMsg);
            return;
        }

        FMMOChatMessage GuildMsg;
        GuildMsg.SenderName = PlayerName;
        GuildMsg.Channel = EMMOChatChannel::Guild;
        GuildMsg.MessageText = Args;
        GuildMsg.ChatType = EMMOChatType::Normal;
        GuildMsg.Timestamp = FDateTime::UtcNow();

        BroadcastMessageToChannel(GuildMsg);
        LogChatMessage(GuildMsg);
        RouteMessageToClients(GuildMsg);
        return;
    }

    if (CmdName == TEXT("/yell") || CmdName == TEXT("/y"))
    {
        if (Args.IsEmpty())
        {
            FMMOChatMessage ErrMsg;
            ErrMsg.ChatType = EMMOChatType::Error;
            ErrMsg.MessageText = TEXT("Usage: /yell <Message>");
            ReceiveMessage(ErrMsg);
            return;
        }

        FMMOChatMessage YellMsg;
        YellMsg.SenderName = PlayerName;
        YellMsg.Channel = EMMOChatChannel::Yell;
        YellMsg.MessageText = Args;
        YellMsg.ChatType = EMMOChatType::Normal;
        YellMsg.Timestamp = FDateTime::UtcNow();

        BroadcastMessageToChannel(YellMsg);
        LogChatMessage(YellMsg);
        RouteMessageToClients(YellMsg);
        return;
    }

    if (CmdName == TEXT("/trade"))
    {
        if (Args.IsEmpty())
        {
            FMMOChatMessage ErrMsg;
            ErrMsg.ChatType = EMMOChatType::Error;
            ErrMsg.MessageText = TEXT("Usage: /trade <Message>");
            ReceiveMessage(ErrMsg);
            return;
        }

        FMMOChatMessage TradeMsg;
        TradeMsg.SenderName = PlayerName;
        TradeMsg.Channel = EMMOChatChannel::Trade;
        TradeMsg.MessageText = Args;
        TradeMsg.ChatType = EMMOChatType::Normal;
        TradeMsg.Timestamp = FDateTime::UtcNow();

        BroadcastMessageToChannel(TradeMsg);
        LogChatMessage(TradeMsg);
        RouteMessageToClients(TradeMsg);
        return;
    }

    if (CmdName == TEXT("/world"))
    {
        if (Args.IsEmpty())
        {
            FMMOChatMessage ErrMsg;
            ErrMsg.ChatType = EMMOChatType::Error;
            ErrMsg.MessageText = TEXT("Usage: /world <Message>");
            ReceiveMessage(ErrMsg);
            return;
        }

        FMMOChatMessage WorldMsg;
        WorldMsg.SenderName = PlayerName;
        WorldMsg.Channel = EMMOChatChannel::World;
        WorldMsg.MessageText = Args;
        WorldMsg.ChatType = EMMOChatType::Normal;
        WorldMsg.Timestamp = FDateTime::UtcNow();

        BroadcastMessageToChannel(WorldMsg);
        LogChatMessage(WorldMsg);
        RouteMessageToClients(WorldMsg);
        return;
    }

    if (CmdName == TEXT("/reply") || CmdName == TEXT("/r"))
    {
        if (Args.IsEmpty())
        {
            FMMOChatMessage ErrMsg;
            ErrMsg.ChatType = EMMOChatType::Error;
            ErrMsg.MessageText = TEXT("Usage: /reply <Message>");
            ReceiveMessage(ErrMsg);
            return;
        }

        FMMOChatMessage ReplyMsg;
        ReplyMsg.SenderName = PlayerName;
        ReplyMsg.Channel = EMMOChatChannel::Whisper;
        ReplyMsg.MessageText = Args;
        ReplyMsg.TargetPlayerName = TEXT("<last whisperer>");
        ReplyMsg.ChatType = EMMOChatType::Normal;
        ReplyMsg.Timestamp = FDateTime::UtcNow();

        FMMOChatMessage Outgoing = ReplyMsg;
        Outgoing.MessageText = FString::Printf(TEXT("[Reply to <last whisperer>] %s"), *Args);
        ReceiveMessage(Outgoing);

        RouteMessageToClients(ReplyMsg);
        return;
    }

    if (CmdName == TEXT("/invite"))
    {
        if (Args.IsEmpty())
        {
            FMMOChatMessage ErrMsg;
            ErrMsg.ChatType = EMMOChatType::Error;
            ErrMsg.MessageText = TEXT("Usage: /invite <PlayerName>");
            ReceiveMessage(ErrMsg);
            return;
        }

        FMMOChatMessage InviteMsg;
        InviteMsg.ChatType = EMMOChatType::System;
        InviteMsg.MessageText = FString::Printf(TEXT("Invitation sent to %s."), *Args);
        ReceiveMessage(InviteMsg);

        UE_LOG(LogMMOChat, Log, TEXT("[Invite] %s invited %s"), *PlayerName, *Args);
        return;
    }

    if (CmdName == TEXT("/friend"))
    {
        if (Args.IsEmpty())
        {
            FMMOChatMessage ErrMsg;
            ErrMsg.ChatType = EMMOChatType::Error;
            ErrMsg.MessageText = TEXT("Usage: /friend <PlayerName>");
            ReceiveMessage(ErrMsg);
            return;
        }

        FMMOChatMessage FriendMsg;
        FriendMsg.ChatType = EMMOChatType::System;
        FriendMsg.MessageText = FString::Printf(TEXT("Friend request sent to %s."), *Args);
        ReceiveMessage(FriendMsg);

        UE_LOG(LogMMOChat, Log, TEXT("[Friend] %s sent friend request to %s"), *PlayerName, *Args);
        return;
    }

    if (CmdName == TEXT("/emotelist"))
    {
        if (!EmoteSystem)
        {
            return;
        }
        TArray<FMMOEmoteDefinition> AllEmotes = EmoteSystem->GetAllEmotes();
        TArray<FString> EmoteNames;
        for (const FMMOEmoteDefinition& E : AllEmotes)
        {
            if (!E.bIsCustom)
            {
                EmoteNames.Add(E.Command);
            }
        }
        FMMOChatMessage EmoteListMsg;
        EmoteListMsg.ChatType = EMMOChatType::System;
        EmoteListMsg.MessageText = TEXT("Available emotes: ") + FString::Join(EmoteNames, TEXT(", "));
        ReceiveMessage(EmoteListMsg);
        return;
    }

    const FMMOEmoteDefinition* Emote = EmoteSystem ? EmoteSystem->FindEmoteByCommand(CmdName) : nullptr;
    if (Emote)
    {
        FString TargetName;
        if (!Args.IsEmpty())
        {
            TargetName = Args;
        }

        FString EmoteText = EmoteSystem->FormatEmoteText(*Emote, PlayerName, TargetName);
        FMMOChatMessage EmoteMsg;
        EmoteMsg.SenderName = PlayerName;
        EmoteMsg.Channel = EMMOChatChannel::Say;
        EmoteMsg.MessageText = EmoteText;
        EmoteMsg.ChatType = EMMOChatType::Emote;
        EmoteMsg.Timestamp = FDateTime::UtcNow();
        EmoteMsg.CustomColor = FLinearColor(0.5f, 0.8f, 1.0f);

        BroadcastMessageToChannel(EmoteMsg);
        LogChatMessage(EmoteMsg);
        RouteMessageToClients(EmoteMsg);

        if (Emote->EmoteId != NAME_None)
        {
            EmoteSystem->PlayEmote(Emote->EmoteId);
        }
        return;
    }

    FMMOChatMessage UnknownCmd;
    UnknownCmd.ChatType = EMMOChatType::Error;
    UnknownCmd.MessageText = FString::Printf(TEXT("Unknown command: %s. Type /help for a list of commands."), *CmdName);
    ReceiveMessage(UnknownCmd);
}

void UMMOChatSubsystem::BroadcastMessageToChannel(const FMMOChatMessage& Message)
{
    if (!GetWorld())
    {
        return;
    }

    TArray<FMMOChatMessage>& History = ChannelHistory.FindOrAdd(Message.Channel);
    History.Add(Message);
    while (History.Num() > MaxHistoryPerChannel)
    {
        History.RemoveAt(0);
    }

    FString ChannelName = FMMOChatMessage::ChannelToString(Message.Channel);
    FString FormattedMessage = FString::Printf(TEXT("[%s][%s] %s: %s"),
        *FDateTime::UtcNow().ToString(TEXT("%H:%M:%S")),
        *ChannelName,
        *Message.SenderName,
        *Message.MessageText);

    UE_LOG(LogMMOChat, Log, TEXT("Broadcasting: %s"), *FormattedMessage);

    OnChatMessageReceived.Broadcast(Message);
}

void UMMOChatSubsystem::RouteMessageToClients(const FMMOChatMessage& Message)
{
    OnClientReceiveRequest.ExecuteIfBound(Message);
}

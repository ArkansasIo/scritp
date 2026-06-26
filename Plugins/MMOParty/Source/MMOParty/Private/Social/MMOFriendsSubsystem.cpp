#include "Social/MMOFriendsSubsystem.h"
#include "Misc/Guid.h"
#include "Engine/World.h"

void UMMOFriendsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogMMOParty, Log, TEXT("MMOFriendsSubsystem initialized."));
}

void UMMOFriendsSubsystem::Deinitialize()
{
	SocialDataMap.Empty();
	GlobalRequestMap.Empty();
	UE_LOG(LogMMOParty, Log, TEXT("MMOFriendsSubsystem deinitialized."));
}

FString UMMOFriendsSubsystem::GenerateRequestId()
{
	return FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
}

UMMOFriendsSubsystem::FPlayerSocialData& UMMOFriendsSubsystem::GetOrCreateSocialData(const FString& PlayerId)
{
	return SocialDataMap.FindOrAdd(PlayerId);
}

bool UMMOFriendsSubsystem::SendFriendRequest(const FString& FromId, const FString& FromName, const FString& ToId, const FString& ToName, const FString& Message)
{
	if (FromId == ToId) return false;
	if (FromId.IsEmpty() || ToId.IsEmpty()) return false;

	FPlayerSocialData& FromData = GetOrCreateSocialData(FromId);
	FPlayerSocialData& ToData = GetOrCreateSocialData(ToId);

	bool bAlreadyFriends = FromData.Friends.ContainsByPredicate([&](const FFriendEntry& F) { return F.PlayerId == ToId; });
	if (bAlreadyFriends) return false;

	bool bRequestExists = ToData.PendingRequests.ContainsByPredicate([&](const FFriendRequest& R) { return R.FromId == FromId; });
	if (bRequestExists) return false;

	if (ToData.IgnoredPlayers.Contains(FromId)) return false;

	FFriendRequest Request;
	Request.RequestId = GenerateRequestId();
	Request.FromId = FromId;
	Request.FromName = FromName;
	Request.ToId = ToId;
	Request.ToName = ToName;
	Request.SentTime = FDateTime::UtcNow();
	Request.Message = Message;

	ToData.PendingRequests.Add(Request);
	FromData.PendingRequests.Add(Request);
	GlobalRequestMap.Add(Request.RequestId, Request);

	OnFriendRequestReceived.Broadcast(Request);
	UE_LOG(LogMMOParty, Log, TEXT("Friend request sent from %s to %s."), *FromName, *ToName);
	return true;
}

bool UMMOFriendsSubsystem::AcceptFriendRequest(const FString& RequestId, const FString& AcceptingPlayerId)
{
	FFriendRequest* Request = GlobalRequestMap.Find(RequestId);
	if (!Request) return false;

	if (Request->ToId != AcceptingPlayerId) return false;

	FPlayerSocialData& AcceptorData = GetOrCreateSocialData(AcceptingPlayerId);
	FPlayerSocialData& RequesterData = GetOrCreateSocialData(Request->FromId);

	int32 PendingIdx = AcceptorData.PendingRequests.IndexOfByPredicate([&](const FFriendRequest& R) { return R.RequestId == RequestId; });
	if (PendingIdx == INDEX_NONE) return false;

	FFriendEntry FriendForAcceptor;
	FriendForAcceptor.PlayerId = Request->FromId;
	FriendForAcceptor.PlayerName = Request->FromName;
	FriendForAcceptor.bIsOnline = true;
	AcceptorData.Friends.Add(FriendForAcceptor);

	FFriendEntry FriendForRequester;
	FriendForRequester.PlayerId = Request->ToId;
	FriendForRequester.PlayerName = Request->ToName;
	FriendForRequester.bIsOnline = true;
	RequesterData.Friends.Add(FriendForRequester);

	AcceptorData.PendingRequests.RemoveAt(PendingIdx);

	int32 RequesterPendingIdx = RequesterData.PendingRequests.IndexOfByPredicate([&](const FFriendRequest& R) { return R.RequestId == RequestId; });
	if (RequesterPendingIdx != INDEX_NONE)
	{
		RequesterData.PendingRequests.RemoveAt(RequesterPendingIdx);
	}

	GlobalRequestMap.Remove(RequestId);

	BroadcastFriendListUpdate(AcceptingPlayerId);
	BroadcastFriendListUpdate(Request->FromId);

	UE_LOG(LogMMOParty, Log, TEXT("Friend request %s accepted. %s and %s are now friends."), *RequestId, *Request->FromName, *Request->ToName);
	return true;
}

bool UMMOFriendsSubsystem::DeclineFriendRequest(const FString& RequestId, const FString& DecliningPlayerId)
{
	FFriendRequest* Request = GlobalRequestMap.Find(RequestId);
	if (!Request) return false;

	if (Request->ToId != DecliningPlayerId && Request->FromId != DecliningPlayerId) return false;

	FPlayerSocialData& DeclinerData = GetOrCreateSocialData(DecliningPlayerId);

	int32 PendingIdx = DeclinerData.PendingRequests.IndexOfByPredicate([&](const FFriendRequest& R) { return R.RequestId == RequestId; });
	if (PendingIdx == INDEX_NONE) return false;

	FString OtherPlayerId = (DecliningPlayerId == Request->ToId) ? Request->FromId : Request->ToId;
	FPlayerSocialData& OtherData = GetOrCreateSocialData(OtherPlayerId);

	DeclinerData.PendingRequests.RemoveAt(PendingIdx);

	int32 OtherPendingIdx = OtherData.PendingRequests.IndexOfByPredicate([&](const FFriendRequest& R) { return R.RequestId == RequestId; });
	if (OtherPendingIdx != INDEX_NONE)
	{
		OtherData.PendingRequests.RemoveAt(OtherPendingIdx);
	}

	GlobalRequestMap.Remove(RequestId);

	UE_LOG(LogMMOParty, Log, TEXT("Friend request %s declined by %s."), *RequestId, *DecliningPlayerId);
	return true;
}

bool UMMOFriendsSubsystem::CancelFriendRequest(const FString& RequestId, const FString& CancellingPlayerId)
{
	FFriendRequest* Request = GlobalRequestMap.Find(RequestId);
	if (!Request) return false;

	if (Request->FromId != CancellingPlayerId) return false;

	return DeclineFriendRequest(RequestId, CancellingPlayerId);
}

bool UMMOFriendsSubsystem::RemoveFriend(const FString& PlayerId, const FString& FriendId)
{
	FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (!Data) return false;

	int32 Index = Data->Friends.IndexOfByPredicate([&](const FFriendEntry& F) { return F.PlayerId == FriendId; });
	if (Index == INDEX_NONE) return false;

	Data->Friends.RemoveAt(Index);

	FPlayerSocialData* OtherData = SocialDataMap.Find(FriendId);
	if (OtherData)
	{
		int32 OtherIndex = OtherData->Friends.IndexOfByPredicate([&](const FFriendEntry& F) { return F.PlayerId == PlayerId; });
		if (OtherIndex != INDEX_NONE)
		{
			OtherData->Friends.RemoveAt(OtherIndex);
		}
	}

	BroadcastFriendListUpdate(PlayerId);
	BroadcastFriendListUpdate(FriendId);

	UE_LOG(LogMMOParty, Log, TEXT("Friend %s removed from %s's friend list."), *FriendId, *PlayerId);
	return true;
}

TArray<FFriendEntry> UMMOFriendsSubsystem::GetFriendsList(const FString& PlayerId) const
{
	const FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (Data) return Data->Friends;
	return TArray<FFriendEntry>();
}

TArray<FFriendRequest> UMMOFriendsSubsystem::GetPendingFriendRequests(const FString& PlayerId) const
{
	const FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (Data) return Data->PendingRequests;
	return TArray<FFriendRequest>();
}

bool UMMOFriendsSubsystem::SetFriendNote(const FString& PlayerId, const FString& FriendId, const FString& Note)
{
	FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (!Data) return false;

	for (FFriendEntry& Friend : Data->Friends)
	{
		if (Friend.PlayerId == FriendId)
		{
			Friend.Note = Note;
			BroadcastFriendListUpdate(PlayerId);
			return true;
		}
	}
	return false;
}

bool UMMOFriendsSubsystem::SetFriendGroup(const FString& PlayerId, const FString& FriendId, const FString& Group)
{
	FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (!Data) return false;

	for (FFriendEntry& Friend : Data->Friends)
	{
		if (Friend.PlayerId == FriendId)
		{
			Friend.FriendGroup = Group;
			BroadcastFriendListUpdate(PlayerId);
			return true;
		}
	}
	return false;
}

bool UMMOFriendsSubsystem::AddToIgnoreList(const FString& PlayerId, const FString& IgnoredPlayerId)
{
	if (PlayerId == IgnoredPlayerId) return false;

	FPlayerSocialData& Data = GetOrCreateSocialData(PlayerId);

	if (Data.IgnoredPlayers.Contains(IgnoredPlayerId)) return false;

	Data.IgnoredPlayers.Add(IgnoredPlayerId);

	Data.Friends.RemoveAll([&](const FFriendEntry& F) { return F.PlayerId == IgnoredPlayerId; });
	Data.PendingRequests.RemoveAll([&](const FFriendRequest& R) { return R.FromId == IgnoredPlayerId || R.ToId == IgnoredPlayerId; });

	FPlayerSocialData* OtherData = SocialDataMap.Find(IgnoredPlayerId);
	if (OtherData)
	{
		OtherData->Friends.RemoveAll([&](const FFriendEntry& F) { return F.PlayerId == PlayerId; });
	}

	return true;
}

bool UMMOFriendsSubsystem::RemoveFromIgnoreList(const FString& PlayerId, const FString& IgnoredPlayerId)
{
	FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (!Data) return false;

	return Data->IgnoredPlayers.Remove(IgnoredPlayerId) > 0;
}

TArray<FString> UMMOFriendsSubsystem::GetIgnoreList(const FString& PlayerId) const
{
	const FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (Data) return Data->IgnoredPlayers;
	return TArray<FString>();
}

bool UMMOFriendsSubsystem::IsIgnored(const FString& PlayerId, const FString& TargetPlayerId) const
{
	const FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (Data)
	{
		return Data->IgnoredPlayers.Contains(TargetPlayerId);
	}
	return false;
}

TArray<FFriendEntry> UMMOFriendsSubsystem::SearchPlayers(const FString& SearchTerm, int32 MaxResults) const
{
	TArray<FFriendEntry> Results;

	if (SearchTerm.IsEmpty()) return Results;

	for (const auto& Pair : SocialDataMap)
	{
		if (Results.Num() >= MaxResults) break;

		FFriendEntry Entry;
		Entry.PlayerId = Pair.Key;
		Entry.PlayerName = Pair.Key;
		Entry.bIsOnline = true;

		if (Pair.Key.Contains(SearchTerm) || Entry.PlayerName.Contains(SearchTerm))
		{
			Results.Add(Entry);
		}
	}

	return Results;
}

void UMMOFriendsSubsystem::AddToRecentPlayers(const FString& PlayerId, const FString& TargetId, const FString& TargetName, const FString& InteractionType)
{
	if (PlayerId == TargetId) return;

	FPlayerSocialData& Data = GetOrCreateSocialData(PlayerId);

	int32 ExistingIdx = Data.RecentPlayers.IndexOfByPredicate([&](const FRecentPlayer& R) { return R.PlayerId == TargetId; });
	if (ExistingIdx != INDEX_NONE)
	{
		Data.RecentPlayers[ExistingIdx].LastInteraction = FDateTime::UtcNow();
		Data.RecentPlayers[ExistingIdx].InteractionType = InteractionType;
	}
	else
	{
		FRecentPlayer Recent;
		Recent.PlayerId = TargetId;
		Recent.PlayerName = TargetName;
		Recent.LastInteraction = FDateTime::UtcNow();
		Recent.InteractionType = InteractionType;
		Data.RecentPlayers.Add(Recent);
	}

	Data.RecentPlayers.Sort([](const FRecentPlayer& A, const FRecentPlayer& B)
	{
		return A.LastInteraction > B.LastInteraction;
	});

	if (Data.RecentPlayers.Num() > 100)
	{
		Data.RecentPlayers.SetNum(100);
	}

	BroadcastFriendListUpdate(PlayerId);
}

TArray<FRecentPlayer> UMMOFriendsSubsystem::GetRecentPlayers(const FString& PlayerId, int32 MaxResults) const
{
	const FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (!Data) return TArray<FRecentPlayer>();

	TArray<FRecentPlayer> Sorted = Data->RecentPlayers;
	Sorted.Sort([](const FRecentPlayer& A, const FRecentPlayer& B)
	{
		return A.LastInteraction > B.LastInteraction;
	});

	if (Sorted.Num() > MaxResults)
	{
		Sorted.SetNum(MaxResults);
	}
	return Sorted;
}

void UMMOFriendsSubsystem::TrackWhisper(const FString& SenderId, const FString& SenderName, const FString& ReceiverId, const FString& MessageText, bool bIsIncoming)
{
	FWhisperMessage Message;
	Message.SenderId = SenderId;
	Message.SenderName = SenderName;
	Message.MessageText = MessageText;
	Message.Timestamp = FDateTime::UtcNow();
	Message.bIsIncoming = bIsIncoming;

	FString OwnerId = bIsIncoming ? ReceiverId : SenderId;
	FString OtherId = bIsIncoming ? SenderId : ReceiverId;
	FString OtherName = bIsIncoming ? SenderName : (SocialDataMap.Find(SenderId) ? TEXT("Unknown") : TEXT("Unknown"));

	FPlayerSocialData& Data = GetOrCreateSocialData(OwnerId);

	FWhisperConversation* Conversation = Data.WhisperConversations.Find(OtherId);
	if (Conversation)
	{
		Conversation->Messages.Add(Message);
		Conversation->bHasUnread = bIsIncoming;
	}
	else
	{
		FWhisperConversation NewConv;
		NewConv.OtherPlayerId = OtherId;
		NewConv.OtherPlayerName = OtherName;
		NewConv.bHasUnread = bIsIncoming;
		NewConv.Messages.Add(Message);
		Data.WhisperConversations.Add(OtherId, NewConv);
	}
}

FWhisperConversation UMMOFriendsSubsystem::GetWhisperConversation(const FString& PlayerId, const FString& OtherPlayerId) const
{
	const FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (Data)
	{
		const FWhisperConversation* Conv = Data->WhisperConversations.Find(OtherPlayerId);
		if (Conv) return *Conv;
	}
	return FWhisperConversation();
}

TArray<FWhisperConversation> UMMOFriendsSubsystem::GetAllWhisperConversations(const FString& PlayerId) const
{
	const FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (Data)
	{
		TArray<FWhisperConversation> Conversations;
		Data->WhisperConversations.GenerateValueArray(Conversations);

		Conversations.Sort([](const FWhisperConversation& A, const FWhisperConversation& B)
		{
			if (A.Messages.Num() == 0 || B.Messages.Num() == 0) return A.Messages.Num() > B.Messages.Num();
			return A.Messages.Last().Timestamp > B.Messages.Last().Timestamp;
		});

		return Conversations;
	}
	return TArray<FWhisperConversation>();
}

void UMMOFriendsSubsystem::MarkConversationRead(const FString& PlayerId, const FString& OtherPlayerId)
{
	FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (Data)
	{
		FWhisperConversation* Conv = Data->WhisperConversations.Find(OtherPlayerId);
		if (Conv)
		{
			Conv->bHasUnread = false;
		}
	}
}

void UMMOFriendsSubsystem::NotifyFriendOnline(const FString& PlayerId, const FString& FriendId, bool bOnline)
{
	FPlayerSocialData* Data = SocialDataMap.Find(PlayerId);
	if (!Data) return;

	for (FFriendEntry& Friend : Data->Friends)
	{
		if (Friend.PlayerId == FriendId)
		{
			Friend.bIsOnline = bOnline;
			if (bOnline)
			{
				Friend.LastOnline = FDateTime::UtcNow();
			}
			OnFriendOnlineStatus.Broadcast(FriendId, bOnline);
			BroadcastFriendListUpdate(PlayerId);
			return;
		}
	}
}

void UMMOFriendsSubsystem::UpdateFriendStatus(const FString& FriendId, int32 Level, const FString& Zone)
{
	for (auto& Pair : SocialDataMap)
	{
		for (FFriendEntry& Friend : Pair.Value.Friends)
		{
			if (Friend.PlayerId == FriendId)
			{
				Friend.Level = Level;
				Friend.Zone = Zone;
				BroadcastFriendListUpdate(Pair.Key);
				break;
			}
		}
	}
}

void UMMOFriendsSubsystem::BroadcastFriendListUpdate(const FString& PlayerId)
{
	OnFriendListUpdated.Broadcast(GetFriendsList(PlayerId));
}

#include "Party/MMOPartySubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Misc/Guid.h"

void UMMOPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogMMOParty, Log, TEXT("MMOPartySubsystem initialized."));
}

void UMMOPartySubsystem::Deinitialize()
{
	PartyMap.Empty();
	FinderListingMap.Empty();
	PlayerToPartyMap.Empty();
	PendingInviteMap.Empty();
	VoteKickSessions.Empty();
	ReadyCheckSessions.Empty();
	UE_LOG(LogMMOParty, Log, TEXT("MMOPartySubsystem deinitialized."));
}

FString UMMOPartySubsystem::GeneratePartyId()
{
	return FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
}

FString UMMOPartySubsystem::GenerateListingId()
{
	return FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
}

FString UMMOPartySubsystem::CreateParty(const FString& LeaderId, const FString& LeaderName, EPartyType Type)
{
	if (LeaderId.IsEmpty()) return TEXT("");

	FString ExistingPartyId;
	if (IsPlayerInParty(LeaderId, ExistingPartyId))
	{
		UE_LOG(LogMMOParty, Warning, TEXT("Player %s is already in party %s."), *LeaderId, *ExistingPartyId);
		return TEXT("");
	}

	FString PartyId = GeneratePartyId();

	FParty NewParty;
	NewParty.PartyId = PartyId;
	NewParty.LeaderId = LeaderId;
	NewParty.Type = Type;
	NewParty.LootMethod = ELootMethod::RoundRobin;
	NewParty.bExperienceSharing = true;
	NewParty.bQuestSharing = true;
	NewParty.ExperienceShareRange = 1500.0f;

	FPartyMember Leader;
	Leader.PlayerId = LeaderId;
	Leader.PlayerName = LeaderName;
	Leader.Level = 1;
	Leader.bIsOnline = true;
	Leader.Role = EPartyMemberRole::None;
	NewParty.Members.Add(Leader);

	PartyMap.Add(PartyId, NewParty);
	PlayerToPartyMap.Add(LeaderId, PartyId);

	BroadcastPartyUpdate(PartyId);
	UE_LOG(LogMMOParty, Log, TEXT("Party %s created by %s."), *PartyId, *LeaderName);
	return PartyId;
}

bool UMMOPartySubsystem::DisbandParty(const FString& PartyId, const FString& RequesterId)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;

	if (Party->LeaderId != RequesterId) return false;

	for (const FPartyMember& Member : Party->Members)
	{
		PlayerToPartyMap.Remove(Member.PlayerId);
	}

	PartyMap.Remove(PartyId);

	UE_LOG(LogMMOParty, Log, TEXT("Party %s disbanded by %s."), *PartyId, *RequesterId);
	return true;
}

bool UMMOPartySubsystem::LeaveParty(const FString& PartyId, const FString& MemberId)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;

	int32 Index = Party->Members.IndexOfByPredicate([&](const FPartyMember& M) { return M.PlayerId == MemberId; });
	if (Index == INDEX_NONE) return false;

	Party->Members.RemoveAt(Index);
	PlayerToPartyMap.Remove(MemberId);

	if (Party->Members.Num() == 0)
	{
		PartyMap.Remove(PartyId);
		UE_LOG(LogMMOParty, Log, TEXT("Party %s disbanded (last member left)."), *PartyId);
		return true;
	}

	if (Party->LeaderId == MemberId)
	{
		Party->LeaderId = Party->Members[0].PlayerId;
		UE_LOG(LogMMOParty, Log, TEXT("Party %s leader changed to %s."), *PartyId, *Party->LeaderId);
	}

	BroadcastPartyUpdate(PartyId);
	return true;
}

bool UMMOPartySubsystem::KickFromParty(const FString& PartyId, const FString& RequesterId, const FString& TargetId)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;

	if (Party->LeaderId != RequesterId) return false;
	if (RequesterId == TargetId) return false;

	int32 Index = Party->Members.IndexOfByPredicate([&](const FPartyMember& M) { return M.PlayerId == TargetId; });
	if (Index == INDEX_NONE) return false;

	Party->Members.RemoveAt(Index);
	PlayerToPartyMap.Remove(TargetId);

	BroadcastPartyUpdate(PartyId);
	UE_LOG(LogMMOParty, Log, TEXT("Player %s kicked from party %s by %s."), *TargetId, *PartyId, *RequesterId);
	return true;
}

bool UMMOPartySubsystem::PromoteToLeader(const FString& PartyId, const FString& RequesterId, const FString& NewLeaderId)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;

	if (Party->LeaderId != RequesterId) return false;

	bool bNewLeaderInParty = Party->Members.ContainsByPredicate([&](const FPartyMember& M) { return M.PlayerId == NewLeaderId; });
	if (!bNewLeaderInParty) return false;

	Party->LeaderId = NewLeaderId;
	BroadcastPartyUpdate(PartyId);
	UE_LOG(LogMMOParty, Log, TEXT("Party %s leader promoted to %s."), *PartyId, *NewLeaderId);
	return true;
}

bool UMMOPartySubsystem::SetMemberRole(const FString& PartyId, const FString& RequesterId, const FString& TargetId, EPartyMemberRole NewRole)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;

	if (Party->LeaderId != RequesterId) return false;

	for (FPartyMember& Member : Party->Members)
	{
		if (Member.PlayerId == TargetId)
		{
			Member.Role = NewRole;
			BroadcastPartyUpdate(PartyId);
			return true;
		}
	}
	return false;
}

bool UMMOPartySubsystem::SendPartyInvite(const FString& InviterId, const FString& InviterName, const FString& InviteeId)
{
	if (InviterId == InviteeId) return false;

	FString ExistingPartyId;
	if (!IsPlayerInParty(InviterId, ExistingPartyId)) return false;

	FString InviteePartyId;
	if (IsPlayerInParty(InviteeId, InviteePartyId))
	{
		FParty* TargetParty = PartyMap.Find(InviteePartyId);
		if (TargetParty && TargetParty->Type == EPartyType::Party)
		{
			return false;
		}
	}

	FParty* Party = PartyMap.Find(ExistingPartyId);
	if (!Party) return false;

	int32 MaxSize = GetMaxPartySize(Party->Type);
	if (Party->Members.Num() >= MaxSize) return false;

	FPartyInvite Invite;
	Invite.InviterId = InviterId;
	Invite.InviterName = InviterName;
	Invite.InviteeId = InviteeId;
	Invite.PartyId = ExistingPartyId;
	Invite.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	FPendingInvite Pending;
	Pending.Invite = Invite;
	Pending.ExpiryTime = Invite.Timestamp + INVITE_TIMEOUT;

	TArray<FPendingInvite>& Invites = PendingInviteMap.FindOrAdd(InviteeId);
	Invites.Add(Pending);

	OnPartyInviteReceived.Broadcast(Invite);
	return true;
}

bool UMMOPartySubsystem::AcceptPartyInvite(const FString& InviteeId, const FString& PartyId)
{
	TArray<FPendingInvite>* Invites = PendingInviteMap.Find(InviteeId);
	if (!Invites) return false;

	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;

	int32 MaxSize = GetMaxPartySize(Party->Type);
	if (Party->Members.Num() >= MaxSize) return false;

	FString ExistingPartyId;
	if (IsPlayerInParty(InviteeId, ExistingPartyId))
	{
		FParty* ExistingParty = PartyMap.Find(ExistingPartyId);
		if (ExistingParty && ExistingParty->Type == EPartyType::Party)
		{
			return false;
		}
	}

	int32 InviteIdx = Invites->IndexOfByPredicate([&](const FPendingInvite& P) { return P.Invite.PartyId == PartyId; });
	if (InviteIdx == INDEX_NONE) return false;

	FPartyMember NewMember;
	NewMember.PlayerId = InviteeId;
	NewMember.PlayerName = Invites->GetData()[InviteIdx].Invite.InviteeName;
	NewMember.Level = 1;
	NewMember.bIsOnline = true;
	NewMember.Role = EPartyMemberRole::None;
	Party->Members.Add(NewMember);

	PlayerToPartyMap.Add(InviteeId, PartyId);

	Invites->RemoveAt(InviteIdx);
	if (Invites->Num() == 0)
	{
		PendingInviteMap.Remove(InviteeId);
	}

	BroadcastPartyUpdate(PartyId);
	UE_LOG(LogMMOParty, Log, TEXT("Player %s joined party %s."), *InviteeId, *PartyId);
	return true;
}

bool UMMOPartySubsystem::DeclinePartyInvite(const FString& InviteeId, const FString& PartyId)
{
	TArray<FPendingInvite>* Invites = PendingInviteMap.Find(InviteeId);
	if (!Invites) return false;

	int32 InviteIdx = Invites->IndexOfByPredicate([&](const FPendingInvite& P) { return P.Invite.PartyId == PartyId; });
	if (InviteIdx == INDEX_NONE) return false;

	Invites->RemoveAt(InviteIdx);
	if (Invites->Num() == 0)
	{
		PendingInviteMap.Remove(InviteeId);
	}

	UE_LOG(LogMMOParty, Log, TEXT("Player %s declined invite to party %s."), *InviteeId, *PartyId);
	return true;
}

bool UMMOPartySubsystem::CancelPartyInvite(const FString& InviterId, const FString& InviteeId, const FString& PartyId)
{
	TArray<FPendingInvite>* Invites = PendingInviteMap.Find(InviteeId);
	if (!Invites) return false;

	int32 InviteIdx = Invites->IndexOfByPredicate([&](const FPendingInvite& P)
	{
		return P.Invite.PartyId == PartyId && P.Invite.InviterId == InviterId;
	});
	if (InviteIdx == INDEX_NONE) return false;

	Invites->RemoveAt(InviteIdx);
	if (Invites->Num() == 0)
	{
		PendingInviteMap.Remove(InviteeId);
	}
	return true;
}

bool UMMOPartySubsystem::SetLootMethod(const FString& PartyId, const FString& RequesterId, ELootMethod NewMethod)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;
	if (Party->LeaderId != RequesterId) return false;

	Party->LootMethod = NewMethod;
	BroadcastPartyUpdate(PartyId);
	return true;
}

bool UMMOPartySubsystem::SetExperienceSharing(const FString& PartyId, bool bEnabled)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;

	Party->bExperienceSharing = bEnabled;
	BroadcastPartyUpdate(PartyId);
	return true;
}

bool UMMOPartySubsystem::SetQuestSharing(const FString& PartyId, bool bEnabled)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;

	Party->bQuestSharing = bEnabled;
	BroadcastPartyUpdate(PartyId);
	return true;
}

bool UMMOPartySubsystem::SetExperienceShareRange(const FString& PartyId, float Range)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;

	Party->ExperienceShareRange = FMath::Max(100.0f, Range);
	BroadcastPartyUpdate(PartyId);
	return true;
}

FParty UMMOPartySubsystem::GetParty(const FString& PartyId) const
{
	const FParty* Party = PartyMap.Find(PartyId);
	if (Party) return *Party;
	return FParty();
}

bool UMMOPartySubsystem::IsPlayerInParty(const FString& PlayerId, FString& OutPartyId) const
{
	const FString* Found = PlayerToPartyMap.Find(PlayerId);
	if (Found)
	{
		OutPartyId = *Found;
		return true;
	}
	return false;
}

bool UMMOPartySubsystem::IsPartyLeader(const FString& PartyId, const FString& PlayerId) const
{
	const FParty* Party = PartyMap.Find(PartyId);
	return Party && Party->LeaderId == PlayerId;
}

TArray<FParty> UMMOPartySubsystem::GetAllParties() const
{
	TArray<FParty> Result;
	PartyMap.GenerateValueArray(Result);
	return Result;
}

int32 UMMOPartySubsystem::GetPartyMemberCount(const FString& PartyId) const
{
	const FParty* Party = PartyMap.Find(PartyId);
	return Party ? Party->Members.Num() : 0;
}

TArray<FPartyMember> UMMOPartySubsystem::GetPartyMembers(const FString& PartyId) const
{
	const FParty* Party = PartyMap.Find(PartyId);
	if (Party) return Party->Members;
	return TArray<FPartyMember>();
}

int32 UMMOPartySubsystem::GetMaxPartySize(EPartyType Type)
{
	return Type == EPartyType::Raid ? 40 : 5;
}

FString UMMOPartySubsystem::CreatePartyFinderListing(const FPartyFinderListing& Listing)
{
	if (Listing.LeaderId.IsEmpty()) return TEXT("");

	FString ListingId = GenerateListingId();
	FPartyFinderListing NewListing = Listing;
	NewListing.ListingId = ListingId;
	NewListing.CreatedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	FinderListingMap.Add(ListingId, NewListing);
	OnPartyFinderListingAdded.Broadcast(NewListing);
	return ListingId;
}

bool UMMOPartySubsystem::RemovePartyFinderListing(const FString& ListingId, const FString& RequesterId)
{
	FPartyFinderListing* Listing = FinderListingMap.Find(ListingId);
	if (!Listing) return false;
	if (Listing->LeaderId != RequesterId) return false;

	FinderListingMap.Remove(ListingId);
	OnPartyFinderListingRemoved.Broadcast(ListingId);
	return true;
}

TArray<FPartyFinderListing> UMMOPartySubsystem::GetPartyFinderListings() const
{
	TArray<FPartyFinderListing> Result;
	FinderListingMap.GenerateValueArray(Result);
	return Result;
}

bool UMMOPartySubsystem::ApplyToPartyFinderListing(const FString& ListingId, const FString& ApplicantId, const FString& ApplicantName)
{
	FPartyFinderListing* Listing = FinderListingMap.Find(ListingId);
	if (!Listing) return false;

	UE_LOG(LogMMOParty, Log, TEXT("Player %s applied to party finder listing %s."), *ApplicantName, *ListingId);
	return true;
}

bool UMMOPartySubsystem::StartReadyCheck(const FString& PartyId, const FString& RequesterId)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;

	if (Party->LeaderId != RequesterId) return false;

	FReadyCheckSession Session;
	Session.InstigatorId = RequesterId;
	Session.StartTime = FPlatformTime::Seconds();

	for (const FPartyMember& Member : Party->Members)
	{
		Session.Responses.Add(Member.PlayerId, false);
	}

	ReadyCheckSessions.Add(PartyId, Session);
	UE_LOG(LogMMOParty, Log, TEXT("Ready check started for party %s by %s."), *PartyId, *RequesterId);
	return true;
}

bool UMMOPartySubsystem::RespondReadyCheck(const FString& PartyId, const FString& MemberId, bool bReady)
{
	FReadyCheckSession* Session = ReadyCheckSessions.Find(PartyId);
	if (!Session) return false;

	if (Session->Responses.Contains(MemberId))
	{
		Session->Responses[MemberId] = bReady;

		bool bAllResponded = true;
		for (const auto& Pair : Session->Responses)
		{
			if (!Pair.Value)
			{
				bAllResponded = false;
				break;
			}
		}

		if (bAllResponded)
		{
			UE_LOG(LogMMOParty, Log, TEXT("All members ready in party %s."), *PartyId);
			ReadyCheckSessions.Remove(PartyId);
		}
		return true;
	}
	return false;
}

bool UMMOPartySubsystem::InitiateVoteKick(const FString& PartyId, const FString& RequesterId, const FString& TargetId)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;
	if (RequesterId == TargetId) return false;
	if (Party->LeaderId == TargetId) return false;

	bool bRequesterInParty = Party->Members.ContainsByPredicate([&](const FPartyMember& M) { return M.PlayerId == RequesterId; });
	bool bTargetInParty = Party->Members.ContainsByPredicate([&](const FPartyMember& M) { return M.PlayerId == TargetId; });
	if (!bRequesterInParty || !bTargetInParty) return false;

	if (VoteKickSessions.Contains(PartyId)) return false;

	FVoteKickSession Session;
	Session.TargetId = TargetId;
	Session.InstigatorId = RequesterId;
	Session.StartTime = FPlatformTime::Seconds();

	VoteKickSessions.Add(PartyId, Session);
	UE_LOG(LogMMOParty, Log, TEXT("Vote kick initiated for %s in party %s by %s."), *TargetId, *PartyId, *RequesterId);
	return true;
}

bool UMMOPartySubsystem::VoteOnKick(const FString& PartyId, const FString& VoterId, bool bApprove)
{
	FVoteKickSession* Session = VoteKickSessions.Find(PartyId);
	if (!Session) return false;

	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return false;

	if (VoterId == Session->TargetId) return false;
	if (Session->VotedYes.Contains(VoterId) || Session->VotedNo.Contains(VoterId)) return false;

	if (bApprove)
	{
		Session->VotedYes.Add(VoterId);
	}
	else
	{
		Session->VotedNo.Add(VoterId);
	}

	int32 RequiredVotes = FMath::CeilToInt(Party->Members.Num() * 0.5f);
	if (Session->VotedYes.Num() >= RequiredVotes)
	{
		KickFromParty(PartyId, Party->LeaderId, Session->TargetId);
		VoteKickSessions.Remove(PartyId);
		UE_LOG(LogMMOParty, Log, TEXT("Vote kick passed for %s in party %s."), *Session->TargetId, *PartyId);
	}
	else if (Session->VotedNo.Num() >= RequiredVotes)
	{
		VoteKickSessions.Remove(PartyId);
		UE_LOG(LogMMOParty, Log, TEXT("Vote kick failed for %s in party %s."), *Session->TargetId, *PartyId);
	}

	return true;
}

void UMMOPartySubsystem::UpdatePartyMember(const FString& PartyId, const FString& MemberId, float HealthPct, float ManaPct, const FString& Zone, bool bOnline)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (!Party) return;

	for (FPartyMember& Member : Party->Members)
	{
		if (Member.PlayerId == MemberId)
		{
			Member.HealthPercent = HealthPct;
			Member.ManaPercent = ManaPct;
			Member.Zone = Zone;
			Member.bIsOnline = bOnline;
			BroadcastPartyUpdate(PartyId);
			return;
		}
	}
}

FPartyMember UMMOPartySubsystem::GetPartyMember(const FString& PartyId, const FString& MemberId) const
{
	const FParty* Party = PartyMap.Find(PartyId);
	if (Party)
	{
		const FPartyMember* Found = Party->Members.FindByPredicate([&](const FPartyMember& M) { return M.PlayerId == MemberId; });
		if (Found) return *Found;
	}
	return FPartyMember();
}

void UMMOPartySubsystem::BroadcastPartyUpdate(const FString& PartyId)
{
	FParty* Party = PartyMap.Find(PartyId);
	if (Party)
	{
		OnPartyUpdated.Broadcast(PartyId, *Party);
	}
}

void UMMOPartySubsystem::RemovePlayerMapping(const FString& PlayerId)
{
	PlayerToPartyMap.Remove(PlayerId);
}

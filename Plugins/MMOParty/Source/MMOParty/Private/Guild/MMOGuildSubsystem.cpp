#include "Guild/MMOGuildSubsystem.h"
#include "Misc/Guid.h"
#include "Engine/World.h"

void UMMOGuildSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogMMOParty, Log, TEXT("MMOGuildSubsystem initialized."));
}

void UMMOGuildSubsystem::Deinitialize()
{
	GuildMap.Empty();
	CharterMap.Empty();
	PlayerToGuildMap.Empty();
	UE_LOG(LogMMOParty, Log, TEXT("MMOGuildSubsystem deinitialized."));
}

FString UMMOGuildSubsystem::GenerateGuildId()
{
	return FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
}

FString UMMOGuildSubsystem::GenerateCharterId()
{
	return FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
}

FString UMMOGuildSubsystem::CreateGuildCharter(const FString& FounderId, const FString& FounderName, const FString& GuildName, const FString& Tag)
{
	if (GuildName.IsEmpty() || FounderId.IsEmpty()) return TEXT("");

	if (PlayerToGuildMap.Contains(FounderId)) return TEXT("");

	for (const auto& Pair : GuildMap)
	{
		if (Pair.Value.GuildName == GuildName)
		{
			UE_LOG(LogMMOParty, Warning, TEXT("Guild name %s already taken."), *GuildName);
			return TEXT("");
		}
	}

	FString CharterId = GenerateCharterId();
	FGuildCharter Charter;
	Charter.GuildName = GuildName;
	Charter.Tag = Tag.Left(4);
	Charter.FounderId = FounderId;
	Charter.FounderName = FounderName;
	Charter.Signatures.Add(FounderId);

	CharterMap.Add(CharterId, Charter);

	UE_LOG(LogMMOParty, Log, TEXT("Guild charter %s created for %s by %s."), *CharterId, *GuildName, *FounderName);
	return CharterId;
}

bool UMMOGuildSubsystem::SignGuildCharter(const FString& CharterId, const FString& SignerId)
{
	FGuildCharter* Charter = CharterMap.Find(CharterId);
	if (!Charter) return false;

	if (PlayerToGuildMap.Contains(SignerId)) return false;
	if (Charter->Signatures.Contains(SignerId)) return false;

	Charter->Signatures.Add(SignerId);
	UE_LOG(LogMMOParty, Log, TEXT("Player %s signed guild charter %s."), *SignerId, *CharterId);
	return true;
}

bool UMMOGuildSubsystem::FinalizeGuildCreation(const FString& CharterId)
{
	FGuildCharter* Charter = CharterMap.Find(CharterId);
	if (!Charter) return false;

	if (Charter->Signatures.Num() < Charter->RequiredSignatures) return false;

	FString GuildId = GenerateGuildId();
	FGuildData NewGuild;
	NewGuild.GuildId = GuildId;
	NewGuild.GuildName = Charter->GuildName;
	NewGuild.Tag = Charter->Tag;
	NewGuild.LeaderId = Charter->FounderId;
	NewGuild.CreatedDate = FDateTime::UtcNow();
	NewGuild.Level = 1;
	NewGuild.Experience = 0;
	NewGuild.BankSlots = 50;
	NewGuild.bIsRecruiting = false;

	for (int32 i = 0; i < Charter->Signatures.Num(); ++i)
	{
		FGuildMember Member;
		Member.PlayerId = Charter->Signatures[i];
		Member.PlayerName = (i == 0) ? Charter->FounderName : TEXT("Member");
		Member.RankIndex = (i == 0) ? 0 : GetDefaultRankIndex(NewGuild);
		Member.JoinDate = FDateTime::UtcNow();
		Member.bIsOnline = true;
		NewGuild.Members.Add(Member);

		PlayerToGuildMap.Add(Member.PlayerId, GuildId);
	}

	InitDefaultRanks(NewGuild);

	if (NewGuild.Members.Num() > 0)
	{
		NewGuild.Members[0].RankIndex = 0;
	}

	GuildMap.Add(GuildId, NewGuild);
	CharterMap.Remove(CharterId);

	BroadcastGuildUpdate(GuildId);
	UE_LOG(LogMMOParty, Log, TEXT("Guild %s (%s) created."), *NewGuild.GuildName, *GuildId);
	return true;
}

bool UMMOGuildSubsystem::DisbandGuild(const FString& GuildId, const FString& RequesterId)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;
	if (Guild->LeaderId != RequesterId) return false;

	for (const FGuildMember& Member : Guild->Members)
	{
		PlayerToGuildMap.Remove(Member.PlayerId);
	}

	GuildMap.Remove(GuildId);
	UE_LOG(LogMMOParty, Log, TEXT("Guild %s disbanded."), *GuildId);
	return true;
}

bool UMMOGuildSubsystem::AddMember(const FString& GuildId, const FGuildMember& Member)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (Guild->Members.ContainsByPredicate([&](const FGuildMember& M) { return M.PlayerId == Member.PlayerId; }))
	{
		return false;
	}

	if (PlayerToGuildMap.Contains(Member.PlayerId)) return false;

	FGuildMember NewMember = Member;
	NewMember.JoinDate = FDateTime::UtcNow();

	if (NewMember.RankIndex <= 0)
	{
		NewMember.RankIndex = GetDefaultRankIndex(*Guild);
	}

	Guild->Members.Add(NewMember);
	PlayerToGuildMap.Add(NewMember.PlayerId, GuildId);

	BroadcastGuildUpdate(GuildId);
	OnGuildMemberJoined.Broadcast(NewMember);
	return true;
}

bool UMMOGuildSubsystem::RemoveMember(const FString& GuildId, const FString& RequesterId, const FString& TargetId)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (Guild->LeaderId == TargetId) return false;

	if (RequesterId != Guild->LeaderId)
	{
		if (!HasPermission(GuildId, RequesterId, EGuildPermission::Kick))
		{
			return false;
		}
	}

	int32 Index = Guild->Members.IndexOfByPredicate([&](const FGuildMember& M) { return M.PlayerId == TargetId; });
	if (Index == INDEX_NONE) return false;

	PlayerToGuildMap.Remove(TargetId);
	Guild->Members.RemoveAt(Index);

	BroadcastGuildUpdate(GuildId);
	OnGuildMemberLeft.Broadcast(TargetId);
	return true;
}

bool UMMOGuildSubsystem::PromoteMember(const FString& GuildId, const FString& RequesterId, const FString& TargetId)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (RequesterId != Guild->LeaderId)
	{
		if (!HasPermission(GuildId, RequesterId, EGuildPermission::Promote))
		{
			return false;
		}
	}

	for (FGuildMember& Member : Guild->Members)
	{
		if (Member.PlayerId == TargetId)
		{
			int32 NewRank = FMath::Max(0, Member.RankIndex - 1);
			if (NewRank < Member.RankIndex)
			{
				Member.RankIndex = NewRank;
				BroadcastGuildUpdate(GuildId);
				return true;
			}
			return false;
		}
	}
	return false;
}

bool UMMOGuildSubsystem::DemoteMember(const FString& GuildId, const FString& RequesterId, const FString& TargetId)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (RequesterId != Guild->LeaderId)
	{
		if (!HasPermission(GuildId, RequesterId, EGuildPermission::Demote))
		{
			return false;
		}
	}

	for (FGuildMember& Member : Guild->Members)
	{
		if (Member.PlayerId == TargetId)
		{
			int32 NewRank = FMath::Min(Guild->Ranks.Num() - 1, Member.RankIndex + 1);
			if (NewRank > Member.RankIndex)
			{
				Member.RankIndex = NewRank;
				BroadcastGuildUpdate(GuildId);
				return true;
			}
			return false;
		}
	}
	return false;
}

bool UMMOGuildSubsystem::SetMemberNote(const FString& GuildId, const FString& TargetId, const FString& Note)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	for (FGuildMember& Member : Guild->Members)
	{
		if (Member.PlayerId == TargetId)
		{
			Member.Note = Note;
			BroadcastGuildUpdate(GuildId);
			return true;
		}
	}
	return false;
}

bool UMMOGuildSubsystem::SetMemberRank(const FString& GuildId, const FString& RequesterId, const FString& TargetId, int32 NewRankIndex)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (RequesterId != Guild->LeaderId)
	{
		if (!HasPermission(GuildId, RequesterId, EGuildPermission::Promote) &&
			!HasPermission(GuildId, RequesterId, EGuildPermission::Demote))
		{
			return false;
		}
	}

	if (NewRankIndex < 0 || NewRankIndex >= Guild->Ranks.Num()) return false;
	if (TargetId == Guild->LeaderId) return false;

	for (FGuildMember& Member : Guild->Members)
	{
		if (Member.PlayerId == TargetId)
		{
			Member.RankIndex = NewRankIndex;
			BroadcastGuildUpdate(GuildId);
			return true;
		}
	}
	return false;
}

int32 UMMOGuildSubsystem::AddRank(const FString& GuildId, const FString& RequesterId, const FString& RankName, int32 Permissions)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return INDEX_NONE;

	if (RequesterId != Guild->LeaderId) return INDEX_NONE;

	FGuildRank NewRank;
	NewRank.RankIndex = Guild->Ranks.Num();
	NewRank.Name = RankName;
	NewRank.Permissions = Permissions;
	Guild->Ranks.Add(NewRank);

	BroadcastGuildUpdate(GuildId);
	return NewRank.RankIndex;
}

bool UMMOGuildSubsystem::RemoveRank(const FString& GuildId, const FString& RequesterId, int32 RankIndex)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (RequesterId != Guild->LeaderId) return false;
	if (RankIndex <= 0 || RankIndex >= Guild->Ranks.Num()) return false;

	if (Guild->Ranks.Num() <= 2) return false;

	Guild->Ranks.RemoveAt(RankIndex);

	for (int32 i = RankIndex; i < Guild->Ranks.Num(); ++i)
	{
		Guild->Ranks[i].RankIndex = i;
	}

	for (FGuildMember& Member : Guild->Members)
	{
		if (Member.RankIndex > RankIndex)
		{
			Member.RankIndex = FMath::Max(1, Member.RankIndex - 1);
		}
	}

	BroadcastGuildUpdate(GuildId);
	return true;
}

bool UMMOGuildSubsystem::UpdateRank(const FString& GuildId, const FString& RequesterId, int32 RankIndex, const FString& NewName, int32 NewPermissions)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (RequesterId != Guild->LeaderId) return false;
	if (RankIndex < 0 || RankIndex >= Guild->Ranks.Num()) return false;

	Guild->Ranks[RankIndex].Name = NewName;
	Guild->Ranks[RankIndex].Permissions = NewPermissions;

	BroadcastGuildUpdate(GuildId);
	return true;
}

bool UMMOGuildSubsystem::SetMotd(const FString& GuildId, const FString& RequesterId, const FString& NewMotd)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (!HasPermission(GuildId, RequesterId, EGuildPermission::Chat))
	{
		if (Guild->LeaderId != RequesterId) return false;
	}

	Guild->Motd = NewMotd;
	BroadcastGuildUpdate(GuildId);
	return true;
}

FString UMMOGuildSubsystem::GetMotd(const FString& GuildId) const
{
	const FGuildData* Guild = GuildMap.Find(GuildId);
	return Guild ? Guild->Motd : TEXT("");
}

bool UMMOGuildSubsystem::DepositGold(const FString& GuildId, const FString& MemberId, int32 Amount)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (Amount <= 0) return false;

	bool bIsMember = Guild->Members.ContainsByPredicate([&](const FGuildMember& M) { return M.PlayerId == MemberId; });
	if (!bIsMember) return false;

	Guild->Gold += Amount;
	BroadcastGuildUpdate(GuildId);
	UE_LOG(LogMMOParty, Log, TEXT("%d gold deposited to guild %s."), Amount, *GuildId);
	return true;
}

bool UMMOGuildSubsystem::WithdrawGold(const FString& GuildId, const FString& RequesterId, int32 Amount)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (!HasPermission(GuildId, RequesterId, EGuildPermission::BankWithdraw)) return false;
	if (Amount <= 0 || Amount > Guild->Gold) return false;

	Guild->Gold -= Amount;
	BroadcastGuildUpdate(GuildId);
	UE_LOG(LogMMOParty, Log, TEXT("%d gold withdrawn from guild %s by %s."), Amount, *GuildId, *RequesterId);
	return true;
}

bool UMMOGuildSubsystem::DepositItem(const FString& GuildId, const FString& MemberId, const FString& ItemId, const FString& ItemName, int32 Quantity)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (!HasPermission(GuildId, MemberId, EGuildPermission::BankDeposit))
	{
		if (Guild->LeaderId != MemberId) return false;
	}

	if (Quantity <= 0) return false;

	int32 EmptySlot = -1;
	for (int32 i = 0; i < Guild->BankItems.Num(); ++i)
	{
		if (Guild->BankItems[i].ItemId == ItemId)
		{
			Guild->BankItems[i].Quantity += Quantity;
			BroadcastGuildUpdate(GuildId);
			return true;
		}
		if (Guild->BankItems[i].ItemId.IsEmpty() && EmptySlot < 0)
		{
			EmptySlot = i;
		}
	}

	if (EmptySlot < 0)
	{
		if (Guild->BankItems.Num() >= Guild->BankSlots) return false;
		EmptySlot = Guild->BankItems.Num();
	}

	FGuildBankEntry Entry;
	Entry.SlotIndex = EmptySlot;
	Entry.ItemId = ItemId;
	Entry.ItemName = ItemName;
	Entry.Quantity = Quantity;
	Entry.AddedBy = MemberId;
	Entry.AddedDate = FDateTime::UtcNow();

	if (EmptySlot < Guild->BankItems.Num())
	{
		Guild->BankItems[EmptySlot] = Entry;
	}
	else
	{
		Guild->BankItems.Add(Entry);
	}

	BroadcastGuildUpdate(GuildId);
	return true;
}

bool UMMOGuildSubsystem::WithdrawItem(const FString& GuildId, const FString& RequesterId, int32 SlotIndex, int32 Quantity)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (!HasPermission(GuildId, RequesterId, EGuildPermission::BankWithdraw)) return false;

	if (SlotIndex < 0 || SlotIndex >= Guild->BankItems.Num()) return false;
	if (Quantity <= 0 || Quantity > Guild->BankItems[SlotIndex].Quantity) return false;

	Guild->BankItems[SlotIndex].Quantity -= Quantity;
	if (Guild->BankItems[SlotIndex].Quantity <= 0)
	{
		Guild->BankItems.RemoveAt(SlotIndex);
	}

	BroadcastGuildUpdate(GuildId);
	return true;
}

void UMMOGuildSubsystem::AddExperience(const FString& GuildId, int32 Amount)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return;

	if (Amount <= 0) return;

	int32 Remaining = Amount;
	int32 MaxWeeklyXp = FMath::FloorToInt(Guild->WeeklyXpCap);

	if (Guild->WeeklyXp >= MaxWeeklyXp) return;

	int32 WeeklyRemaining = MaxWeeklyXp - Guild->WeeklyXp;
	int32 ActualAmount = FMath::Min(Remaining, WeeklyRemaining);

	Guild->WeeklyXp += ActualAmount;
	Guild->Experience += ActualAmount;

	int32 XpForNextLevel = GetExperienceForLevel(Guild->Level);
	while (Guild->Experience >= XpForNextLevel)
	{
		Guild->Experience -= XpForNextLevel;
		Guild->Level++;
		XpForNextLevel = GetExperienceForLevel(Guild->Level);
		UE_LOG(LogMMOParty, Log, TEXT("Guild %s reached level %d!"), *GuildId, Guild->Level);
	}

	BroadcastGuildUpdate(GuildId);
}

int32 UMMOGuildSubsystem::GetExperienceForLevel(int32 Level)
{
	return 1000 + (Level * 500) + (Level * Level * 50);
}

bool UMMOGuildSubsystem::SetTabard(const FString& GuildId, const FString& RequesterId, const FGuildTabard& NewTabard)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;
	if (Guild->LeaderId != RequesterId) return false;

	Guild->Tabard = NewTabard;
	BroadcastGuildUpdate(GuildId);
	return true;
}

FGuildTabard UMMOGuildSubsystem::GetTabard(const FString& GuildId) const
{
	const FGuildData* Guild = GuildMap.Find(GuildId);
	if (Guild) return Guild->Tabard;
	return FGuildTabard();
}

bool UMMOGuildSubsystem::UnlockAchievement(const FString& GuildId, const FString& AchievementId, const FString& Name, const FString& Description)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (Guild->Achievements.ContainsByPredicate([&](const FGuildAchievement& A) { return A.AchievementId == AchievementId; }))
	{
		return false;
	}

	FGuildAchievement Achievement;
	Achievement.AchievementId = AchievementId;
	Achievement.Name = Name;
	Achievement.Description = Description;
	Achievement.UnlockedDate = FDateTime::UtcNow();
	Achievement.bUnlocked = true;
	Achievement.Progress = 1;
	Achievement.MaxProgress = 1;

	Guild->Achievements.Add(Achievement);
	BroadcastGuildUpdate(GuildId);
	return true;
}

TArray<FGuildAchievement> UMMOGuildSubsystem::GetAchievements(const FString& GuildId) const
{
	const FGuildData* Guild = GuildMap.Find(GuildId);
	if (Guild) return Guild->Achievements;
	return TArray<FGuildAchievement>();
}

bool UMMOGuildSubsystem::UpdateAchievementProgress(const FString& GuildId, const FString& AchievementId, int32 Progress)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	for (FGuildAchievement& Achievement : Guild->Achievements)
	{
		if (Achievement.AchievementId == AchievementId)
		{
			Achievement.Progress = FMath::Min(Progress, Achievement.MaxProgress);
			if (Achievement.Progress >= Achievement.MaxProgress && !Achievement.bUnlocked)
			{
				Achievement.bUnlocked = true;
				Achievement.UnlockedDate = FDateTime::UtcNow();
			}
			BroadcastGuildUpdate(GuildId);
			return true;
		}
	}
	return false;
}

bool UMMOGuildSubsystem::AddPerk(const FString& GuildId, const FGuildPerk& Perk)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (Guild->ActivePerks.ContainsByPredicate([&](const FGuildPerk& P) { return P.PerkId == Perk.PerkId; }))
	{
		return false;
	}

	FGuildPerk NewPerk = Perk;
	NewPerk.bActive = true;
	Guild->ActivePerks.Add(NewPerk);

	BroadcastGuildUpdate(GuildId);
	return true;
}

TArray<FGuildPerk> UMMOGuildSubsystem::GetPerks(const FString& GuildId) const
{
	const FGuildData* Guild = GuildMap.Find(GuildId);
	if (Guild) return Guild->ActivePerks;
	return TArray<FGuildPerk>();
}

bool UMMOGuildSubsystem::SetRecruitment(const FString& GuildId, const FString& RequesterId, bool bRecruiting, const FString& Message)
{
	FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;
	if (Guild->LeaderId != RequesterId) return false;

	Guild->bIsRecruiting = bRecruiting;
	Guild->RecruitmentMessage = Message;

	BroadcastGuildUpdate(GuildId);
	return true;
}

TArray<FGuildData> UMMOGuildSubsystem::GetRecruitingGuilds() const
{
	TArray<FGuildData> Result;
	for (const auto& Pair : GuildMap)
	{
		if (Pair.Value.bIsRecruiting)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FGuildData UMMOGuildSubsystem::GetGuild(const FString& GuildId) const
{
	const FGuildData* Guild = GuildMap.Find(GuildId);
	if (Guild) return *Guild;
	return FGuildData();
}

bool UMMOGuildSubsystem::IsPlayerInGuild(const FString& PlayerId, FString& OutGuildId) const
{
	const FString* Found = PlayerToGuildMap.Find(PlayerId);
	if (Found)
	{
		OutGuildId = *Found;
		return true;
	}
	return false;
}

FGuildMember UMMOGuildSubsystem::GetGuildMember(const FString& GuildId, const FString& PlayerId) const
{
	const FGuildData* Guild = GuildMap.Find(GuildId);
	if (Guild)
	{
		const FGuildMember* Found = Guild->Members.FindByPredicate([&](const FGuildMember& M) { return M.PlayerId == PlayerId; });
		if (Found) return *Found;
	}
	return FGuildMember();
}

TArray<FGuildData> UMMOGuildSubsystem::GetAllGuilds() const
{
	TArray<FGuildData> Result;
	GuildMap.GenerateValueArray(Result);
	return Result;
}

bool UMMOGuildSubsystem::HasPermission(const FString& GuildId, const FString& PlayerId, EGuildPermission Permission) const
{
	const FGuildData* Guild = GuildMap.Find(GuildId);
	if (!Guild) return false;

	if (Guild->LeaderId == PlayerId) return true;

	const FGuildMember* Member = Guild->Members.FindByPredicate([&](const FGuildMember& M) { return M.PlayerId == PlayerId; });
	if (!Member) return false;

	const FGuildRank* Rank = Guild->Ranks.FindByPredicate([&](const FGuildRank& R) { return R.RankIndex == Member->RankIndex; });
	if (!Rank) return false;

	return (Rank->Permissions & static_cast<int32>(Permission)) != 0;
}

void UMMOGuildSubsystem::BroadcastGuildUpdate(const FString& GuildId)
{
	const FGuildData* Guild = GuildMap.Find(GuildId);
	if (Guild)
	{
		OnGuildUpdated.Broadcast(GuildId, *Guild);
	}
}

void UMMOGuildSubsystem::InitDefaultRanks(FGuildData& GuildData)
{
	GuildData.Ranks.Empty();

	FGuildRank LeaderRank;
	LeaderRank.RankIndex = 0;
	LeaderRank.Name = TEXT("Guild Master");
	LeaderRank.Permissions = static_cast<int32>(EGuildPermission::All);
	LeaderRank.BankWithdrawLimit = -1;
	GuildData.Ranks.Add(LeaderRank);

	FGuildRank OfficerRank;
	OfficerRank.RankIndex = 1;
	OfficerRank.Name = TEXT("Officer");
	OfficerRank.Permissions = static_cast<int32>(
		EGuildPermission::Invite | EGuildPermission::Kick | EGuildPermission::Promote |
		EGuildPermission::Demote | EGuildPermission::Chat | EGuildPermission::BankDeposit |
		EGuildPermission::BankWithdraw);
	OfficerRank.BankWithdrawLimit = 1000;
	GuildData.Ranks.Add(OfficerRank);

	FGuildRank MemberRank;
	MemberRank.RankIndex = 2;
	MemberRank.Name = TEXT("Member");
	MemberRank.Permissions = static_cast<int32>(EGuildPermission::Chat | EGuildPermission::BankDeposit);
	MemberRank.BankWithdrawLimit = 0;
	GuildData.Ranks.Add(MemberRank);

	FGuildRank InitiateRank;
	InitiateRank.RankIndex = 3;
	InitiateRank.Name = TEXT("Initiate");
	InitiateRank.Permissions = static_cast<int32>(EGuildPermission::Chat);
	InitiateRank.BankWithdrawLimit = 0;
	GuildData.Ranks.Add(InitiateRank);
}

int32 UMMOGuildSubsystem::GetDefaultRankIndex(const FGuildData& GuildData) const
{
	if (GuildData.Ranks.Num() > 0)
	{
		return GuildData.Ranks.Last().RankIndex;
	}
	return 2;
}

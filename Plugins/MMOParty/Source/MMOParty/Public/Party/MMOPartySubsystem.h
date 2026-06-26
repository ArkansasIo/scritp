#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MMOPartySubsystem.generated.h"

UENUM(BlueprintType)
enum class EPartyMemberRole : uint8
{
	None    UMETA(DisplayName = "None"),
	Tank    UMETA(DisplayName = "Tank"),
	Healer  UMETA(DisplayName = "Healer"),
	DPS     UMETA(DisplayName = "DPS"),
	Support UMETA(DisplayName = "Support")
};

UENUM(BlueprintType)
enum class ELootMethod : uint8
{
	FreeForAll       UMETA(DisplayName = "Free For All"),
	RoundRobin       UMETA(DisplayName = "Round Robin"),
	MasterLooter     UMETA(DisplayName = "Master Looter"),
	NeedBeforeGreed  UMETA(DisplayName = "Need Before Greed"),
	Personal         UMETA(DisplayName = "Personal")
};

UENUM(BlueprintType)
enum class EPartyType : uint8
{
	Party UMETA(DisplayName = "Party"),
	Raid  UMETA(DisplayName = "Raid")
};

USTRUCT(BlueprintType)
struct FPartyMember
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString PlayerId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	int32 Level = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString ClassName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString Zone;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	float HealthPercent = 100.0f;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	float ManaPercent = 100.0f;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	bool bIsOnline = true;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	EPartyMemberRole Role = EPartyMemberRole::None;
};

USTRUCT(BlueprintType)
struct FParty
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString PartyId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString LeaderId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	EPartyType Type = EPartyType::Party;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	ELootMethod LootMethod = ELootMethod::RoundRobin;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	TArray<FPartyMember> Members;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	bool bExperienceSharing = true;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	bool bQuestSharing = true;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	float ExperienceShareRange = 1500.0f;
};

USTRUCT(BlueprintType)
struct FPartyInvite
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString InviterId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString InviterName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString InviteeId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString InviteeName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString PartyId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	float Timestamp = 0.0f;
};

USTRUCT(BlueprintType)
struct FPartyFinderListing
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString ListingId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString LeaderId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString LeaderName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	TArray<EPartyMemberRole> NeededRoles;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	int32 MinLevel = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	int32 MaxLevel = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	FString DungeonTarget;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	bool bIsRaid = false;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Party")
	float CreatedTime = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartyUpdatedDelegate, const FString&, PartyId, const FParty&, PartyData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyInviteReceivedDelegate, const FPartyInvite&, Invite);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyFinderListingAddedDelegate, const FPartyFinderListing&, Listing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyFinderListingRemovedDelegate, const FString&, ListingId);

UCLASS(BlueprintType)
class MMPARTY_API UMMOPartySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	FString CreateParty(const FString& LeaderId, const FString& LeaderName, EPartyType Type = EPartyType::Party);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool DisbandParty(const FString& PartyId, const FString& RequesterId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool LeaveParty(const FString& PartyId, const FString& MemberId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool KickFromParty(const FString& PartyId, const FString& RequesterId, const FString& TargetId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool PromoteToLeader(const FString& PartyId, const FString& RequesterId, const FString& NewLeaderId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool SetMemberRole(const FString& PartyId, const FString& RequesterId, const FString& TargetId, EPartyMemberRole NewRole);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool SendPartyInvite(const FString& InviterId, const FString& InviterName, const FString& InviteeId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool AcceptPartyInvite(const FString& InviteeId, const FString& PartyId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool DeclinePartyInvite(const FString& InviteeId, const FString& PartyId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool CancelPartyInvite(const FString& InviterId, const FString& InviteeId, const FString& PartyId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool SetLootMethod(const FString& PartyId, const FString& RequesterId, ELootMethod NewMethod);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool SetExperienceSharing(const FString& PartyId, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool SetQuestSharing(const FString& PartyId, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool SetExperienceShareRange(const FString& PartyId, float Range);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	FParty GetParty(const FString& PartyId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool IsPlayerInParty(const FString& PlayerId, FString& OutPartyId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool IsPartyLeader(const FString& PartyId, const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	TArray<FParty> GetAllParties() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	int32 GetPartyMemberCount(const FString& PartyId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	TArray<FPartyMember> GetPartyMembers(const FString& PartyId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	static int32 GetMaxPartySize(EPartyType Type);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party|Finder")
	FString CreatePartyFinderListing(const FPartyFinderListing& Listing);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party|Finder")
	bool RemovePartyFinderListing(const FString& ListingId, const FString& RequesterId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party|Finder")
	TArray<FPartyFinderListing> GetPartyFinderListings() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party|Finder")
	bool ApplyToPartyFinderListing(const FString& ListingId, const FString& ApplicantId, const FString& ApplicantName);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool StartReadyCheck(const FString& PartyId, const FString& RequesterId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool RespondReadyCheck(const FString& PartyId, const FString& MemberId, bool bReady);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool InitiateVoteKick(const FString& PartyId, const FString& RequesterId, const FString& TargetId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool VoteOnKick(const FString& PartyId, const FString& VoterId, bool bApprove);

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	void UpdatePartyMember(const FString& PartyId, const FString& MemberId, float HealthPct, float ManaPct, const FString& Zone, bool bOnline);

	UFUNCTION(BlueprintPure, Category = "MMO|Party")
	FPartyMember GetPartyMember(const FString& PartyId, const FString& MemberId) const;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnPartyUpdatedDelegate OnPartyUpdated;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnPartyInviteReceivedDelegate OnPartyInviteReceived;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnPartyFinderListingAddedDelegate OnPartyFinderListingAdded;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnPartyFinderListingRemovedDelegate OnPartyFinderListingRemoved;

protected:
	UPROPERTY()
	TMap<FString, FParty> PartyMap;

	UPROPERTY()
	TMap<FString, FPartyFinderListing> FinderListingMap;

	TMap<FString, FString> PlayerToPartyMap;

	struct FPendingInvite
	{
		FPartyInvite Invite;
		float ExpiryTime;
	};
	TMap<FString, TArray<FPendingInvite>> PendingInviteMap;

	struct FVoteKickSession
	{
		FString TargetId;
		FString InstigatorId;
		TSet<FString> VotedYes;
		TSet<FString> VotedNo;
		double StartTime;
	};
	TMap<FString, FVoteKickSession> VoteKickSessions;

	struct FReadyCheckSession
	{
		FString InstigatorId;
		TMap<FString, bool> Responses;
		double StartTime;
	};
	TMap<FString, FReadyCheckSession> ReadyCheckSessions;

	FString GeneratePartyId();
	FString GenerateListingId();
	void BroadcastPartyUpdate(const FString& PartyId);
	void RemovePlayerMapping(const FString& PlayerId);

	static constexpr float INVITE_TIMEOUT = 60.0f;
	static constexpr float READY_CHECK_TIMEOUT = 30.0f;
	static constexpr float VOTE_KICK_TIMEOUT = 60.0f;
};

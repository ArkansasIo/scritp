#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MMOGuildSubsystem.generated.h"

UENUM(BlueprintType, meta = (Bitflags))
enum class EGuildPermission : uint8
{
	None         UMETA(Hidden),
	Invite       = 1 << 0,
	Kick         = 1 << 1,
	Promote      = 1 << 2,
	Demote       = 1 << 3,
	Chat         = 1 << 4,
	BankDeposit  = 1 << 5,
	BankWithdraw = 1 << 6,
	WarDeclare   = 1 << 7,
	All          = 255
};
ENUM_CLASS_FLAGS(EGuildPermission);

USTRUCT(BlueprintType)
struct FGuildMember
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString PlayerId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 Level = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString ClassName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 RankIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	bool bIsOnline = true;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString Note;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FDateTime JoinDate;
};

USTRUCT(BlueprintType)
struct FGuildRank
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 RankIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 Permissions = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 MemberLimit = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 BankWithdrawLimit = 0;
};

USTRUCT(BlueprintType)
struct FGuildTabard
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 EmblemId = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 BackgroundColor = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 EmblemColor = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 BorderColor = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 BorderStyle = 0;
};

USTRUCT(BlueprintType)
struct FGuildBankEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString ItemId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString ItemName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString AddedBy;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FDateTime AddedDate;
};

USTRUCT(BlueprintType)
struct FGuildAchievement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString AchievementId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FDateTime UnlockedDate;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	bool bUnlocked = false;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 Progress = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 MaxProgress = 0;
};

USTRUCT(BlueprintType)
struct FGuildPerk
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString PerkId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 Level = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 MaxLevel = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	bool bActive = false;
};

USTRUCT(BlueprintType)
struct FGuildData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString GuildId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString GuildName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString Tag;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString LeaderId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	TArray<FGuildMember> Members;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	TArray<FGuildRank> Ranks;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 Level = 1;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 Experience = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString Motd;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FGuildTabard Tabard;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	TArray<FGuildAchievement> Achievements;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	TArray<FGuildPerk> ActivePerks;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 Gold = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	TArray<FGuildBankEntry> BankItems;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 BankSlots = 50;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FDateTime CreatedDate;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString RecruitmentMessage;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	bool bIsRecruiting = false;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	float WeeklyXp = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	float WeeklyXpCap = 5000.0f;
};

USTRUCT(BlueprintType)
struct FGuildCharter
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString GuildName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString Tag;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString FounderId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	FString FounderName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	TArray<FString> Signatures;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 RequiredSignatures = 5;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	int32 RequiredGold = 1000;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Guild")
	float TimeRemaining = 86400.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuildUpdatedDelegate, const FString&, GuildId, const FGuildData&, GuildData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuildMemberJoinedDelegate, const FGuildMember&, Member);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuildMemberLeftDelegate, const FString&, PlayerId);

UCLASS(BlueprintType)
class MMOPARTY_API UMMOGuildSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	FString CreateGuildCharter(const FString& FounderId, const FString& FounderName, const FString& GuildName, const FString& Tag);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool SignGuildCharter(const FString& CharterId, const FString& SignerId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool FinalizeGuildCreation(const FString& CharterId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool DisbandGuild(const FString& GuildId, const FString& RequesterId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool AddMember(const FString& GuildId, const FGuildMember& Member);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool RemoveMember(const FString& GuildId, const FString& RequesterId, const FString& TargetId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool PromoteMember(const FString& GuildId, const FString& RequesterId, const FString& TargetId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool DemoteMember(const FString& GuildId, const FString& RequesterId, const FString& TargetId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool SetMemberNote(const FString& GuildId, const FString& TargetId, const FString& Note);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool SetMemberRank(const FString& GuildId, const FString& RequesterId, const FString& TargetId, int32 NewRankIndex);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	int32 AddRank(const FString& GuildId, const FString& RequesterId, const FString& RankName, int32 Permissions);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool RemoveRank(const FString& GuildId, const FString& RequesterId, int32 RankIndex);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool UpdateRank(const FString& GuildId, const FString& RequesterId, int32 RankIndex, const FString& NewName, int32 NewPermissions);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool SetMotd(const FString& GuildId, const FString& RequesterId, const FString& NewMotd);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	FString GetMotd(const FString& GuildId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool DepositGold(const FString& GuildId, const FString& MemberId, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool WithdrawGold(const FString& GuildId, const FString& RequesterId, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool DepositItem(const FString& GuildId, const FString& MemberId, const FString& ItemId, const FString& ItemName, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool WithdrawItem(const FString& GuildId, const FString& RequesterId, int32 SlotIndex, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	void AddExperience(const FString& GuildId, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	static int32 GetExperienceForLevel(int32 Level);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool SetTabard(const FString& GuildId, const FString& RequesterId, const FGuildTabard& NewTabard);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	FGuildTabard GetTabard(const FString& GuildId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool UnlockAchievement(const FString& GuildId, const FString& AchievementId, const FString& Name, const FString& Description);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	TArray<FGuildAchievement> GetAchievements(const FString& GuildId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool UpdateAchievementProgress(const FString& GuildId, const FString& AchievementId, int32 Progress);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool AddPerk(const FString& GuildId, const FGuildPerk& Perk);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	TArray<FGuildPerk> GetPerks(const FString& GuildId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool SetRecruitment(const FString& GuildId, const FString& RequesterId, bool bRecruiting, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	TArray<FGuildData> GetRecruitingGuilds() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	FGuildData GetGuild(const FString& GuildId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool IsPlayerInGuild(const FString& PlayerId, FString& OutGuildId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	FGuildMember GetGuildMember(const FString& GuildId, const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	TArray<FGuildData> GetAllGuilds() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Guild")
	bool HasPermission(const FString& GuildId, const FString& PlayerId, EGuildPermission Permission) const;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Guild")
	FOnGuildUpdatedDelegate OnGuildUpdated;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Guild")
	FOnGuildMemberJoinedDelegate OnGuildMemberJoined;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Guild")
	FOnGuildMemberLeftDelegate OnGuildMemberLeft;

private:
	UPROPERTY()
	TMap<FString, FGuildData> GuildMap;

	UPROPERTY()
	TMap<FString, FGuildCharter> CharterMap;

	TMap<FString, FString> PlayerToGuildMap;

	FString GenerateGuildId();
	FString GenerateCharterId();
	void BroadcastGuildUpdate(const FString& GuildId);
	void InitDefaultRanks(FGuildData& GuildData);
	int32 GetDefaultRankIndex(const FGuildData& GuildData) const;
};

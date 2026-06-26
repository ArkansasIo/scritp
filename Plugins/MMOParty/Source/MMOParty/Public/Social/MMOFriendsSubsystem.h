#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MMOFriendsSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FFriendEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString PlayerId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	int32 Level = 0;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString ClassName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString Zone;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	bool bIsOnline = false;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FDateTime LastOnline;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString Note;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString FriendGroup;
};

USTRUCT(BlueprintType)
struct FFriendRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString RequestId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString FromId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString FromName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString ToId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString ToName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FDateTime SentTime;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString Message;
};

USTRUCT(BlueprintType)
struct FRecentPlayer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString PlayerId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FDateTime LastInteraction;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString InteractionType;
};

USTRUCT(BlueprintType)
struct FWhisperMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString SenderId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString SenderName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString MessageText;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	bool bIsIncoming = true;
};

USTRUCT(BlueprintType)
struct FWhisperConversation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString OtherPlayerId;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	FString OtherPlayerName;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	TArray<FWhisperMessage> Messages;

	UPROPERTY(BlueprintReadWrite, Category = "MMO|Social")
	bool bHasUnread = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendListUpdatedDelegate, const TArray<FFriendEntry>&, Friends);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendRequestReceivedDelegate, const FFriendRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFriendOnlineStatusDelegate, const FString&, PlayerId, bool, bOnline);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecentPlayersUpdatedDelegate, const TArray<FRecentPlayer>&, RecentPlayers);

UCLASS(BlueprintType)
class MMOPARTY_API UMMOFriendsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	bool SendFriendRequest(const FString& FromId, const FString& FromName, const FString& ToId, const FString& ToName, const FString& Message = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	bool AcceptFriendRequest(const FString& RequestId, const FString& AcceptingPlayerId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	bool DeclineFriendRequest(const FString& RequestId, const FString& DecliningPlayerId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	bool CancelFriendRequest(const FString& RequestId, const FString& CancellingPlayerId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	bool RemoveFriend(const FString& PlayerId, const FString& FriendId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	TArray<FFriendEntry> GetFriendsList(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	TArray<FFriendRequest> GetPendingFriendRequests(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	bool SetFriendNote(const FString& PlayerId, const FString& FriendId, const FString& Note);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	bool SetFriendGroup(const FString& PlayerId, const FString& FriendId, const FString& Group);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	bool AddToIgnoreList(const FString& PlayerId, const FString& IgnoredPlayerId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	bool RemoveFromIgnoreList(const FString& PlayerId, const FString& IgnoredPlayerId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	TArray<FString> GetIgnoreList(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	bool IsIgnored(const FString& PlayerId, const FString& TargetPlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	TArray<FFriendEntry> SearchPlayers(const FString& SearchTerm, int32 MaxResults = 50) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	void AddToRecentPlayers(const FString& PlayerId, const FString& TargetId, const FString& TargetName, const FString& InteractionType);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	TArray<FRecentPlayer> GetRecentPlayers(const FString& PlayerId, int32 MaxResults = 50) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	void TrackWhisper(const FString& SenderId, const FString& SenderName, const FString& ReceiverId, const FString& MessageText, bool bIsIncoming);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	FWhisperConversation GetWhisperConversation(const FString& PlayerId, const FString& OtherPlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	TArray<FWhisperConversation> GetAllWhisperConversations(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	void MarkConversationRead(const FString& PlayerId, const FString& OtherPlayerId);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	void NotifyFriendOnline(const FString& PlayerId, const FString& FriendId, bool bOnline);

	UFUNCTION(BlueprintCallable, Category = "MMO|Social")
	void UpdateFriendStatus(const FString& FriendId, int32 Level, const FString& Zone);

	UPROPERTY(BlueprintAssignable, Category = "MMO|Social")
	FOnFriendListUpdatedDelegate OnFriendListUpdated;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Social")
	FOnFriendRequestReceivedDelegate OnFriendRequestReceived;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Social")
	FOnFriendOnlineStatusDelegate OnFriendOnlineStatus;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Social")
	FOnRecentPlayersUpdatedDelegate OnRecentPlayersUpdated;

private:
	struct FPlayerSocialData
	{
		TArray<FFriendEntry> Friends;
		TArray<FFriendRequest> PendingRequests;
		TArray<FString> IgnoredPlayers;
		TArray<FRecentPlayer> RecentPlayers;
		TMap<FString, FWhisperConversation> WhisperConversations;
	};

	TMap<FString, FPlayerSocialData> SocialDataMap;
	TMap<FString, FFriendRequest> GlobalRequestMap;

	FString GenerateRequestId();
	FPlayerSocialData& GetOrCreateSocialData(const FString& PlayerId);
	void BroadcastFriendListUpdate(const FString& PlayerId);
};

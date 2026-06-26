#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Party/MMOPartySubsystem.h"
#include "MMOPartyMemberComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKickedFromPartyDelegate, const FString&, PartyId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyDisbandedDelegate, const FString&, PartyId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyCheckStartedDelegate, const FString&, InstigatorId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReadyCheckRespondedDelegate, const FString&, MemberId, bool, bReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoteKickInitiatedDelegate, const FString&, TargetId, const FString&, InstigatorId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartyChatMessageDelegate, const FString&, SenderName, const FString&, Message);

UCLASS(ClassGroup = (MMO), meta = (BlueprintSpawnableComponent))
class MMOPARTY_API UMMOPartyMemberComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOPartyMemberComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Party")
	FString CurrentPartyId;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Party")
	FParty CurrentPartyData;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Party")
	TArray<FPartyInvite> IncomingInvites;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Party")
	TArray<FPartyInvite> OutgoingInvites;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	FString GetPlayerId() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	FString GetPlayerName() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool IsPartyLeader() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Party")
	bool IsInParty() const { return !CurrentPartyId.IsEmpty(); }

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_CreateParty(EPartyType Type);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_LeaveParty();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_KickMember(const FString& TargetPlayerId);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_InviteToParty(const FString& TargetPlayerId);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_AcceptInvite(const FString& PartyId);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_DeclineInvite(const FString& PartyId);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_PromoteLeader(const FString& NewLeaderId);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_SetMemberRole(const FString& TargetId, EPartyMemberRole NewRole);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_SetLootMethod(ELootMethod NewMethod);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_SetExperienceSharing(bool bEnabled);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_SetQuestSharing(bool bEnabled);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_StartReadyCheck();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_RespondReadyCheck(bool bReady);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_InitiateVoteKick(const FString& TargetId);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_VoteOnKick(const FString& TargetId, bool bApprove);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_CreatePartyFinderListing(const FString& Description, const TArray<EPartyMemberRole>& NeededRoles, int32 MinLevel, int32 MaxLevel, const FString& DungeonTarget);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_RemovePartyFinderListing(const FString& ListingId);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "MMO|Party")
	void Server_UpdateMyStatus(float HealthPct, float ManaPct, const FString& Zone);

	UFUNCTION(Client, Reliable, Category = "MMO|Party")
	void Client_OnPartyUpdated(const FString& PartyId, const FParty& PartyData);

	UFUNCTION(Client, Reliable, Category = "MMO|Party")
	void Client_OnInviteReceived(const FPartyInvite& Invite);

	UFUNCTION(Client, Reliable, Category = "MMO|Party")
	void Client_OnInviteCancelled(const FString& PartyId, const FString& InviterId);

	UFUNCTION(Client, Reliable, Category = "MMO|Party")
	void Client_OnKickedFromParty(const FString& PartyId);

	UFUNCTION(Client, Reliable, Category = "MMO|Party")
	void Client_OnPartyDisbanded(const FString& PartyId);

	UFUNCTION(Client, Reliable, Category = "MMO|Party")
	void Client_OnReadyCheckStarted(const FString& InstigatorId);

	UFUNCTION(Client, Reliable, Category = "MMO|Party")
	void Client_OnReadyCheckResponse(const FString& MemberId, bool bReady);

	UFUNCTION(Client, Reliable, Category = "MMO|Party")
	void Client_OnVoteKickStarted(const FString& TargetId, const FString& InstigatorId);

	UFUNCTION(Client, Reliable, Category = "MMO|Party")
	void Client_OnVoteKickResult(const FString& TargetId, bool bKicked);

	UFUNCTION(Client, Reliable, Category = "MMO|Party")
	void Client_ReceivePartyChatMessage(const FString& SenderName, const FString& Message);

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnPartyUpdatedDelegate OnLocalPartyUpdated;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnPartyInviteReceivedDelegate OnLocalInviteReceived;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnKickedFromPartyDelegate OnKickedFromParty;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnPartyDisbandedDelegate OnPartyDisbanded;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnReadyCheckStartedDelegate OnReadyCheckStarted;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnReadyCheckRespondedDelegate OnReadyCheckResponded;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnVoteKickInitiatedDelegate OnVoteKickStarted;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Party")
	FOnPartyChatMessageDelegate OnPartyChatMessage;

private:
	void ClearExpiredInvites();
	FTimerHandle InviteCleanupTimer;

	static UMMOPartyMemberComponent* FindComponentForPlayer(UWorld* World, const FString& PlayerId);
};

#include "Party/MMOPartyMemberComponent.h"
#include "Party/MMOPartySubsystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

UMMOPartyMemberComponent::UMMOPartyMemberComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UMMOPartyMemberComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		GetWorld()->GetTimerManager().SetTimer(InviteCleanupTimer, this, &UMMOPartyMemberComponent::ClearExpiredInvites, 5.0f, true);
	}
}

void UMMOPartyMemberComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMMOPartyMemberComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UMMOPartyMemberComponent, CurrentPartyId);
}

FString UMMOPartyMemberComponent::GetPlayerId() const
{
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (APlayerState* PS = PC->PlayerState)
		{
			return PS->GetUniqueId().ToString();
		}
	}
	return TEXT("");
}

FString UMMOPartyMemberComponent::GetPlayerName() const
{
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (APlayerState* PS = PC->PlayerState)
		{
			return PS->GetPlayerName();
		}
	}
	return TEXT("");
}

bool UMMOPartyMemberComponent::IsPartyLeader() const
{
	return CurrentPartyData.LeaderId == GetPlayerId();
}

// --- Server RPCs ---

bool UMMOPartyMemberComponent::Server_CreateParty_Validate(EPartyType Type) { return true; }
void UMMOPartyMemberComponent::Server_CreateParty_Implementation(EPartyType Type)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	FString PlayerName = GetPlayerName();
	if (PlayerId.IsEmpty()) return;

	FString PartyId = Sub->CreateParty(PlayerId, PlayerName, Type);
	if (!PartyId.IsEmpty())
	{
		CurrentPartyId = PartyId;
		CurrentPartyData = Sub->GetParty(PartyId);
		Client_OnPartyUpdated(PartyId, CurrentPartyData);
	}
}

bool UMMOPartyMemberComponent::Server_LeaveParty_Validate() { return true; }
void UMMOPartyMemberComponent::Server_LeaveParty_Implementation()
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	if (CurrentPartyId.IsEmpty()) return;

	FParty PartyData = CurrentPartyData;
	if (Sub->LeaveParty(CurrentPartyId, PlayerId))
	{
		CurrentPartyId.Empty();
		CurrentPartyData = FParty();
	}
}

bool UMMOPartyMemberComponent::Server_KickMember_Validate(const FString& TargetPlayerId) { return !TargetPlayerId.IsEmpty(); }
void UMMOPartyMemberComponent::Server_KickMember_Implementation(const FString& TargetPlayerId)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	if (Sub->KickFromParty(CurrentPartyId, PlayerId, TargetPlayerId))
	{
		CurrentPartyData = Sub->GetParty(CurrentPartyId);

		UMMOPartyMemberComponent* TargetComp = FindComponentForPlayer(GetWorld(), TargetPlayerId);
		if (TargetComp)
		{
			TargetComp->CurrentPartyId.Empty();
			TargetComp->CurrentPartyData = FParty();
			TargetComp->Client_OnKickedFromParty(CurrentPartyId);
		}
	}
}

bool UMMOPartyMemberComponent::Server_InviteToParty_Validate(const FString& TargetPlayerId) { return !TargetPlayerId.IsEmpty(); }
void UMMOPartyMemberComponent::Server_InviteToParty_Implementation(const FString& TargetPlayerId)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	FString PlayerName = GetPlayerName();

	if (Sub->SendPartyInvite(PlayerId, PlayerName, TargetPlayerId))
	{
		FPartyInvite Invite;
		Invite.InviterId = PlayerId;
		Invite.InviterName = PlayerName;
		Invite.InviteeId = TargetPlayerId;
		Invite.PartyId = CurrentPartyId;

		UMMOPartyMemberComponent* TargetComp = FindComponentForPlayer(GetWorld(), TargetPlayerId);
		if (TargetComp)
		{
			TargetComp->IncomingInvites.Add(Invite);
			TargetComp->Client_OnInviteReceived(Invite);
		}

		OutgoingInvites.Add(Invite);
	}
}

bool UMMOPartyMemberComponent::Server_AcceptInvite_Validate(const FString& PartyId) { return !PartyId.IsEmpty(); }
void UMMOPartyMemberComponent::Server_AcceptInvite_Implementation(const FString& PartyId)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	if (Sub->AcceptPartyInvite(PlayerId, PartyId))
	{
		CurrentPartyId = PartyId;
		CurrentPartyData = Sub->GetParty(PartyId);

		IncomingInvites.RemoveAll([&](const FPartyInvite& I) { return I.PartyId == PartyId; });

		Client_OnPartyUpdated(PartyId, CurrentPartyData);
	}
}

bool UMMOPartyMemberComponent::Server_DeclineInvite_Validate(const FString& PartyId) { return !PartyId.IsEmpty(); }
void UMMOPartyMemberComponent::Server_DeclineInvite_Implementation(const FString& PartyId)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	if (Sub->DeclinePartyInvite(PlayerId, PartyId))
	{
		IncomingInvites.RemoveAll([&](const FPartyInvite& I) { return I.PartyId == PartyId; });
	}
}

bool UMMOPartyMemberComponent::Server_PromoteLeader_Validate(const FString& NewLeaderId) { return !NewLeaderId.IsEmpty(); }
void UMMOPartyMemberComponent::Server_PromoteLeader_Implementation(const FString& NewLeaderId)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	if (Sub->PromoteToLeader(CurrentPartyId, PlayerId, NewLeaderId))
	{
		CurrentPartyData = Sub->GetParty(CurrentPartyId);
		Client_OnPartyUpdated(CurrentPartyId, CurrentPartyData);
	}
}

bool UMMOPartyMemberComponent::Server_SetMemberRole_Validate(const FString& TargetId, EPartyMemberRole NewRole) { return !TargetId.IsEmpty(); }
void UMMOPartyMemberComponent::Server_SetMemberRole_Implementation(const FString& TargetId, EPartyMemberRole NewRole)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	if (Sub->SetMemberRole(CurrentPartyId, PlayerId, TargetId, NewRole))
	{
		CurrentPartyData = Sub->GetParty(CurrentPartyId);
		Client_OnPartyUpdated(CurrentPartyId, CurrentPartyData);
	}
}

bool UMMOPartyMemberComponent::Server_SetLootMethod_Validate(ELootMethod NewMethod) { return true; }
void UMMOPartyMemberComponent::Server_SetLootMethod_Implementation(ELootMethod NewMethod)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	if (Sub->SetLootMethod(CurrentPartyId, PlayerId, NewMethod))
	{
		CurrentPartyData = Sub->GetParty(CurrentPartyId);
		Client_OnPartyUpdated(CurrentPartyId, CurrentPartyData);
	}
}

bool UMMOPartyMemberComponent::Server_SetExperienceSharing_Validate(bool bEnabled) { return true; }
void UMMOPartyMemberComponent::Server_SetExperienceSharing_Implementation(bool bEnabled)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	if (Sub->SetExperienceSharing(CurrentPartyId, bEnabled))
	{
		CurrentPartyData = Sub->GetParty(CurrentPartyId);
		Client_OnPartyUpdated(CurrentPartyId, CurrentPartyData);
	}
}

bool UMMOPartyMemberComponent::Server_SetQuestSharing_Validate(bool bEnabled) { return true; }
void UMMOPartyMemberComponent::Server_SetQuestSharing_Implementation(bool bEnabled)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	if (Sub->SetQuestSharing(CurrentPartyId, bEnabled))
	{
		CurrentPartyData = Sub->GetParty(CurrentPartyId);
		Client_OnPartyUpdated(CurrentPartyId, CurrentPartyData);
	}
}

// --- Ready Check ---

bool UMMOPartyMemberComponent::Server_StartReadyCheck_Validate() { return true; }
void UMMOPartyMemberComponent::Server_StartReadyCheck_Implementation()
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	if (Sub->StartReadyCheck(CurrentPartyId, PlayerId))
	{
		FParty PartyData = Sub->GetParty(CurrentPartyId);
		for (const FPartyMember& Member : PartyData.Members)
		{
			UMMOPartyMemberComponent* Comp = FindComponentForPlayer(GetWorld(), Member.PlayerId);
			if (Comp)
			{
				Comp->Client_OnReadyCheckStarted(PlayerId);
			}
		}
	}
}

bool UMMOPartyMemberComponent::Server_RespondReadyCheck_Validate(bool bReady) { return true; }
void UMMOPartyMemberComponent::Server_RespondReadyCheck_Implementation(bool bReady)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	if (Sub->RespondReadyCheck(CurrentPartyId, PlayerId, bReady))
	{
		FParty PartyData = Sub->GetParty(CurrentPartyId);
		for (const FPartyMember& Member : PartyData.Members)
		{
			UMMOPartyMemberComponent* Comp = FindComponentForPlayer(GetWorld(), Member.PlayerId);
			if (Comp)
			{
				Comp->Client_OnReadyCheckResponse(PlayerId, bReady);
			}
		}
	}
}

// --- Vote Kick ---

bool UMMOPartyMemberComponent::Server_InitiateVoteKick_Validate(const FString& TargetId) { return !TargetId.IsEmpty(); }
void UMMOPartyMemberComponent::Server_InitiateVoteKick_Implementation(const FString& TargetId)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	if (Sub->InitiateVoteKick(CurrentPartyId, PlayerId, TargetId))
	{
		FParty PartyData = Sub->GetParty(CurrentPartyId);
		for (const FPartyMember& Member : PartyData.Members)
		{
			UMMOPartyMemberComponent* Comp = FindComponentForPlayer(GetWorld(), Member.PlayerId);
			if (Comp)
			{
				Comp->Client_OnVoteKickStarted(TargetId, PlayerId);
			}
		}
	}
}

bool UMMOPartyMemberComponent::Server_VoteOnKick_Validate(const FString& TargetId, bool bApprove) { return !TargetId.IsEmpty(); }
void UMMOPartyMemberComponent::Server_VoteOnKick_Implementation(const FString& TargetId, bool bApprove)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FString PlayerId = GetPlayerId();
	Sub->VoteOnKick(CurrentPartyId, PlayerId, bApprove);
}

// --- Party Finder ---

bool UMMOPartyMemberComponent::Server_CreatePartyFinderListing_Validate(const FString& Description, const TArray<EPartyMemberRole>& NeededRoles, int32 MinLevel, int32 MaxLevel, const FString& DungeonTarget) { return true; }
void UMMOPartyMemberComponent::Server_CreatePartyFinderListing_Implementation(const FString& Description, const TArray<EPartyMemberRole>& NeededRoles, int32 MinLevel, int32 MaxLevel, const FString& DungeonTarget)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	FPartyFinderListing Listing;
	Listing.LeaderId = GetPlayerId();
	Listing.LeaderName = GetPlayerName();
	Listing.Description = Description;
	Listing.NeededRoles = NeededRoles;
	Listing.MinLevel = MinLevel;
	Listing.MaxLevel = MaxLevel;
	Listing.DungeonTarget = DungeonTarget;
	Listing.bIsRaid = (Sub->GetParty(CurrentPartyId).Type == EPartyType::Raid);

	Sub->CreatePartyFinderListing(Listing);
}

bool UMMOPartyMemberComponent::Server_RemovePartyFinderListing_Validate(const FString& ListingId) { return !ListingId.IsEmpty(); }
void UMMOPartyMemberComponent::Server_RemovePartyFinderListing_Implementation(const FString& ListingId)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	Sub->RemovePartyFinderListing(ListingId, GetPlayerId());
}

bool UMMOPartyMemberComponent::Server_UpdateMyStatus_Validate(float HealthPct, float ManaPct, const FString& Zone) { return true; }
void UMMOPartyMemberComponent::Server_UpdateMyStatus_Implementation(float HealthPct, float ManaPct, const FString& Zone)
{
	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	Sub->UpdatePartyMember(CurrentPartyId, GetPlayerId(), HealthPct, ManaPct, Zone, true);
}

// --- Client RPCs ---

void UMMOPartyMemberComponent::Client_OnPartyUpdated_Implementation(const FString& PartyId, const FParty& PartyData)
{
	CurrentPartyId = PartyId;
	CurrentPartyData = PartyData;
	OnLocalPartyUpdated.Broadcast(PartyId, PartyData);
}

void UMMOPartyMemberComponent::Client_OnInviteReceived_Implementation(const FPartyInvite& Invite)
{
	IncomingInvites.Add(Invite);
	OnLocalInviteReceived.Broadcast(Invite);
}

void UMMOPartyMemberComponent::Client_OnInviteCancelled_Implementation(const FString& PartyId, const FString& InviterId)
{
	IncomingInvites.RemoveAll([&](const FPartyInvite& I) { return I.PartyId == PartyId && I.InviterId == InviterId; });
}

void UMMOPartyMemberComponent::Client_OnKickedFromParty_Implementation(const FString& PartyId)
{
	CurrentPartyId.Empty();
	CurrentPartyData = FParty();
	OnKickedFromParty.Broadcast(PartyId);
}

void UMMOPartyMemberComponent::Client_OnPartyDisbanded_Implementation(const FString& PartyId)
{
	if (CurrentPartyId == PartyId)
	{
		CurrentPartyId.Empty();
		CurrentPartyData = FParty();
		OnPartyDisbanded.Broadcast(PartyId);
	}
}

void UMMOPartyMemberComponent::Client_OnReadyCheckStarted_Implementation(const FString& InstigatorId)
{
	OnReadyCheckStarted.Broadcast(InstigatorId);
}

void UMMOPartyMemberComponent::Client_OnReadyCheckResponse_Implementation(const FString& MemberId, bool bReady)
{
	OnReadyCheckResponded.Broadcast(MemberId, bReady);
}

void UMMOPartyMemberComponent::Client_OnVoteKickStarted_Implementation(const FString& TargetId, const FString& InstigatorId)
{
	OnVoteKickStarted.Broadcast(TargetId, InstigatorId);
}

void UMMOPartyMemberComponent::Client_OnVoteKickResult_Implementation(const FString& TargetId, bool bKicked)
{
	if (bKicked && GetPlayerId() == TargetId)
	{
		CurrentPartyId.Empty();
		CurrentPartyData = FParty();
		OnKickedFromParty.Broadcast(CurrentPartyId);
	}
	CurrentPartyData = GetWorld()->GetSubsystem<UMMOPartySubsystem>()->GetParty(CurrentPartyId);
}

void UMMOPartyMemberComponent::Client_ReceivePartyChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
	OnPartyChatMessage.Broadcast(SenderName, Message);
}

void UMMOPartyMemberComponent::ClearExpiredInvites()
{
	if (GetOwnerRole() != ROLE_Authority) return;

	UMMOPartySubsystem* Sub = GetWorld()->GetSubsystem<UMMOPartySubsystem>();
	if (!Sub) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	OutgoingInvites.RemoveAll([&](const FPartyInvite& I) { return (CurrentTime - I.Timestamp) > 60.0f; });

	for (int32 i = IncomingInvites.Num() - 1; i >= 0; --i)
	{
		if ((CurrentTime - IncomingInvites[i].Timestamp) > 60.0f)
		{
			Sub->DeclinePartyInvite(IncomingInvites[i].InviteeId, IncomingInvites[i].PartyId);
			IncomingInvites.RemoveAt(i);
		}
	}
}

UMMOPartyMemberComponent* UMMOPartyMemberComponent::FindComponentForPlayer(UWorld* World, const FString& PlayerId)
{
	if (!World) return nullptr;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			UMMOPartyMemberComponent* Comp = PC->FindComponentByClass<UMMOPartyMemberComponent>();
			if (Comp && Comp->GetPlayerId() == PlayerId)
			{
				return Comp;
			}
		}
	}
	return nullptr;
}

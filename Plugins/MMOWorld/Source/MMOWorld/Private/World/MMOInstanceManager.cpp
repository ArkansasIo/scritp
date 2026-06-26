#include "World/MMOInstanceManager.h"
#include "MMOWorld.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMMOInstanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InstanceIDCounter = 0;

	UE_LOG(LogMMOWorld, Log, TEXT("MMOInstanceManager initialized."));
}

void UMMOInstanceManager::Deinitialize()
{
	ActiveInstances.Empty();
	PlayerLockouts.Empty();

	Super::Deinitialize();
}

FName UMMOInstanceManager::GenerateInstanceID()
{
	InstanceIDCounter++;
	FString InstanceIDStr = FString::Printf(TEXT("Instance_%d_%lld"), InstanceIDCounter, FDateTime::Now().GetTicks());
	return FName(*InstanceIDStr);
}

FName UMMOInstanceManager::CreateInstance(FName ZoneID, EMMOInstanceDifficulty Difficulty, const TArray<FString>& AssignedPlayers, const FString& OwnerPartyOrRaidID)
{
	FName InstanceID = GenerateInstanceID();

	FMMOInstanceInfo InstanceInfo;
	InstanceInfo.InstanceID = InstanceID;
	InstanceInfo.ZoneID = ZoneID;
	InstanceInfo.Difficulty = Difficulty;
	InstanceInfo.State = EMMOInstanceState::InProgress;
	InstanceInfo.AssignedPlayerIDs = AssignedPlayers;
	InstanceInfo.OwnerPartyOrRaidID = OwnerPartyOrRaidID;
	InstanceInfo.CurrentPlayerCount = AssignedPlayers.Num();

	switch (Difficulty)
	{
	case EMMOInstanceDifficulty::Normal:
		InstanceInfo.MaxPlayers = 5;
		InstanceInfo.TimeLimitMinutes = 0.0f;
		break;
	case EMMOInstanceDifficulty::Heroic:
		InstanceInfo.MaxPlayers = 5;
		InstanceInfo.TimeLimitMinutes = 120.0f;
		break;
	case EMMOInstanceDifficulty::Mythic:
		InstanceInfo.MaxPlayers = 5;
		InstanceInfo.TimeLimitMinutes = 90.0f;
		break;
	}

	InstanceInfo.CreationTime = FDateTime::Now();
	InstanceInfo.LastActivityTime = FDateTime::Now();

	ActiveInstances.Add(InstanceID, InstanceInfo);

	for (const FString& PlayerID : AssignedPlayers)
	{
		int32 MaxCompletions = (Difficulty == EMMOInstanceDifficulty::Mythic) ? 1 : 10;
		EnforceLockout(PlayerID, ZoneID, MaxCompletions, true);
	}

	OnInstanceCreated.Broadcast(InstanceID);

	UE_LOG(LogMMOWorld, Log, TEXT("Created instance %s for zone %s with %d players."),
		*InstanceID.ToString(), *ZoneID.ToString(), AssignedPlayers.Num());

	return InstanceID;
}

bool UMMOInstanceManager::DestroyInstance(FName InstanceID)
{
	if (FMMOInstanceInfo* InstanceInfo = ActiveInstances.Find(InstanceID))
	{
		InstanceInfo->State = EMMOInstanceState::Expired;
		ActiveInstances.Remove(InstanceID);

		UE_LOG(LogMMOWorld, Log, TEXT("Destroyed instance %s."), *InstanceID.ToString());
		return true;
	}
	return false;
}

bool UMMOInstanceManager::AddPlayerToInstance(FName InstanceID, const FString& PlayerID)
{
	FMMOInstanceInfo* InstanceInfo = ActiveInstances.Find(InstanceID);
	if (!InstanceInfo)
	{
		UE_LOG(LogMMOWorld, Warning, TEXT("Instance %s not found."), *InstanceID.ToString());
		return false;
	}

	if (InstanceInfo->State != EMMOInstanceState::InProgress)
	{
		UE_LOG(LogMMOWorld, Warning, TEXT("Instance %s is not in progress."), *InstanceID.ToString());
		return false;
	}

	if (InstanceInfo->CurrentPlayerCount >= InstanceInfo->MaxPlayers)
	{
		UE_LOG(LogMMOWorld, Warning, TEXT("Instance %s is full."), *InstanceID.ToString());
		return false;
	}

	if (InstanceInfo->AssignedPlayerIDs.Contains(PlayerID))
	{
		UE_LOG(LogMMOWorld, Verbose, TEXT("Player %s is already in instance %s."), *PlayerID, *InstanceID.ToString());
		return true;
	}

	InstanceInfo->AssignedPlayerIDs.Add(PlayerID);
	InstanceInfo->CurrentPlayerCount++;
	InstanceInfo->LastActivityTime = FDateTime::Now();

	return true;
}

bool UMMOInstanceManager::RemovePlayerFromInstance(FName InstanceID, const FString& PlayerID)
{
	FMMOInstanceInfo* InstanceInfo = ActiveInstances.Find(InstanceID);
	if (!InstanceInfo)
	{
		return false;
	}

	if (InstanceInfo->AssignedPlayerIDs.Remove(PlayerID))
	{
		InstanceInfo->CurrentPlayerCount--;
		InstanceInfo->LastActivityTime = FDateTime::Now();

		if (InstanceInfo->CurrentPlayerCount <= 0)
		{
			ExpireInstance(InstanceID);
		}

		return true;
	}

	return false;
}

FMMOInstanceInfo UMMOInstanceManager::GetInstanceInfo(FName InstanceID) const
{
	if (const FMMOInstanceInfo* InstanceInfo = ActiveInstances.Find(InstanceID))
	{
		return *InstanceInfo;
	}
	return FMMOInstanceInfo();
}

TArray<FName> UMMOInstanceManager::GetPlayerInstances(const FString& PlayerID) const
{
	TArray<FName> PlayerInstances;
	for (const auto& Pair : ActiveInstances)
	{
		if (Pair.Value.AssignedPlayerIDs.Contains(PlayerID))
		{
			PlayerInstances.Add(Pair.Key);
		}
	}
	return PlayerInstances;
}

void UMMOInstanceManager::CompleteInstance(FName InstanceID)
{
	FMMOInstanceInfo* InstanceInfo = ActiveInstances.Find(InstanceID);
	if (!InstanceInfo)
	{
		return;
	}

	InstanceInfo->State = EMMOInstanceState::Completed;
	InstanceInfo->LastActivityTime = FDateTime::Now();

	OnInstanceCompleted.Broadcast(InstanceID);

	UE_LOG(LogMMOWorld, Log, TEXT("Instance %s completed."), *InstanceID.ToString());
}

void UMMOInstanceManager::ExpireInstance(FName InstanceID)
{
	FMMOInstanceInfo* InstanceInfo = ActiveInstances.Find(InstanceID);
	if (!InstanceInfo)
	{
		return;
	}

	InstanceInfo->State = EMMOInstanceState::Expired;
	ActiveInstances.Remove(InstanceID);

	OnInstanceExpired.Broadcast(InstanceID);

	UE_LOG(LogMMOWorld, Log, TEXT("Instance %s expired."), *InstanceID.ToString());
}

void UMMOInstanceManager::ResetInstance(FName InstanceID)
{
	FMMOInstanceInfo* InstanceInfo = ActiveInstances.Find(InstanceID);
	if (!InstanceInfo)
	{
		return;
	}

	InstanceInfo->State = EMMOInstanceState::InProgress;
	InstanceInfo->LastActivityTime = FDateTime::Now();
	InstanceInfo->SaveData.Empty();
	InstanceInfo->bIsSaved = false;

	UE_LOG(LogMMOWorld, Log, TEXT("Instance %s has been reset."), *InstanceID.ToString());
}

bool UMMOInstanceManager::HasInstanceLockout(const FString& PlayerID, FName ZoneID) const
{
	if (const TArray<FMMOInstanceLockout>* Lockouts = PlayerLockouts.Find(PlayerID))
	{
		for (const FMMOInstanceLockout& Lockout : *Lockouts)
		{
			if (Lockout.InstanceID == ZoneID && FDateTime::Now() < Lockout.LockoutExpires)
			{
				if (Lockout.CompletedCount >= Lockout.MaxCompletions)
				{
					return true;
				}
			}
		}
	}
	return false;
}

FMMOInstanceLockout UMMOInstanceManager::GetPlayerLockout(const FString& PlayerID, FName ZoneID) const
{
	if (const TArray<FMMOInstanceLockout>* Lockouts = PlayerLockouts.Find(PlayerID))
	{
		for (const FMMOInstanceLockout& Lockout : *Lockouts)
		{
			if (Lockout.InstanceID == ZoneID)
			{
				return Lockout;
			}
		}
	}
	return FMMOInstanceLockout();
}

void UMMOInstanceManager::ClearPlayerLockout(const FString& PlayerID, FName ZoneID)
{
	if (TArray<FMMOInstanceLockout>* Lockouts = PlayerLockouts.Find(PlayerID))
	{
		Lockouts->RemoveAll([ZoneID](const FMMOInstanceLockout& Lockout)
		{
			return Lockout.InstanceID == ZoneID;
		});
	}
}

void UMMOInstanceManager::SaveInstanceState(FName InstanceID)
{
	FMMOInstanceInfo* InstanceInfo = ActiveInstances.Find(InstanceID);
	if (!InstanceInfo)
	{
		return;
	}

	InstanceInfo->bIsSaved = true;
	InstanceInfo->LastActivityTime = FDateTime::Now();

	UE_LOG(LogMMOWorld, Log, TEXT("Instance %s state saved."), *InstanceID.ToString());
}

void UMMOInstanceManager::LoadInstanceState(FName InstanceID)
{
	FMMOInstanceInfo* InstanceInfo = ActiveInstances.Find(InstanceID);
	if (!InstanceInfo || !InstanceInfo->bIsSaved)
	{
		return;
	}

	UE_LOG(LogMMOWorld, Log, TEXT("Instance %s state loaded."), *InstanceID.ToString());
}

void UMMOInstanceManager::CheckExpiredInstances()
{
	TArray<FName> ExpiredIDs;
	FDateTime Now = FDateTime::Now();

	for (const auto& Pair : ActiveInstances)
	{
		if (Pair.Value.State == EMMOInstanceState::InProgress && Pair.Value.TimeLimitMinutes > 0.0f)
		{
			FTimespan Elapsed = Now - Pair.Value.CreationTime;
			if (Elapsed.GetTotalMinutes() >= Pair.Value.TimeLimitMinutes)
			{
				ExpiredIDs.Add(Pair.Key);
			}
		}
	}

	for (FName ID : ExpiredIDs)
	{
		ExpireInstance(ID);
	}
}

void UMMOInstanceManager::EnforceLockout(const FString& PlayerID, FName ZoneID, int32 MaxCompletions, bool bIsWeekly)
{
	FMMOInstanceLockout Lockout;
	Lockout.InstanceID = ZoneID;
	Lockout.PlayerID = PlayerID;
	Lockout.MaxCompletions = MaxCompletions;
	Lockout.bIsWeekly = bIsWeekly;

	if (bIsWeekly)
	{
		FDateTime Now = FDateTime::Now();
		int32 DaysUntilNextReset = (7 - (int32)Now.GetDayOfWeek()) % 7;
		if (DaysUntilNextReset == 0) DaysUntilNextReset = 7;
		Lockout.LockoutExpires = Now + FTimespan(DaysUntilNextReset, 0, 0, 0);
	}
	else
	{
		Lockout.LockoutExpires = FDateTime::Now() + FTimespan(1, 0, 0, 0);
	}

	TArray<FMMOInstanceLockout>& PlayerLocks = PlayerLockouts.FindOrAdd(PlayerID);

	FMMOInstanceLockout* Existing = PlayerLocks.FindByPredicate([ZoneID](const FMMOInstanceLockout& L)
	{
		return L.InstanceID == ZoneID;
	});

	if (Existing)
	{
		Existing->CompletedCount++;
	}
	else
	{
		Lockout.CompletedCount = 1;
		PlayerLocks.Add(Lockout);
	}
}

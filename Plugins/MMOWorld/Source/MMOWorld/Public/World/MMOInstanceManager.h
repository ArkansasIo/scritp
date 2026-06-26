#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MMOInstanceManager.generated.h"

UENUM(BlueprintType)
enum class EMMOInstanceDifficulty : uint8
{
	Normal	UMETA(DisplayName = "Normal"),
	Heroic	UMETA(DisplayName = "Heroic"),
	Mythic	UMETA(DisplayName = "Mythic")
};

UENUM(BlueprintType)
enum class EMMOInstanceState : uint8
{
	InProgress	UMETA(DisplayName = "In Progress"),
	Completed	UMETA(DisplayName = "Completed"),
	Expired		UMETA(DisplayName = "Expired")
};

USTRUCT(BlueprintType)
struct FMMOInstanceLockout
{
	GENERATED_BODY()

	UPROPERTY()
	FName InstanceID;

	UPROPERTY()
	FString PlayerID;

	UPROPERTY()
	int32 CompletedCount = 0;

	UPROPERTY()
	int32 MaxCompletions = 1;

	UPROPERTY()
	FDateTime LockoutExpires;

	UPROPERTY()
	bool bIsWeekly = true;
};

USTRUCT(BlueprintType)
struct FMMOInstanceInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FName InstanceID;

	UPROPERTY()
	FName ZoneID;

	UPROPERTY()
	EMMOInstanceDifficulty Difficulty = EMMOInstanceDifficulty::Normal;

	UPROPERTY()
	EMMOInstanceState State = EMMOInstanceState::InProgress;

	UPROPERTY()
	TArray<FString> AssignedPlayerIDs;

	UPROPERTY()
	FString OwnerPartyOrRaidID;

	UPROPERTY()
	int32 MaxPlayers = 5;

	UPROPERTY()
	int32 CurrentPlayerCount = 0;

	UPROPERTY()
	FDateTime CreationTime;

	UPROPERTY()
	FDateTime LastActivityTime;

	UPROPERTY()
	float TimeLimitMinutes = 0.0f;

	UPROPERTY()
	TArray<uint8> SaveData;

	UPROPERTY()
	bool bIsSaved = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstanceCreated, FName, InstanceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstanceCompleted, FName, InstanceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstanceExpired, FName, InstanceID);

UCLASS()
class MMWORLD_API UMMOInstanceManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Instance")
	FName CreateInstance(FName ZoneID, EMMOInstanceDifficulty Difficulty, const TArray<FString>& AssignedPlayers, const FString& OwnerPartyOrRaidID);

	UFUNCTION(BlueprintCallable, Category = "Instance")
	bool DestroyInstance(FName InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Instance")
	bool AddPlayerToInstance(FName InstanceID, const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Instance")
	bool RemovePlayerFromInstance(FName InstanceID, const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Instance")
	FMMOInstanceInfo GetInstanceInfo(FName InstanceID) const;

	UFUNCTION(BlueprintCallable, Category = "Instance")
	TArray<FName> GetPlayerInstances(const FString& PlayerID) const;

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void CompleteInstance(FName InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void ExpireInstance(FName InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void ResetInstance(FName InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Instance")
	bool HasInstanceLockout(const FString& PlayerID, FName ZoneID) const;

	UFUNCTION(BlueprintCallable, Category = "Instance")
	FMMOInstanceLockout GetPlayerLockout(const FString& PlayerID, FName ZoneID) const;

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void ClearPlayerLockout(const FString& PlayerID, FName ZoneID);

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void SaveInstanceState(FName InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void LoadInstanceState(FName InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Instance")
	int32 GetActiveInstanceCount() const { return ActiveInstances.Num(); }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInstanceCreated OnInstanceCreated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInstanceCompleted OnInstanceCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInstanceExpired OnInstanceExpired;

protected:
	UPROPERTY()
	TMap<FName, FMMOInstanceInfo> ActiveInstances;

	UPROPERTY()
	TMap<FString, TArray<FMMOInstanceLockout>> PlayerLockouts;

	UPROPERTY()
	int32 InstanceIDCounter = 0;

	FName GenerateInstanceID();

	void CheckExpiredInstances();
	void EnforceLockout(const FString& PlayerID, FName ZoneID, int32 MaxCompletions, bool bIsWeekly);
};

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Space/MMOSpaceTypes.h"
#include "MMOSQLDatabaseSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDatabaseConnected, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDataLoaded, FString, PlayerId, bool, bSuccess);

USTRUCT(BlueprintType)
struct FPlayerSaveData
{
	GENERATED_BODY()

	bool operator==(const FPlayerSaveData& Other) const
	{
		return PlayerId == Other.PlayerId && Username == Other.Username
			&& Resources == Other.Resources && Buildings == Other.Buildings
			&& Research == Other.Research && EmpireLevel == Other.EmpireLevel
			&& EmpireExperience == Other.EmpireExperience && PrestigeLevel == Other.PrestigeLevel
			&& HomeWorld == Other.HomeWorld && PlanetName == Other.PlanetName
			&& LastResourceUpdate == Other.LastResourceUpdate && LastTurnUpdate == Other.LastTurnUpdate
			&& CurrentTurns == Other.CurrentTurns && Government == Other.Government;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString Username;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FResourceSet Resources;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FBuildingLevel> Buildings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FResearchProgress> Research;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 EmpireLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int64 EmpireExperience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 PrestigeLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FGalaxyCoordinate HomeWorld;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString PlanetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	double LastResourceUpdate = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	double LastTurnUpdate = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 CurrentTurns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	EGovernmentType Government = EGovernmentType::Democracy;
};

UCLASS()
class MMOSPACE_API UMMOSQLDatabaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Database")
	FOnDatabaseConnected OnDatabaseConnected;

	UPROPERTY(BlueprintAssignable, Category = "Database")
	FOnPlayerDataLoaded OnPlayerDataLoaded;

	UFUNCTION(BlueprintCallable, Category = "Database")
	bool Connect(const FString& ConnectionString);

	UFUNCTION(BlueprintCallable, Category = "Database")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "Database")
	bool IsConnected() const { return bConnected; }

	UFUNCTION(BlueprintCallable, Category = "Database")
	void SavePlayerData(const FPlayerSaveData& PlayerData);

	UFUNCTION(BlueprintCallable, Category = "Database")
	void LoadPlayerData(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Database")
	void SaveResourceData(const FString& PlayerId, const FResourceSet& Resources);

	UFUNCTION(BlueprintCallable, Category = "Database")
	void SaveBuildingData(const FString& PlayerId, const TArray<FBuildingLevel>& Buildings);

	UFUNCTION(BlueprintCallable, Category = "Database")
	void SaveResearchData(const FString& PlayerId, const TArray<FResearchProgress>& Research);

	UFUNCTION(BlueprintCallable, Category = "Database")
	void SaveFleetData(const FString& PlayerId, const FString& FleetJson);

	UFUNCTION(BlueprintCallable, Category = "Database")
	FString GetConnectionStatus() const;

protected:
	UFUNCTION()
	void HandleDatabaseResponse(const FString& Response, bool bSuccess);

private:
	bool bConnected = false;
	FString CurrentConnectionString;
	TMap<FString, FPlayerSaveData> PlayerCache;

	bool ExecuteQuery(const FString& Query);
	void ProcessResourceTick(const FString& PlayerId, double DeltaTime);
};

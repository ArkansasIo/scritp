#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Space/MMOSpaceTypes.h"
#include "MMOFleetSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFleetDispatched, FString, FleetId, FGalaxyCoordinate, Destination);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFleetArrived, FString, FleetId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFleetReturned, FString, FleetId);

USTRUCT(BlueprintType)
struct FFleetUnit
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	EShipClass ShipClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	int32 Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	FShipStats Stats;
};

USTRUCT(BlueprintType)
struct FFleetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	FString FleetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	FString OwnerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	FString FleetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	TArray<FFleetUnit> Units;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	FGalaxyCoordinate Origin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	FGalaxyCoordinate Destination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	FGalaxyCoordinate CurrentPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	double DepartureTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	double ArrivalTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	double ReturnTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	double Speed = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	EFleetFormation Formation = EFleetFormation::LineAbreast;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	bool bIsReturning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	FString MissionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	TArray<FResourceSet> Loot;
};

USTRUCT(BlueprintType)
struct FShipBuildCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	EShipClass ShipClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	FResourceSet Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	double BuildTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	int32 RequiredShipyardLevel = 1;
};

UCLASS()
class MMOSPACE_API UMMOFleetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Fleet")
	FOnFleetDispatched OnFleetDispatched;

	UPROPERTY(BlueprintAssignable, Category = "Fleet")
	FOnFleetArrived OnFleetArrived;

	UPROPERTY(BlueprintAssignable, Category = "Fleet")
	FOnFleetReturned OnFleetReturned;

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	FString DispatchFleet(const TArray<FFleetUnit>& Units, const FGalaxyCoordinate& Destination, const FString& MissionType);

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	void RecallFleet(const FString& FleetId);

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	TArray<FFleetData> GetActiveFleets() const;

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	TArray<FFleetData> GetFleetsAtPosition(const FGalaxyCoordinate& Position) const;

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	FFleetData GetFleetById(const FString& FleetId) const;

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	int32 CalculateTravelTime(const FGalaxyCoordinate& Origin, const FGalaxyCoordinate& Dest, double Speed) const;

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	FShipStats CalculateShipStats(EShipClass ShipClass, int32 Level) const;

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	TArray<FShipBuildCost> GetShipBuildCosts() const;

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	FShipBuildCost GetShipBuildCost(EShipClass ShipClass) const;

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	void UpdateFleetPositions(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	int32 GetTotalFleetCount(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Fleet")
	double GetFleetSpeed(const TArray<FFleetUnit>& Units) const;

protected:
	void OnFleetArrival(FFleetData& Fleet);
	void OnFleetReturn(FFleetData& Fleet);
	FString GenerateFleetId();

private:
	TArray<FFleetData> ActiveFleets;
	TMap<EShipClass, FShipBuildCost> BuildCosts;
	int32 FleetCounter = 0;
};

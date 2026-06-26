#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Space/MMOSpaceTypes.h"
#include "MMOEconomySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeCompleted, FString, TradeId, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMarketOrderFilled, FString, OrderId, int32, AmountFilled);

USTRUCT(BlueprintType)
struct FBuildingProduction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	EBuildingType BuildingType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	FResourceSet ProductionPerHour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	FResourceSet EnergyConsumption;
};

USTRUCT(BlueprintType)
struct FBuildingCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	EBuildingType BuildingType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 CurrentLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	FResourceSet Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	double ConstructionTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	double EnergyCost = 0;
};

USTRUCT(BlueprintType)
struct FMarketOrder
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	FString OrderId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	bool bIsBuyOrder = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	EResourceType ResourceOffered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	int64 AmountOffered = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	EResourceType ResourceRequested;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	int64 AmountRequested = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	double PricePerUnit = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	double CreatedAt = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Market")
	bool bIsActive = true;
};

UCLASS()
class MMOSPACE_API UMMOEconomySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Economy")
	FOnTradeCompleted OnTradeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Market")
	FOnMarketOrderFilled OnMarketOrderFilled;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	FBuildingProduction GetBuildingProduction(EBuildingType Type, int32 Level) const;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	FBuildingCost GetBuildingCost(EBuildingType Type, int32 CurrentLevel) const;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	FResourceSet GetTotalProduction(const TArray<FBuildingLevel>& Buildings) const;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool CanAffordUpgrade(const FResourceSet& CurrentResources, const FBuildingCost& Cost) const;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	double CalculateBuildTime(EBuildingType Type, int32 TargetLevel, int32 RoboticsLevel = 0, int32 NaniteLevel = 0) const;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	void ProcessTickProduction(FResourceSet& Resources, const TArray<FBuildingLevel>& Buildings, double DeltaHours);

	UFUNCTION(BlueprintCallable, Category = "Market")
	FString CreateMarketOrder(const FString& PlayerId, bool bIsBuyOrder,
		EResourceType ResourceOffered, int64 AmountOffered,
		EResourceType ResourceRequested, int64 AmountRequested,
		double PricePerUnit);

	UFUNCTION(BlueprintCallable, Category = "Market")
	bool FillMarketOrder(const FString& OrderId, int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "Market")
	void CancelMarketOrder(const FString& OrderId);

	UFUNCTION(BlueprintCallable, Category = "Market")
	TArray<FMarketOrder> GetActiveOrders(EResourceType Resource) const;

	UFUNCTION(BlueprintCallable, Category = "Market")
	TArray<FMarketOrder> GetPlayerOrders(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Market")
	double GetMarketPrice(EResourceType Resource) const;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool CanBuild(EBuildingType Type, int32 CurrentLevel) const;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	double CalculateEnergyBalance(const TArray<FBuildingLevel>& Buildings) const;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	FString GetEconomyReport(const TArray<FBuildingLevel>& Buildings) const;

protected:
	static TMap<EBuildingType, FBuildingProduction> InitializeProductionDefaults();
	static TMap<EBuildingType, double> CalculateBaseCostMultiplier();

private:
	TMap<EBuildingType, FBuildingProduction> ProductionDefaults;
	TArray<FMarketOrder> ActiveMarketOrders;
	int32 MarketOrderCounter = 0;
};

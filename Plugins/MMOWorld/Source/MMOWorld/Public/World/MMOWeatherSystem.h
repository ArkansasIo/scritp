#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Containers/Ticker.h"
#include "MMOZoneData.h"
#include "MMOWeatherSystem.generated.h"

USTRUCT(BlueprintType)
struct FMMOWeatherState
{
	GENERATED_BODY()

	UPROPERTY()
	EMMOWeatherType CurrentWeather = EMMOWeatherType::Clear;

	UPROPERTY()
	EMMOWeatherType TargetWeather = EMMOWeatherType::Clear;

	UPROPERTY()
	float BlendProgress = 1.0f;

	UPROPERTY()
	float BlendDuration = 3.0f;

	UPROPERTY()
	float Intensity = 1.0f;

	UPROPERTY()
	FName ZoneID;
};

USTRUCT(BlueprintType)
struct FMMOWeatherCycleEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Cycle")
	EMMOWeatherType WeatherType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Cycle")
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Cycle")
	float MinDurationMinutes = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Cycle")
	float MaxDurationMinutes = 30.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeatherChanged, FName, ZoneID, EMMOWeatherType, NewWeather, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherBlendProgress, FName, ZoneID, float, Progress);

UCLASS()
class MMOWORLD_API UMMOWeatherSystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableInEditor() const override { return false; }

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeather(FName ZoneID, EMMOWeatherType NewWeather, float BlendDuration = 3.0f, float Intensity = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	FMMOWeatherState GetWeatherState(FName ZoneID) const;

	UFUNCTION(BlueprintCallable, Category = "Weather")
	EMMOWeatherType GetCurrentWeather(FName ZoneID) const;

	UFUNCTION(BlueprintCallable, Category = "Weather")
	float GetWeatherIntensity(FName ZoneID) const;

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetGlobalWeather(EMMOWeatherType NewWeather, float BlendDuration = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetPerformanceScaling(bool bEnable) { bUsePerformanceScaling = bEnable; }

	UFUNCTION(BlueprintCallable, Category = "Weather")
	bool IsUsingPerformanceScaling() const { return bUsePerformanceScaling; }

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void RegisterWeatherCycle(FName ZoneID, const TArray<FMMOWeatherCycleEntry>& CycleEntries);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void StartWeatherCycle(FName ZoneID);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void StopWeatherCycle(FName ZoneID);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void ServerSyncWeather(FName ZoneID);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeatherChanged OnWeatherChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeatherBlendProgress OnWeatherBlendProgress;

protected:
	UPROPERTY()
	TMap<FName, FMMOWeatherState> ZoneWeatherStates;

	TMap<FName, TArray<FMMOWeatherCycleEntry>> WeatherCycles;

	UPROPERTY()
	TMap<FName, float> CycleTimers;

	UPROPERTY()
	bool bUsePerformanceScaling = true;

	void ProcessWeatherBlends(float DeltaTime);
	void ProcessWeatherCycles(float DeltaTime);
	void SelectNextWeatherInCycle(FName ZoneID);
	void ApplyWeatherVisuals(FName ZoneID, EMMOWeatherType Weather, float Intensity);
};

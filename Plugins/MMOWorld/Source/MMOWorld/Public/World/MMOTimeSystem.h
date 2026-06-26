#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Containers/Ticker.h"
#include "MMOTimeSystem.generated.h"

USTRUCT(BlueprintType)
struct FMMOGameTime
{
	GENERATED_BODY()

	UPROPERTY()
	float GameTimeSeconds = 0.0f;

	UPROPERTY()
	float TimeOfDay = 6.0f;

	UPROPERTY()
	int32 DayNumber = 1;

	UPROPERTY()
	int32 MonthNumber = 1;

	UPROPERTY()
	int32 YearNumber = 1;

	UPROPERTY()
	float TimeSinceEpoch = 0.0f;
};

USTRUCT(BlueprintType)
struct FMMOTimeEvent
{
	GENERATED_BODY()

	UPROPERTY()
	FName EventID;

	UPROPERTY()
	FText EventName;

	UPROPERTY()
	FDateTime EventTime;

	UPROPERTY()
	bool bIsRecurring = false;

	UPROPERTY()
	bool bIsWeekly = false;

	UPROPERTY()
	bool bIsDaily = false;

	UPROPERTY()
	TArray<FName> AffectedZoneIDs;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, float, NewTimeOfDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayChanged, int32, NewDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDailyReset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeeklyReset);

UCLASS()
class MMOWORLD_API UMMOTimeSystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableInEditor() const override { return false; }

	UFUNCTION(BlueprintCallable, Category = "Time")
	float GetTimeOfDay() const { return CurrentTime.TimeOfDay; }

	UFUNCTION(BlueprintCallable, Category = "Time")
	int32 GetDayNumber() const { return CurrentTime.DayNumber; }

	UFUNCTION(BlueprintCallable, Category = "Time")
	int32 GetMonthNumber() const { return CurrentTime.MonthNumber; }

	UFUNCTION(BlueprintCallable, Category = "Time")
	int32 GetYearNumber() const { return CurrentTime.YearNumber; }

	UFUNCTION(BlueprintCallable, Category = "Time")
	FMMOGameTime GetCurrentTime() const { return CurrentTime; }

	UFUNCTION(BlueprintCallable, Category = "Time")
	float GetTimeSinceEpoch() const { return CurrentTime.TimeSinceEpoch; }

	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeSpeedMultiplier(float NewMultiplier);

	UFUNCTION(BlueprintCallable, Category = "Time")
	float GetTimeSpeedMultiplier() const { return TimeSpeedMultiplier; }

	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeOfDay(float NewTimeOfDay);

	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetServerAuthoritative(bool bAuthoritative) { bServerAuthoritative = bAuthoritative; }

	UFUNCTION(BlueprintCallable, Category = "Time")
	bool IsServerAuthoritative() const { return bServerAuthoritative; }

	UFUNCTION(BlueprintCallable, Category = "Time")
	void ScheduleEvent(const FMMOTimeEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Time")
	void CancelEvent(FName EventID);

	UFUNCTION(BlueprintCallable, Category = "Time")
	TArray<FMMOTimeEvent> GetUpcomingEvents(int32 MaxCount = 10) const;

	UFUNCTION(BlueprintCallable, Category = "Time")
	bool IsEventActive(FName EventID) const;

	UFUNCTION(BlueprintCallable, Category = "Time")
	float GetTimeUntilDailyReset() const;

	UFUNCTION(BlueprintCallable, Category = "Time")
	float GetTimeUntilWeeklyReset() const;

	UFUNCTION(BlueprintCallable, Category = "Time")
	void SyncTimeFromServer(const FMMOGameTime& ServerTime);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDayChanged OnDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDailyReset OnDailyReset;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeeklyReset OnWeeklyReset;

protected:
	UPROPERTY()
	FMMOGameTime CurrentTime;

	UPROPERTY()
	float TimeSpeedMultiplier = 1.0f;

	UPROPERTY()
	bool bServerAuthoritative = true;

	UPROPERTY()
	TArray<FMMOTimeEvent> ScheduledEvents;

	UPROPERTY()
	TArray<FName> ActiveEventIDs;

	UPROPERTY()
	float DailyResetTime = 6.0f;

	UPROPERTY()
	int32 LastDailyResetDay = 0;

	UPROPERTY()
	int32 LastWeeklyResetDay = 0;

	void AdvanceTime(float DeltaTime);
	void CheckDailyReset();
	void CheckWeeklyReset();
	void CheckScheduledEvents();
};

#include "World/MMOTimeSystem.h"
#include "MMOWorld.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void UMMOTimeSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentTime.TimeOfDay = 6.0f;
	CurrentTime.DayNumber = 1;
	CurrentTime.MonthNumber = 1;
	CurrentTime.YearNumber = 1;
	CurrentTime.TimeSinceEpoch = 0.0f;
	CurrentTime.GameTimeSeconds = 0.0f;

	TimeSpeedMultiplier = 1.0f;
	bServerAuthoritative = true;

	LastDailyResetDay = CurrentTime.DayNumber;
	LastWeeklyResetDay = CurrentTime.DayNumber;

	UE_LOG(LogMMOWorld, Log, TEXT("MMOTimeSystem initialized. TimeOfDay: %.1f, Day: %d"), CurrentTime.TimeOfDay, CurrentTime.DayNumber);
}

void UMMOTimeSystem::Deinitialize()
{
	ScheduledEvents.Empty();
	ActiveEventIDs.Empty();

	Super::Deinitialize();
}

void UMMOTimeSystem::Tick(float DeltaTime)
{
	AdvanceTime(DeltaTime);
	CheckDailyReset();
	CheckWeeklyReset();
	CheckScheduledEvents();
}

TStatId UMMOTimeSystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMMOTimeSystem, STATGROUP_Tickables);
}

void UMMOTimeSystem::SetTimeSpeedMultiplier(float NewMultiplier)
{
	TimeSpeedMultiplier = FMath::Clamp(NewMultiplier, 0.0f, 100.0f);
	UE_LOG(LogMMOWorld, Log, TEXT("Time speed multiplier set to %.2f"), TimeSpeedMultiplier);
}

void UMMOTimeSystem::SetTimeOfDay(float NewTimeOfDay)
{
	float OldTimeOfDay = CurrentTime.TimeOfDay;
	CurrentTime.TimeOfDay = FMath::Fmod(NewTimeOfDay, 24.0f);
	if (CurrentTime.TimeOfDay < 0.0f)
	{
		CurrentTime.TimeOfDay += 24.0f;
	}

	if (FMath::Abs(CurrentTime.TimeOfDay - OldTimeOfDay) > KINDA_SMALL_NUMBER)
	{
		OnTimeOfDayChanged.Broadcast(CurrentTime.TimeOfDay);
	}
}

void UMMOTimeSystem::ScheduleEvent(const FMMOTimeEvent& Event)
{
	ScheduledEvents.Add(Event);
	ScheduledEvents.Sort([](const FMMOTimeEvent& A, const FMMOTimeEvent& B)
	{
		return A.EventTime < B.EventTime;
	});
}

void UMMOTimeSystem::CancelEvent(FName EventID)
{
	ScheduledEvents.RemoveAll([EventID](const FMMOTimeEvent& Event)
	{
		return Event.EventID == EventID;
	});
	ActiveEventIDs.Remove(EventID);
}

TArray<FMMOTimeEvent> UMMOTimeSystem::GetUpcomingEvents(int32 MaxCount) const
{
	TArray<FMMOTimeEvent> Upcoming;
	FDateTime Now = FDateTime::Now();

	for (const FMMOTimeEvent& Event : ScheduledEvents)
	{
		if (Event.EventTime >= Now)
		{
			Upcoming.Add(Event);
			if (Upcoming.Num() >= MaxCount)
			{
				break;
			}
		}
	}

	return Upcoming;
}

bool UMMOTimeSystem::IsEventActive(FName EventID) const
{
	return ActiveEventIDs.Contains(EventID);
}

float UMMOTimeSystem::GetTimeUntilDailyReset() const
{
	float HoursUntilReset = DailyResetTime - CurrentTime.TimeOfDay;
	if (HoursUntilReset < 0.0f)
	{
		HoursUntilReset += 24.0f;
	}
	return HoursUntilReset;
}

float UMMOTimeSystem::GetTimeUntilWeeklyReset() const
{
	float DaysUntilReset = 7.0f - static_cast<float>(CurrentTime.DayNumber % 7);
	if (DaysUntilReset >= 7.0f)
	{
		DaysUntilReset -= 7.0f;
	}
	return (DaysUntilReset * 24.0f) + GetTimeUntilDailyReset();
}

void UMMOTimeSystem::SyncTimeFromServer(const FMMOGameTime& ServerTime)
{
	if (!bServerAuthoritative)
	{
		CurrentTime = ServerTime;
		OnTimeOfDayChanged.Broadcast(CurrentTime.TimeOfDay);
	}
}

void UMMOTimeSystem::AdvanceTime(float DeltaTime)
{
	if (!bServerAuthoritative && GetWorld() && GetWorld()->GetNetMode() != NM_DedicatedServer && GetWorld()->GetNetMode() != NM_ListenServer)
	{
		return;
	}

	float AdjustedDelta = DeltaTime * TimeSpeedMultiplier;
	CurrentTime.GameTimeSeconds += AdjustedDelta;
	CurrentTime.TimeSinceEpoch += AdjustedDelta;

	float OldTimeOfDay = CurrentTime.TimeOfDay;
	CurrentTime.TimeOfDay += AdjustedDelta / 60.0f;

	if (CurrentTime.TimeOfDay >= 24.0f)
	{
		CurrentTime.TimeOfDay -= 24.0f;
		CurrentTime.DayNumber++;

		if (CurrentTime.DayNumber > 30)
		{
			CurrentTime.DayNumber = 1;
			CurrentTime.MonthNumber++;
		}

		if (CurrentTime.MonthNumber > 12)
		{
			CurrentTime.MonthNumber = 1;
			CurrentTime.YearNumber++;
		}

		OnDayChanged.Broadcast(CurrentTime.DayNumber);
	}

	if (FMath::Abs(CurrentTime.TimeOfDay - OldTimeOfDay) > 0.01f)
	{
		OnTimeOfDayChanged.Broadcast(CurrentTime.TimeOfDay);
	}
}

void UMMOTimeSystem::CheckDailyReset()
{
	if (CurrentTime.DayNumber != LastDailyResetDay)
	{
		LastDailyResetDay = CurrentTime.DayNumber;

		if (FMath::IsNearlyEqual(CurrentTime.TimeOfDay, DailyResetTime, 0.1f))
		{
			OnDailyReset.Broadcast();
			UE_LOG(LogMMOWorld, Log, TEXT("Daily reset triggered for day %d."), CurrentTime.DayNumber);
		}
	}
}

void UMMOTimeSystem::CheckWeeklyReset()
{
	int32 CurrentWeek = CurrentTime.DayNumber / 7;
	int32 LastWeek = LastWeeklyResetDay / 7;

	if (CurrentWeek > LastWeek)
	{
		LastWeeklyResetDay = CurrentTime.DayNumber;
		OnWeeklyReset.Broadcast();
		UE_LOG(LogMMOWorld, Log, TEXT("Weekly reset triggered for week %d."), CurrentWeek);
	}
}

void UMMOTimeSystem::CheckScheduledEvents()
{
	FDateTime Now = FDateTime::Now();

	TArray<FName> ActivatedEvents;

	for (FMMOTimeEvent& Event : ScheduledEvents)
	{
		if (Event.EventTime <= Now && !ActiveEventIDs.Contains(Event.EventID))
		{
			ActiveEventIDs.Add(Event.EventID);
			ActivatedEvents.Add(Event.EventID);

			UE_LOG(LogMMOWorld, Log, TEXT("Time event activated: %s"), *Event.EventID.ToString());

			if (Event.bIsRecurring)
			{
				if (Event.bIsDaily)
				{
					Event.EventTime += FTimespan(1, 0, 0, 0);
				}
				else if (Event.bIsWeekly)
				{
					Event.EventTime += FTimespan(7, 0, 0, 0);
				}
			}
		}
	}
}

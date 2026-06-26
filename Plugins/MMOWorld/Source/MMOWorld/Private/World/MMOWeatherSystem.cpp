#include "World/MMOWeatherSystem.h"
#include "MMOWorld.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void UMMOWeatherSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogMMOWorld, Log, TEXT("MMOWeatherSystem initialized."));
}

void UMMOWeatherSystem::Deinitialize()
{
	ZoneWeatherStates.Empty();
	WeatherCycles.Empty();
	CycleTimers.Empty();

	Super::Deinitialize();
}

void UMMOWeatherSystem::Tick(float DeltaTime)
{
	ProcessWeatherBlends(DeltaTime);
	ProcessWeatherCycles(DeltaTime);
}

TStatId UMMOWeatherSystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMMOWeatherSystem, STATGROUP_Tickables);
}

void UMMOWeatherSystem::SetWeather(FName ZoneID, EMMOWeatherType NewWeather, float BlendDuration, float Intensity)
{
	FMMOWeatherState* State = ZoneWeatherStates.Find(ZoneID);
	if (!State)
	{
		FMMOWeatherState NewState;
		NewState.ZoneID = ZoneID;
		NewState.CurrentWeather = NewWeather;
		NewState.TargetWeather = NewWeather;
		NewState.BlendProgress = 1.0f;
		NewState.Intensity = Intensity;
		ZoneWeatherStates.Add(ZoneID, NewState);
		ApplyWeatherVisuals(ZoneID, NewWeather, Intensity);
		OnWeatherChanged.Broadcast(ZoneID, NewWeather, Intensity);
		return;
	}

	if (State->TargetWeather == NewWeather)
	{
		return;
	}

	State->CurrentWeather = State->TargetWeather;
	State->TargetWeather = NewWeather;
	State->BlendProgress = 0.0f;
	State->BlendDuration = BlendDuration;
	State->Intensity = Intensity;

	OnWeatherChanged.Broadcast(ZoneID, NewWeather, Intensity);
}

FMMOWeatherState UMMOWeatherSystem::GetWeatherState(FName ZoneID) const
{
	if (const FMMOWeatherState* State = ZoneWeatherStates.Find(ZoneID))
	{
		return *State;
	}
	return FMMOWeatherState();
}

EMMOWeatherType UMMOWeatherSystem::GetCurrentWeather(FName ZoneID) const
{
	if (const FMMOWeatherState* State = ZoneWeatherStates.Find(ZoneID))
	{
		if (State->BlendProgress >= 1.0f)
		{
			return State->TargetWeather;
		}
		return State->CurrentWeather;
	}
	return EMMOWeatherType::Clear;
}

float UMMOWeatherSystem::GetWeatherIntensity(FName ZoneID) const
{
	if (const FMMOWeatherState* State = ZoneWeatherStates.Find(ZoneID))
	{
		return State->Intensity;
	}
	return 0.0f;
}

void UMMOWeatherSystem::SetGlobalWeather(EMMOWeatherType NewWeather, float BlendDuration)
{
	for (auto& Pair : ZoneWeatherStates)
	{
		SetWeather(Pair.Key, NewWeather, BlendDuration, Pair.Value.Intensity);
	}
}

void UMMOWeatherSystem::RegisterWeatherCycle(FName ZoneID, const TArray<FMMOWeatherCycleEntry>& CycleEntries)
{
	WeatherCycles.Add(ZoneID, CycleEntries);
}

void UMMOWeatherSystem::StartWeatherCycle(FName ZoneID)
{
	if (!WeatherCycles.Contains(ZoneID))
	{
		return;
	}

	CycleTimers.FindOrAdd(ZoneID) = 0.0f;

	if (!ZoneWeatherStates.Contains(ZoneID))
	{
		FMMOWeatherState State;
		State.ZoneID = ZoneID;
		State.CurrentWeather = EMMOWeatherType::Clear;
		State.TargetWeather = EMMOWeatherType::Clear;
		State.BlendProgress = 1.0f;
		ZoneWeatherStates.Add(ZoneID, State);
	}
}

void UMMOWeatherSystem::StopWeatherCycle(FName ZoneID)
{
	CycleTimers.Remove(ZoneID);
}

void UMMOWeatherSystem::ServerSyncWeather(FName ZoneID)
{
	FMMOWeatherState* State = ZoneWeatherStates.Find(ZoneID);
	if (State)
	{
		OnWeatherChanged.Broadcast(ZoneID, State->TargetWeather, State->Intensity);
	}
}

void UMMOWeatherSystem::ProcessWeatherBlends(float DeltaTime)
{
	for (auto& Pair : ZoneWeatherStates)
	{
		FMMOWeatherState& State = Pair.Value;

		if (State.BlendProgress < 1.0f)
		{
			State.BlendProgress = FMath::Min(1.0f, State.BlendProgress + (DeltaTime / FMath::Max(0.1f, State.BlendDuration)));

			if (State.BlendProgress >= 1.0f)
			{
				State.CurrentWeather = State.TargetWeather;
				ApplyWeatherVisuals(Pair.Key, State.TargetWeather, State.Intensity);
			}

			OnWeatherBlendProgress.Broadcast(Pair.Key, State.BlendProgress);
		}
	}
}

void UMMOWeatherSystem::ProcessWeatherCycles(float DeltaTime)
{
	for (const auto& CyclePair : WeatherCycles)
	{
		FName ZoneID = CyclePair.Key;

		if (!CycleTimers.Contains(ZoneID))
		{
			continue;
		}

		float& Timer = CycleTimers[ZoneID];
		Timer -= DeltaTime;

		if (Timer <= 0.0f)
		{
			SelectNextWeatherInCycle(ZoneID);
		}
	}
}

void UMMOWeatherSystem::SelectNextWeatherInCycle(FName ZoneID)
{
	const TArray<FMMOWeatherCycleEntry>* CycleEntries = WeatherCycles.Find(ZoneID);
	if (!CycleEntries || CycleEntries->Num() == 0)
	{
		return;
	}

	float TotalWeight = 0.0f;
	for (const FMMOWeatherCycleEntry& Entry : *CycleEntries)
	{
		TotalWeight += Entry.Weight;
	}

	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;

	for (const FMMOWeatherCycleEntry& Entry : *CycleEntries)
	{
		AccumulatedWeight += Entry.Weight;
		if (Roll <= AccumulatedWeight)
		{
			float Duration = FMath::FRandRange(Entry.MinDurationMinutes, Entry.MaxDurationMinutes);
			CycleTimers[ZoneID] = Duration * 60.0f;

			SetWeather(ZoneID, Entry.WeatherType, 5.0f, 1.0f);
			break;
		}
	}
}

void UMMOWeatherSystem::ApplyWeatherVisuals(FName ZoneID, EMMOWeatherType Weather, float Intensity)
{
	if (!bUsePerformanceScaling)
	{
		Intensity = FMath::Min(Intensity, 1.0f);
	}

	UE_LOG(LogMMOWorld, Verbose, TEXT("Applying weather %d to zone %s with intensity %.2f"),
		static_cast<int32>(Weather), *ZoneID.ToString(), Intensity);
}

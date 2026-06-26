#include "World/MMOLevelStreamingComponent.h"
#include "MMOWorld.h"
#include "Engine/World.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Components/PrimitiveComponent.h"

UMMOLevelStreamingComponent::UMMOLevelStreamingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
	bAutoActivate = true;
}

void UMMOLevelStreamingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMMOLevelStreamingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bUseStreamingVolumes)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			EvaluateLevelDistance(Owner->GetActorLocation());
		}
	}

	ProcessPendingLoads();
}

void UMMOLevelStreamingComponent::RequestLevelLoad(TSoftObjectPtr<UWorld> Level, EMMOStreamingPriority Priority)
{
	if (Level.IsNull())
	{
		return;
	}

	if (!ManagedLevels.Contains(Level))
	{
		ManagedLevels.Add(Level);
	}

	FMMONearbyLevelInfo* LevelInfo = NearbyLevels.Find(Level);
	if (!LevelInfo)
	{
		FMMONearbyLevelInfo NewInfo;
		NewInfo.Level = Level;
		NewInfo.Priority = Priority;
		NewInfo.bShouldBeLoaded = true;
		NewInfo.bIsLoaded = false;
		NearbyLevels.Add(Level, NewInfo);
	}
	else
	{
		LevelInfo->bShouldBeLoaded = true;
		if (static_cast<uint8>(Priority) > static_cast<uint8>(LevelInfo->Priority))
		{
			LevelInfo->Priority = Priority;
		}
	}

	if (!PendingLoadQueue.Contains(Level))
	{
		PendingLoadQueue.Add(Level);
	}

	OnLevelLoadRequested.Broadcast(Level);
}

void UMMOLevelStreamingComponent::RequestLevelUnload(TSoftObjectPtr<UWorld> Level)
{
	if (FMMONearbyLevelInfo* LevelInfo = NearbyLevels.Find(Level))
	{
		LevelInfo->bShouldBeLoaded = false;
		LevelInfo->Priority = EMMOStreamingPriority::Low;
	}

	PendingLoadQueue.Remove(Level);

	UnloadLevelAsync(Level);
	OnLevelUnloadRequested.Broadcast(Level);
}

bool UMMOLevelStreamingComponent::IsLevelLoaded(TSoftObjectPtr<UWorld> Level) const
{
	if (const FMMONearbyLevelInfo* LevelInfo = NearbyLevels.Find(Level))
	{
		return LevelInfo->bIsLoaded;
	}
	return false;
}

void UMMOLevelStreamingComponent::UpdateNearbyLevels()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		EvaluateLevelDistance(Owner->GetActorLocation());
	}
}

void UMMOLevelStreamingComponent::AddStreamingLevel(TSoftObjectPtr<UWorld> Level)
{
	if (!ManagedLevels.Contains(Level))
	{
		ManagedLevels.Add(Level);
	}
}

void UMMOLevelStreamingComponent::RemoveStreamingLevel(TSoftObjectPtr<UWorld> Level)
{
	ManagedLevels.Remove(Level);
	NearbyLevels.Remove(Level);
	PendingLoadQueue.Remove(Level);

	if (IsLevelLoaded(Level))
	{
		UnloadLevelAsync(Level);
	}
}

void UMMOLevelStreamingComponent::RegisterStreamingVolume(AActor* VolumeActor)
{
	if (VolumeActor && !StreamingVolumes.Contains(VolumeActor))
	{
		StreamingVolumes.Add(VolumeActor);
	}
}

void UMMOLevelStreamingComponent::SortLevelsByPriority()
{
	PendingLoadQueue.Sort([this](const TSoftObjectPtr<UWorld>& A, const TSoftObjectPtr<UWorld>& B)
	{
		const FMMONearbyLevelInfo* InfoA = NearbyLevels.Find(A);
		const FMMONearbyLevelInfo* InfoB = NearbyLevels.Find(B);
		uint8 PriorityA = InfoA ? static_cast<uint8>(InfoA->Priority) : 0;
		uint8 PriorityB = InfoB ? static_cast<uint8>(InfoB->Priority) : 0;
		return PriorityA > PriorityB;
	});
}

void UMMOLevelStreamingComponent::ProcessPendingLoads()
{
	int32 CurrentlyLoading = 0;
	for (const auto& Pair : NearbyLevels)
	{
		if (Pair.Value.bShouldBeLoaded && !Pair.Value.bIsLoaded)
		{
			CurrentlyLoading++;
		}
	}

	if (CurrentlyLoading >= MaxConcurrentLoads)
	{
		return;
	}

	SortLevelsByPriority();

	TArray<TSoftObjectPtr<UWorld>> ToRemove;
	for (const TSoftObjectPtr<UWorld>& Level : PendingLoadQueue)
	{
		if (CurrentlyLoading >= MaxConcurrentLoads)
		{
			break;
		}

		FMMONearbyLevelInfo* LevelInfo = NearbyLevels.Find(Level);
		if (LevelInfo && LevelInfo->bShouldBeLoaded && !LevelInfo->bIsLoaded)
		{
			LoadLevelAsync(Level);
			ToRemove.Add(Level);
			CurrentlyLoading++;
		}
		else
		{
			ToRemove.Add(Level);
		}
	}

	for (const TSoftObjectPtr<UWorld>& Level : ToRemove)
	{
		PendingLoadQueue.Remove(Level);
	}
}

void UMMOLevelStreamingComponent::EvaluateLevelDistance(const FVector& PlayerLocation)
{
	for (const TSoftObjectPtr<UWorld>& Level : ManagedLevels)
	{
		FMMONearbyLevelInfo* LevelInfo = NearbyLevels.Find(Level);
		if (!LevelInfo)
		{
			continue;
		}

		LevelInfo->Distance = 0.0f;

		FLatentActionInfo LatentInfo;
		ULevelStreamingDynamic* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
			GetOwner(),
			Level,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			LatentInfo
		);

		if (StreamingLevel)
		{
			FVector LevelOrigin = StreamingLevel->LevelTransform.GetLocation();
			LevelInfo->Distance = FVector::Dist(PlayerLocation, LevelOrigin);
		}

		bool bShouldLoad = LevelInfo->Distance <= LoadDistance;
		bool bShouldUnload = LevelInfo->Distance >= UnloadDistance;

		if (bShouldLoad && !LevelInfo->bIsLoaded)
		{
			LevelInfo->bShouldBeLoaded = true;
			RequestLevelLoad(Level, LevelInfo->Priority);
		}
		else if (bShouldUnload && LevelInfo->bIsLoaded)
		{
			LevelInfo->bShouldBeLoaded = false;
			RequestLevelUnload(Level);
		}
	}
}

void UMMOLevelStreamingComponent::LoadLevelAsync(TSoftObjectPtr<UWorld> Level)
{
	if (Level.IsNull())
	{
		return;
	}

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = TEXT("OnLevelLoaded");
	LatentInfo.UUID = GetUniqueID();
	LatentInfo.Linkage = 0;

	ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		GetOwner(),
		Level,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		LatentInfo
	);

	if (FMMONearbyLevelInfo* LevelInfo = NearbyLevels.Find(Level))
	{
		LevelInfo->bIsLoaded = true;
		OnLevelStreamingStateChanged.Broadcast(Level, true);
	}
}

void UMMOLevelStreamingComponent::UnloadLevelAsync(TSoftObjectPtr<UWorld> Level)
{
	if (Level.IsNull())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FString LevelName = Level.ToSoftObjectPath().GetAssetName();
	for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
	{
		if (StreamingLevel && StreamingLevel->GetWorldAssetPackageName().Contains(LevelName))
		{
			StreamingLevel->SetShouldBeLoaded(false);
			StreamingLevel->SetShouldBeVisible(false);
			StreamingLevel->bShouldBlockOnUnload = false;
			break;
		}
	}

	if (FMMONearbyLevelInfo* LevelInfo = NearbyLevels.Find(Level))
	{
		LevelInfo->bIsLoaded = false;
		OnLevelStreamingStateChanged.Broadcast(Level, false);
	}
}

void UMMOLevelStreamingComponent::OnLevelLoaded()
{
	UE_LOG(LogMMOWorld, Verbose, TEXT("Level streaming load completed."));
}

void UMMOLevelStreamingComponent::OnLevelUnloaded()
{
	UE_LOG(LogMMOWorld, Verbose, TEXT("Level streaming unload completed."));
}

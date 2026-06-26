#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/LevelStreaming.h"
#include "MMOLevelStreamingComponent.generated.h"

UENUM(BlueprintType)
enum class EMMOStreamingPriority : uint8
{
	Low			UMETA(DisplayName = "Low"),
	Medium		UMETA(DisplayName = "Medium"),
	High		UMETA(DisplayName = "High"),
	Critical	UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FMMONearbyLevelInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<UWorld> Level;

	UPROPERTY()
	float Distance = 0.0f;

	UPROPERTY()
	EMMOStreamingPriority Priority = EMMOStreamingPriority::Low;

	UPROPERTY()
	bool bShouldBeLoaded = false;

	UPROPERTY()
	bool bIsLoaded = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelLoadRequested, TSoftObjectPtr<UWorld>, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUnloadRequested, TSoftObjectPtr<UWorld>, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelStreamingStateChanged, TSoftObjectPtr<UWorld>, Level, bool, bLoaded);

UCLASS(ClassGroup=(MMO), meta=(BlueprintSpawnableComponent))
class MMOWORLD_API UMMOLevelStreamingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOLevelStreamingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void SetLoadDistance(float NewLoadDistance) { LoadDistance = NewLoadDistance; }

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void SetUnloadDistance(float NewUnloadDistance) { UnloadDistance = NewUnloadDistance; }

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	float GetLoadDistance() const { return LoadDistance; }

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	float GetUnloadDistance() const { return UnloadDistance; }

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void RequestLevelLoad(TSoftObjectPtr<UWorld> Level, EMMOStreamingPriority Priority = EMMOStreamingPriority::Medium);

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void RequestLevelUnload(TSoftObjectPtr<UWorld> Level);

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	bool IsLevelLoaded(TSoftObjectPtr<UWorld> Level) const;

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void UpdateNearbyLevels();

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void AddStreamingLevel(TSoftObjectPtr<UWorld> Level);

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void RemoveStreamingLevel(TSoftObjectPtr<UWorld> Level);

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void SetStreamingVolumeBased(bool bEnable) { bUseStreamingVolumes = bEnable; }

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void RegisterStreamingVolume(AActor* VolumeActor);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLevelLoadRequested OnLevelLoadRequested;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLevelUnloadRequested OnLevelUnloadRequested;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLevelStreamingStateChanged OnLevelStreamingStateChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
	float LoadDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
	float UnloadDistance = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
	bool bUseStreamingVolumes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
	int32 MaxConcurrentLoads = 4;

	UPROPERTY()
	TMap<TSoftObjectPtr<UWorld>, FMMONearbyLevelInfo> NearbyLevels;

	UPROPERTY()
	TArray<TSoftObjectPtr<UWorld>> ManagedLevels;

	UPROPERTY()
	TArray<AActor*> StreamingVolumes;

	UPROPERTY()
	TArray<TSoftObjectPtr<UWorld>> PendingLoadQueue;

	void SortLevelsByPriority();
	void ProcessPendingLoads();
	void EvaluateLevelDistance(const FVector& PlayerLocation);
	void LoadLevelAsync(TSoftObjectPtr<UWorld> Level);
	void UnloadLevelAsync(TSoftObjectPtr<UWorld> Level);
	void OnLevelLoaded();
	void OnLevelUnloaded();
};

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundBase.h"
#include "Engine/LevelStreaming.h"
#include "GameFramework/MMOGameMode.h"
#include "MMOZoneData.generated.h"

UENUM(BlueprintType)
enum class EMMOWeatherType : uint8
{
	Clear		UMETA(DisplayName = "Clear"),
	Rain		UMETA(DisplayName = "Rain"),
	Snow		UMETA(DisplayName = "Snow"),
	Storm		UMETA(DisplayName = "Storm"),
	Fog			UMETA(DisplayName = "Fog"),
	Sandstorm	UMETA(DisplayName = "Sandstorm")
};

USTRUCT(BlueprintType)
struct FMMOLevelRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Range")
	int32 MinLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Range")
	int32 MaxLevel = 100;
};

USTRUCT(BlueprintType)
struct FMMORespawnPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	FName RespawnPointName = NAME_None;
};

USTRUCT(BlueprintType)
struct FMMOBindingPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Binding")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Binding")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Binding")
	FName BindingPointName = NAME_None;
};

UCLASS(BlueprintType)
class MMWORLD_API UMMOZoneData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UMMOZoneData();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Identity")
	FName ZoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Identity")
	FText ZoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Identity")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Type")
	EMMOZoneType ZoneType = EMMOZoneType::Overworld;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	FMMOLevelRange MobLevelRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
	FMMOLevelRange PlayerLevelRequirement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
	FName ParentZoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
	TArray<FName> ConnectedZoneIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
	TSoftObjectPtr<UWorld> ZoneMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
	TSoftObjectPtr<UTexture2D> MinimapTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capacity")
	int32 MaxPlayerCapacity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsPvPZone = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsSanctuary = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	TArray<EMMOWeatherType> AllowedWeatherTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> AmbientMusic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> AmbientSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	TSoftClassPtr<UUserWidget> LoadingScreenWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	UTexture2D* LoadingScreenImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	FText LoadingScreenTip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Points")
	FMMORespawnPoint RespawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Points")
	FMMOBindingPoint BindingPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Continent")
	int32 ContinentID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
	TArray<TSoftObjectPtr<UWorld>> SubLevels;
};

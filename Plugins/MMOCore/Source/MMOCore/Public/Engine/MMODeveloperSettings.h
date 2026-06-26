#pragma once

#include "MMOCore.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Data/MMOGlobalData.h"
#include "MMODeveloperSettings.generated.h"

USTRUCT(BlueprintType)
struct FMMOLevelRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	FName ZoneID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	FText DenyMessage;
};

UCLASS(Config = "MMOCore", DefaultConfig, BlueprintType, meta = (DisplayName = "MMOCore Settings"))
class MMOCORE_API UMMODeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMMODeveloperSettings();

	static const UMMODeveloperSettings& Get();

	UFUNCTION(BlueprintPure, Category = "MMO|Settings")
	static UMMODeveloperSettings* GetDeveloperSettings();

	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	virtual FName GetSectionName() const override { return TEXT("MMOCore"); }

#if WITH_EDITOR
	virtual FText GetSectionText() const override { return NSLOCTEXT("MMOCore", "MMOCoreSettingsSection", "MMOCore"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("MMOCore", "MMOCoreSettingsSectionDescription", "MMOCore game framework configuration"); }
#endif

	UPROPERTY(Config, EditAnywhere, Category = "Network", meta = (DisplayName = "Server IP"))
	FString ServerIP = TEXT("127.0.0.1");

	UPROPERTY(Config, EditAnywhere, Category = "Network", meta = (DisplayName = "Server Port"))
	int32 ServerPort = 7777;

	UPROPERTY(Config, EditAnywhere, Category = "Network", meta = (DisplayName = "Login Timeout (Seconds)"))
	float LoginTimeout = 30.0f;

	UPROPERTY(Config, EditAnywhere, Category = "World", meta = (DisplayName = "Max Players Per Zone"))
	int32 MaxPlayersPerZone = 100;

	UPROPERTY(Config, EditAnywhere, Category = "World", meta = (DisplayName = "Default Spawn Location"))
	TSoftObjectPtr<UWorld> DefaultSpawnLocation;

	UPROPERTY(Config, EditAnywhere, Category = "World", meta = (DisplayName = "Default Player Start Tag"))
	FName DefaultPlayerStartTag = NAME_None;

	UPROPERTY(Config, EditAnywhere, Category = "World", meta = (DisplayName = "Level Requirements Per Zone"))
	TArray<FMMOLevelRequirement> ZoneLevelRequirements;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay|Experience", meta = (DisplayName = "Experience Curve"))
	TArray<FMMOExperienceEntry> ExperienceCurve;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay|Experience", meta = (DisplayName = "Experience Curve Table"))
	TSoftObjectPtr<UDataTable> ExperienceCurveTable;

	UPROPERTY(Config, EditAnywhere, Category = "Chat", meta = (DisplayName = "Filtered Words"))
	TArray<FString> ChatFilteredWords;

	UPROPERTY(Config, EditAnywhere, Category = "Chat", meta = (DisplayName = "Chat Max Length"))
	int32 ChatMaxLength = 200;

	UPROPERTY(Config, EditAnywhere, Category = "Chat", meta = (DisplayName = "Chat Cooldown (Seconds)"))
	float ChatCooldown = 1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay", meta = (DisplayName = "Max Character Level"))
	int32 MaxCharacterLevel = 100;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay", meta = (DisplayName = "Base Movement Speed"))
	float BaseMovementSpeed = 600.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay", meta = (DisplayName = "Sprint Speed Multiplier"))
	float SprintSpeedMultiplier = 1.5f;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay", meta = (DisplayName = "Base Health"))
	float BaseHealth = 100.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay", meta = (DisplayName = "Base Mana"))
	float BaseMana = 50.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay|Combat", meta = (DisplayName = "Global Damage Multiplier"))
	float GlobalDamageMultiplier = 1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay|Combat", meta = (DisplayName = "Global Defense Multiplier"))
	float GlobalDefenseMultiplier = 1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay|Combat", meta = (DisplayName = "PVP Damage Multiplier"))
	float PVPDamageMultiplier = 1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay|Drops", meta = (DisplayName = "Global Drop Rate Multiplier"))
	float GlobalDropRateMultiplier = 1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay|Drops", meta = (DisplayName = "Drop Rate Table"))
	TSoftObjectPtr<UDataTable> DropRateTable;

	UPROPERTY(Config, EditAnywhere, Category = "Performance", meta = (DisplayName = "Server Tick Rate"))
	float ServerTickRate = 30.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Performance", meta = (DisplayName = "Net Update Frequency"))
	int32 NetUpdateFrequency = 100;

	UPROPERTY(Config, EditAnywhere, Category = "Replication", meta = (DisplayName = "Relevancy Distance"))
	float RelevancyDistance = 150000.0f;
};

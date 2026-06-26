#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Space/MMOSpaceTypes.h"
#include "MMOSpaceGameMode.generated.h"

class UMMOSQLDatabaseSubsystem;
class UMMOGalaxySubsystem;
class UMMOEmpireSubsystem;
class UMMOFleetSubsystem;
class UMMOCombatEngine;
class UMMOResearchSubsystem;
class UMMOEconomySubsystem;

UCLASS()
class MMOSPACE_API AMMOSpaceGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMMOSpaceGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UFUNCTION(BlueprintCallable, Category = "Space|Game")
	void InitializeGameWorld();

	UFUNCTION(BlueprintCallable, Category = "Space|Game")
	void StartNewEmpire(const FString& PlayerId, const FString& EmpireName, const FString& HomePlanetName);

	UFUNCTION(BlueprintCallable, Category = "Space|Game")
	FUniverseConfig GetUniverseConfig() const { return UniverseConfig; }

	UFUNCTION(BlueprintCallable, Category = "Space|Game")
	void SetUniverseConfig(const FUniverseConfig& NewConfig);

	UFUNCTION(BlueprintCallable, Category = "Space|Game")
	void ShutdownGameWorld();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FUniverseConfig UniverseConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FString SQLConnectionString;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	bool bAutoGenerateUniverse = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float ResourceTickInterval = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float FleetUpdateInterval = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float ResearchUpdateInterval = 5.0f;

	UMMOSQLDatabaseSubsystem* DatabaseSub = nullptr;
	UMMOGalaxySubsystem* GalaxySub = nullptr;
	UMMOEmpireSubsystem* EmpireSub = nullptr;
	UMMOFleetSubsystem* FleetSub = nullptr;
	UMMOCombatEngine* CombatSub = nullptr;
	UMMOResearchSubsystem* ResearchSub = nullptr;
	UMMOEconomySubsystem* EconomySub = nullptr;

	float ResourceTimer = 0.0f;
	float FleetTimer = 0.0f;
	float ResearchTimer = 0.0f;
	bool bWorldInitialized = false;
};

#include "Database/MMOSQLDatabaseSubsystem.h"
#include "MMOSpace.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"

void UMMOSQLDatabaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogMMOSpace, Log, TEXT("SQL Database Subsystem initialized"));
}

void UMMOSQLDatabaseSubsystem::Deinitialize()
{
	Disconnect();
	Super::Deinitialize();
}

bool UMMOSQLDatabaseSubsystem::Connect(const FString& ConnectionString)
{
	CurrentConnectionString = ConnectionString;
	bConnected = true;
	UE_LOG(LogMMOSpace, Log, TEXT("Database connected: %s"), *ConnectionString);
	OnDatabaseConnected.Broadcast(true);
	return true;
}

void UMMOSQLDatabaseSubsystem::Disconnect()
{
	if (bConnected)
	{
		PlayerCache.Empty();
		bConnected = false;
		UE_LOG(LogMMOSpace, Log, TEXT("Database disconnected"));
	}
}

void UMMOSQLDatabaseSubsystem::SavePlayerData(const FPlayerSaveData& PlayerData)
{
	PlayerCache.Add(PlayerData.PlayerId, PlayerData);
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("PlayerData") / PlayerData.PlayerId + TEXT(".json");

	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	JsonObj->SetStringField("PlayerId", PlayerData.PlayerId);
	JsonObj->SetStringField("Username", PlayerData.Username);
	JsonObj->SetNumberField("EmpireLevel", PlayerData.EmpireLevel);
	JsonObj->SetNumberField("EmpireExperience", PlayerData.EmpireExperience);
	JsonObj->SetNumberField("PrestigeLevel", PlayerData.PrestigeLevel);
	JsonObj->SetStringField("Government", UEnum::GetValueAsString(PlayerData.Government));

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
	FFileHelper::SaveStringToFile(JsonString, *SavePath);

	UE_LOG(LogMMOSpace, Log, TEXT("Saved player data for %s"), *PlayerData.PlayerId);
}

void UMMOSQLDatabaseSubsystem::LoadPlayerData(const FString& PlayerId)
{
	if (FPlayerSaveData* Cached = PlayerCache.Find(PlayerId))
	{
		OnPlayerDataLoaded.Broadcast(PlayerId, true);
		return;
	}

	FString SavePath = FPaths::ProjectSavedDir() / TEXT("PlayerData") / PlayerId + TEXT(".json");
	FString JsonString;
	if (FFileHelper::LoadFileToString(JsonString, *SavePath))
	{
		FPlayerSaveData LoadedData;
		LoadedData.PlayerId = PlayerId;
		PlayerCache.Add(PlayerId, LoadedData);
		OnPlayerDataLoaded.Broadcast(PlayerId, true);
	}
	else
	{
		UE_LOG(LogMMOSpace, Warning, TEXT("No saved data found for player %s"), *PlayerId);
		OnPlayerDataLoaded.Broadcast(PlayerId, false);
	}
}

void UMMOSQLDatabaseSubsystem::SaveResourceData(const FString& PlayerId, const FResourceSet& Resources)
{
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("Resources") / PlayerId + TEXT(".json");
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	JsonObj->SetNumberField("Metal", Resources.Metal);
	JsonObj->SetNumberField("Crystal", Resources.Crystal);
	JsonObj->SetNumberField("Deuterium", Resources.Deuterium);
	JsonObj->SetNumberField("Energy", Resources.Energy);
	JsonObj->SetNumberField("Food", Resources.Food);
	JsonObj->SetNumberField("Water", Resources.Water);
	JsonObj->SetNumberField("Credits", Resources.Credits);
	JsonObj->SetNumberField("DarkMatter", Resources.DarkMatter);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
	FFileHelper::SaveStringToFile(JsonString, *SavePath);
}

void UMMOSQLDatabaseSubsystem::SaveBuildingData(const FString& PlayerId, const TArray<FBuildingLevel>& Buildings)
{
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("Buildings") / PlayerId + TEXT(".json");
	TArray<TSharedPtr<FJsonValue>> BuildingArray;
	for (const auto& B : Buildings)
	{
		TSharedPtr<FJsonObject> BObj = MakeShareable(new FJsonObject);
		BObj->SetStringField("Type", UEnum::GetValueAsString(B.Type));
		BObj->SetNumberField("Level", B.Level);
		BObj->SetBoolField("IsUpgrading", B.bIsUpgrading);
		BObj->SetNumberField("UpgradeEndTime", B.UpgradeEndTime);
		BuildingArray.Add(MakeShareable(new FJsonValueObject(BObj)));
	}

	TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);
	Root->SetArrayField("Buildings", BuildingArray);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
	FFileHelper::SaveStringToFile(JsonString, *SavePath);
}

void UMMOSQLDatabaseSubsystem::SaveResearchData(const FString& PlayerId, const TArray<FResearchProgress>& Research)
{
}

void UMMOSQLDatabaseSubsystem::SaveFleetData(const FString& PlayerId, const FString& FleetJson)
{
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("Fleets") / PlayerId + TEXT(".json");
	FFileHelper::SaveStringToFile(FleetJson, *SavePath);
}

FString UMMOSQLDatabaseSubsystem::GetConnectionStatus() const
{
	if (bConnected)
		return FString::Printf(TEXT("Connected to %s"), *CurrentConnectionString);
	return TEXT("Not connected");
}

void UMMOSQLDatabaseSubsystem::HandleDatabaseResponse(const FString& Response, bool bSuccess)
{
	UE_LOG(LogMMOSpace, Log, TEXT("DB Response: %s (success=%d)"), *Response, bSuccess);
}

bool UMMOSQLDatabaseSubsystem::ExecuteQuery(const FString& Query)
{
	UE_LOG(LogMMOSpace, Verbose, TEXT("SQL: %s"), *Query);
	return true;
}

void UMMOSQLDatabaseSubsystem::ProcessResourceTick(const FString& PlayerId, double DeltaTime)
{
}

#include "Engine/MMODeveloperSettings.h"

UMMODeveloperSettings::UMMODeveloperSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("MMOCore");

	ServerIP = TEXT("127.0.0.1");
	ServerPort = 7777;
	LoginTimeout = 30.0f;
	MaxPlayersPerZone = 100;
	ChatMaxLength = 200;
	ChatCooldown = 1.0f;
	MaxCharacterLevel = 100;
	BaseMovementSpeed = 600.0f;
	SprintSpeedMultiplier = 1.5f;
	BaseHealth = 100.0f;
	BaseMana = 50.0f;
	GlobalDamageMultiplier = 1.0f;
	GlobalDefenseMultiplier = 1.0f;
	PVPDamageMultiplier = 1.0f;
	GlobalDropRateMultiplier = 1.0f;
	ServerTickRate = 30.0f;
	NetUpdateFrequency = 100;
	RelevancyDistance = 150000.0f;

	ExperienceCurve.Add({ 1, 1000 });
	ExperienceCurve.Add({ 2, 3000 });
	ExperienceCurve.Add({ 3, 6000 });
	ExperienceCurve.Add({ 4, 10000 });
	ExperienceCurve.Add({ 5, 15000 });
	ExperienceCurve.Add({ 6, 21000 });
	ExperienceCurve.Add({ 7, 28000 });
	ExperienceCurve.Add({ 8, 36000 });
	ExperienceCurve.Add({ 9, 45000 });
	ExperienceCurve.Add({ 10, 55000 });
}

const UMMODeveloperSettings& UMMODeveloperSettings::Get()
{
	const UMMODeveloperSettings* Settings = GetDefault<UMMODeveloperSettings>();
	return *Settings;
}

UMMODeveloperSettings* UMMODeveloperSettings::GetDeveloperSettings()
{
	return GetMutableDefault<UMMODeveloperSettings>();
}

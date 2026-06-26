#include "World/MMOZoneData.h"

UMMOZoneData::UMMOZoneData()
{
	ZoneID = FName(TEXT("default_zone"));
	ZoneName = FText::FromString(TEXT("Default Zone"));
	Description = FText::GetEmpty();
	ZoneType = EMMOZoneType::Overworld;
	MobLevelRange = FMMOLevelRange{ 1, 100 };
	PlayerLevelRequirement = FMMOLevelRange{ 1, 100 };
	ParentZoneID = NAME_None;
	bIsPvPZone = false;
	bIsSanctuary = false;
	MaxPlayerCapacity = 0;
	ContinentID = 0;
	AllowedWeatherTypes = { EMMOWeatherType::Clear };
}

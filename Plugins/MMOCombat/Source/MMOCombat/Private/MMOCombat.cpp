#include "MMOCombat.h"

DEFINE_LOG_CATEGORY(LogMMOCombat);

IMPLEMENT_MODULE(FMMOCombatModule, MMOCombat)

void FMMOCombatModule::StartupModule()
{
	UE_LOG(LogMMOCombat, Log, TEXT("MMOCombat module started."));
}

void FMMOCombatModule::ShutdownModule()
{
	UE_LOG(LogMMOCombat, Log, TEXT("MMOCombat module shut down."));
}

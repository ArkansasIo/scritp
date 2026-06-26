#include "MMOParty.h"

DEFINE_LOG_CATEGORY(LogMMOParty);

IMPLEMENT_MODULE(FMMOPartyModule, MMOParty);

void FMMOPartyModule::StartupModule()
{
	UE_LOG(LogMMOParty, Log, TEXT("MMOParty module started."));
}

void FMMOPartyModule::ShutdownModule()
{
	UE_LOG(LogMMOParty, Log, TEXT("MMOParty module shut down."));
}
